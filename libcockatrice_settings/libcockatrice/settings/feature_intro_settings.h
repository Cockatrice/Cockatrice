#ifndef FEATURE_INTRO_SETTINGS_H
#define FEATURE_INTRO_SETTINGS_H

#include "settings_manager.h"

#include <QList>
#include <QPair>
#include <QString>
#include <QStringList>
#include <optional>
#include <qtmetamacros.h>

class QObject;

/**
 * @brief What the user decided about one feature introduction.
 *
 * Pending is the only state that can still show an invitation. Baseline means the
 * feature predates the user's install (or already shipped to them) and must never
 * be announced. Skipped means the feature was already configured explicitly.
 */
enum class FeatureIntroDecision
{
    Pending,
    Accepted,
    Declined,
    Dismissed,
    Baseline,
    Skipped,
};

struct FeatureIntroState
{
    FeatureIntroDecision decision = FeatureIntroDecision::Pending;
    /// Catalog revision the decision applies to; a newer revision re-queues the entry.
    int revision = 0;
    /// Unix epoch seconds of the decision, or 0 when it has not been made yet.
    qint64 decidedAt = 0;
    /// Catalog revision already displayed to the user, or 0 when never shown. This
    /// is the unread marker: an entry that has been on screen keeps a neutral glyph
    /// even while it still awaits an answer.
    int shownRevision = 0;
};

class FeatureIntroSettings : public SettingsManager
{
    Q_OBJECT
    friend class SettingsCache;

public:
    [[nodiscard]] bool getGlobalEnabled() const;
    void setGlobalEnabled(bool enabled);

    [[nodiscard]] std::optional<FeatureIntroState> getState(const QString &id) const;
    void setState(const QString &id, const FeatureIntroState &state);
    /// Writes several states in one sync, so a crash cannot leave a half-seeded catalog.
    void setStates(const QList<QPair<QString, FeatureIntroState>> &states);
    void clearState(const QString &id);
    [[nodiscard]] QStringList getKnownIds() const;

    /// Records that @a revision of @a id has been on screen; writes only when it
    /// changes, so a dialog shown again does not touch the file.
    void setShownRevision(const QString &id, int revision);

signals:
    void globalEnabledChanged(bool enabled);
    void stateChanged(const QString &id);

public:
    explicit FeatureIntroSettings(const QString &settingPath, QObject *parent = nullptr);

private:
    FeatureIntroSettings(const FeatureIntroSettings &) = delete;
    FeatureIntroSettings &operator=(const FeatureIntroSettings &) = delete;
};

#endif // FEATURE_INTRO_SETTINGS_H
