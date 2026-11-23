#include "feature_intro_service.h"

#include "../settings/cache_settings.h"
#include "feature_intro_planner.h"

#include <QApplication>
#include <QDateTime>
#include <QHash>

namespace
{
/// One prompt per week keeps "later" from turning into a nag.
constexpr qint64 kCooldownSeconds = 7 * 24 * 60 * 60;

FeatureIntroSettings &introSettings()
{
    return SettingsCache::instance().featureIntros();
}

qint64 nowEpoch()
{
    return QDateTime::currentSecsSinceEpoch();
}
} // namespace

FeatureIntroService::FeatureIntroService(QObject *parent) : QObject(parent)
{
}

bool isFeatureIntroNew(const QString &id)
{
    const auto state = SettingsCache::instance().featureIntros().getState(id);
    return state && state->decision == FeatureIntroDecision::Pending;
}

void FeatureIntroService::seed(const QString &previousVersion, const QString &currentVersion)
{
    const QVector<FeatureIntro> &catalog = featureIntroCatalog();

    QHash<QString, FeatureIntroState> known;
    for (const QString &id : introSettings().getKnownIds()) {
        if (const auto state = introSettings().getState(id)) {
            known.insert(id, *state);
        }
    }

    QSet<QString> configured;
    for (const FeatureIntro &intro : catalog) {
        if (isConfigured(intro.id)) {
            configured.insert(intro.id);
        }
    }

    const FeatureIntroPlan plan = planFeatureIntros(catalog, known, configured, previousVersion, currentVersion);
    if (!plan.writes.isEmpty()) {
        // One sync: a crash must not leave half the catalog seeded.
        introSettings().setStates(plan.writes);
    }
}

const FeatureIntro *FeatureIntroService::nextDueIntro()
{
    if (!promptAllowed()) {
        return nullptr;
    }

    for (const FeatureIntro &intro : featureIntroCatalog()) {
        const auto state = introSettings().getState(intro.id);
        if (!state || state->decision != FeatureIntroDecision::Pending) {
            continue;
        }

        if (isConfigured(intro.id)) {
            // Configured since it was queued: answer it for them instead of asking.
            introSettings().setState(intro.id, FeatureIntroState{.decision = FeatureIntroDecision::Skipped,
                                                                 .revision = intro.revision,
                                                                 .decidedAt = nowEpoch()});
            emit decisionChanged(intro.id);
            continue;
        }

        notePromptShown();
        return &intro;
    }

    return nullptr;
}

std::optional<FeatureIntroState> FeatureIntroService::stateFor(const QString &id) const
{
    return introSettings().getState(id);
}

FeatureIntroDecision FeatureIntroService::decisionFor(const QString &id) const
{
    const auto state = introSettings().getState(id);
    return state ? state->decision : FeatureIntroDecision::Baseline;
}

bool FeatureIntroService::isNew(const QString &id) const
{
    return isFeatureIntroNew(id);
}

bool FeatureIntroService::isConfigured(const QString &id) const
{
    const FeatureIntroHandlers handlers = featureIntroHandlers(id);
    return handlers.isConfigured && handlers.isConfigured();
}

bool FeatureIntroService::isAlternativeActive(const QString &id) const
{
    const FeatureIntroHandlers handlers = featureIntroHandlers(id);
    return handlers.isAlternativeActive && handlers.isAlternativeActive();
}

FeatureIntroAction FeatureIntroService::decide(const QString &id, FeatureIntroDecision decision)
{
    const FeatureIntro *intro = findFeatureIntro(id);
    if (!intro) {
        return {};
    }

    const FeatureIntroHandlers handlers = featureIntroHandlers(id);
    FeatureIntroAction undo;

    if (decision == FeatureIntroDecision::Accepted) {
        if (handlers.applyAccept) {
            undo = handlers.applyAccept();
        }
    } else if (decision == FeatureIntroDecision::Declined) {
        if (handlers.applyDecline) {
            undo = handlers.applyDecline();
        }
    }

    introSettings().setState(
        id, FeatureIntroState{.decision = decision, .revision = intro->revision, .decidedAt = nowEpoch()});
    emit decisionChanged(id);
    return undo;
}

bool FeatureIntroService::isGlobalEnabled() const
{
    return introSettings().getGlobalEnabled();
}

void FeatureIntroService::setGlobalEnabled(bool enabled)
{
    if (introSettings().getGlobalEnabled() == enabled) {
        return;
    }
    introSettings().setGlobalEnabled(enabled);
    emit globalEnabledChanged(enabled);
}

bool FeatureIntroService::promptAllowed() const
{
    if (promptedThisSession) {
        return false;
    }

    if (!introSettings().getGlobalEnabled()) {
        return false;
    }

    if (QApplication::activeModalWidget() != nullptr) {
        // A prompt behind a dialog is a prompt nobody reads.
        return false;
    }

    const qint64 lastPrompt = introSettings().getLastPromptEpoch();
    return lastPrompt <= 0 || nowEpoch() - lastPrompt >= kCooldownSeconds;
}

void FeatureIntroService::notePromptShown()
{
    promptedThisSession = true;
    introSettings().setLastPromptEpoch(nowEpoch());
}
