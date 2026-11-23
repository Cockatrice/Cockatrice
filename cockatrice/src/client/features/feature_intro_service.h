#ifndef FEATURE_INTRO_SERVICE_H
#define FEATURE_INTRO_SERVICE_H

#include "feature_intro.h"
#include "feature_intro_actions.h"

#include <QObject>
#include <QSet>
#include <QString>
#include <libcockatrice/settings/feature_intro_settings.h>
#include <optional>

/**
 * @file feature_intro_service.h
 * @brief Decides when a feature introduction may be shown, and records the answer.
 *
 * Owned by `TabSupervisor` and seeded once per launch from
 * `MainWindow::startupConfigCheck`, before the running version overwrites the
 * previous one. The service holds no UI: the banner and the dialog ask it
 * questions and report back, which keeps the guards in one testable place.
 */
/**
 * @brief True while @a id has not been answered yet.
 *
 * Shared by the service and the settings badge so that "still new" cannot drift
 * into two definitions.
 */
bool isFeatureIntroNew(const QString &id);

class FeatureIntroService : public QObject
{
    Q_OBJECT

public:
    explicit FeatureIntroService(QObject *parent = nullptr);

    /**
     * @brief Seeds and refreshes the stored state for the shipped catalog.
     *
     * @param previousVersion version this install ran before, empty on a clean install
     * @param currentVersion  the running `VERSION_STRING`
     */
    void seed(const QString &previousVersion, const QString &currentVersion);

    /**
     * @brief The entry that may be announced right now, or nullptr.
     *
     * Applies every guard: the global switch, one banner per session, the cooldown,
     * and "never over a modal dialog". Consuming this counts as showing the banner.
     */
    const FeatureIntro *nextDueIntro();

    [[nodiscard]] std::optional<FeatureIntroState> stateFor(const QString &id) const;
    [[nodiscard]] FeatureIntroDecision decisionFor(const QString &id) const;
    /// True while the user has not been asked about @a id yet; drives the settings badge.
    [[nodiscard]] bool isNew(const QString &id) const;
    [[nodiscard]] bool isConfigured(const QString &id) const;
    /// True when the user currently has the classic / off behavior.
    [[nodiscard]] bool isAlternativeActive(const QString &id) const;

    /**
     * @brief Records @a decision for @a id and applies the matching behavior.
     * @return the undo for the behavior change, empty when nothing changed
     */
    FeatureIntroAction decide(const QString &id, FeatureIntroDecision decision);

    [[nodiscard]] bool isGlobalEnabled() const;
    void setGlobalEnabled(bool enabled);

signals:
    void globalEnabledChanged(bool enabled);
    void decisionChanged(const QString &id);

private:
    bool promptAllowed() const;
    void notePromptShown();

    bool promptedThisSession = false;
};

#endif // FEATURE_INTRO_SERVICE_H
