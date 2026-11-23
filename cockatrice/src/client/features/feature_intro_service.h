#ifndef FEATURE_INTRO_SERVICE_H
#define FEATURE_INTRO_SERVICE_H

#include "feature_intro.h"
#include "feature_intro_actions.h"

#include <QHash>
#include <QObject>
#include <QString>
#include <libcockatrice/settings/feature_intro_settings.h>
#include <optional>

/**
 * @file feature_intro_service.h
 * @brief Decides when a feature introduction may be shown, and records the answer.
 *
 * Owned by `TabSupervisor` and seeded once per launch from
 * `MainWindow::startupConfigCheck`, before the running version overwrites the
 * previous one. The service holds no UI: the corner glyph and the dialog ask it
 * questions and report back, which keeps the rules in one testable place.
 *
 * Nothing here is a prompt any more. The glyph appears only on the surface whose
 * change the entry describes, so there is no cooldown to run and no dialog to
 * defer: an unanswered entry is simply visible where it applies until it is
 * answered.
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
     * @brief Every unanswered entry that applies to @a surface, in catalog order.
     *
     * An entry whose @c surface is set only applies where that surface is in front;
     * an entry with no surface applies anywhere. Returns empty while the global
     * switch is off — the entries stay pending rather than being answered for the
     * user. Entries the user configured since they were queued are answered as
     * `Skipped` instead of being listed.
     */
    QVector<const FeatureIntro *> pendingFor(const QString &surface);

    /// True while @a id awaits an answer and its current revision has never been
    /// on screen; drives the glyph's marked tone.
    [[nodiscard]] bool isUnseen(const QString &id) const;

    /// Records that the current revision of @a id has been on screen.
    void markShown(const QString &id);

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
    /**
     * @brief Undo kept from a decline made in this process, by entry id.
     *
     * The banner used to keep this alive on screen. The index can restore the exact
     * values the decline replaced only while the process that made it is still
     * running; after a restart it falls back to the accepted behavior, which is the
     * shipped default.
     */
    QHash<QString, FeatureIntroAction> declinedUndo;
};

#endif // FEATURE_INTRO_SERVICE_H
