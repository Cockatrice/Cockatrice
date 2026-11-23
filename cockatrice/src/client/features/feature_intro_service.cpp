#include "feature_intro_service.h"

#include "../settings/cache_settings.h"
#include "feature_intro_planner.h"

#include <QDateTime>

namespace
{
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

QVector<const FeatureIntro *> FeatureIntroService::pendingFor(const QString &surface)
{
    QVector<const FeatureIntro *> due;

    if (!introSettings().getGlobalEnabled()) {
        return due;
    }

    for (const FeatureIntro &intro : featureIntroCatalog()) {
        const auto state = introSettings().getState(intro.id);
        if (!state || state->decision != FeatureIntroDecision::Pending) {
            continue;
        }

        // Only where the change is visible. An entry with no surface applies
        // anywhere, which is reserved for a change with no screen of its own.
        if (!intro.surface.isEmpty() && intro.surface != surface) {
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

        due.append(&intro);
    }

    return due;
}

bool FeatureIntroService::isUnseen(const QString &id) const
{
    const FeatureIntro *intro = findFeatureIntro(id);
    const auto state = introSettings().getState(id);
    return intro != nullptr && state && state->decision == FeatureIntroDecision::Pending &&
           state->shownRevision < intro->revision;
}

void FeatureIntroService::markShown(const QString &id)
{
    const FeatureIntro *intro = findFeatureIntro(id);
    if (intro == nullptr) {
        return;
    }
    introSettings().setShownRevision(id, intro->revision);
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
        // Undoing a decline restores the values the decline replaced, exactly, as
        // long as the process that made it is still running. A restart leaves only
        // the shipped default, which is what applyAccept writes.
        auto kept = declinedUndo.find(id);
        if (kept != declinedUndo.end()) {
            const FeatureIntroAction restore = kept.value();
            declinedUndo.erase(kept);
            if (restore) {
                restore();
                // The undo of accepting is declining again, but only as a value:
                // applying it here would put both triggers back to Off while the
                // state below records Accepted — finding 9's bug in reverse.
                if (handlers.applyDecline) {
                    const FeatureIntroAction declineAgain = handlers.applyDecline;
                    undo = [declineAgain] { declineAgain(); };
                }
            }
        } else if (handlers.applyAccept) {
            undo = handlers.applyAccept();
        }
    } else if (decision == FeatureIntroDecision::Declined) {
        if (handlers.applyDecline) {
            undo = handlers.applyDecline();
            if (undo) {
                declinedUndo.insert(id, undo);
            }
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
