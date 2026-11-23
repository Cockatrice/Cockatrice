#ifndef FEATURE_INTRO_H
#define FEATURE_INTRO_H

#include <QString>
#include <QVector>

/**
 * @file feature_intro.h
 * @brief Description of one feature rollout the client may announce in place.
 *
 * The catalog is static data: one entry per feature whose rollout deserves an
 * introduction. Deciding *whether* to show an entry is the planner's job
 * (`feature_intro_planner.h`); deciding *when* to show it is the service's
 * (`feature_intro_service.h`); the per-feature settings wiring lives in
 * `feature_intro_actions.h` so this type stays free of client dependencies and
 * stays testable on its own.
 */

/**
 * @brief How the feature relates to the behavior the user already had.
 */
enum class FeatureIntroShape
{
    /// The feature ships off. The invitation offers to turn it on.
    OptIn,
    /// The feature ships on in place of the old behavior. The invitation explains
    /// it and offers the classic path back.
    Rework,
};

struct FeatureIntro
{
    /// Stable identifier; also the settings key, so it must never change.
    QString id;
    /// Bumped when the feature changed enough that an earlier decision should be
    /// revisited exactly once.
    int revision = 1;
    FeatureIntroShape shape = FeatureIntroShape::OptIn;
    /// First release that contains the feature, as a `VERSION_STRING` prefix
    /// (`3.2.0`). Installs older than this are invited; the rest are baselined.
    QString introducedIn;

    /// Stable key of the surface whose arrival may announce this feature, e.g.
    /// `game` (see `Tab::introSurface()`). The corner glyph only appears while the
    /// surface in front matches, so an introduction is never offered where its
    /// change cannot be seen. Empty means anywhere: the glyph may appear on every
    /// surface until the entry is acknowledged, which is reserved for a change with
    /// no screen of its own.
    QString surface;

    /// One line naming the feature.
    QString title;
    /// What the feature does, in the user's terms; plain text, no HTML.
    QString body;
    /// Keeps the new behavior (`Rework`) or turns it on (`OptIn`).
    QString acceptLabel;
    /// Keeps the classic behavior (`Rework`) or leaves the feature off (`OptIn`).
    QString declineLabel;
    /// Shown in place of the invitation once the user switched to the classic
    /// behavior, above the undo. Empty for shapes whose decline changes nothing.
    QString declinedOutcome;
    /// Shown on the "New" badge that marks the owning settings group.
    QString badgeTooltip;
    /// Stable key of the settings page holding the feature's controls, e.g.
    /// `user-interface`; `DlgSettings::openSetting` maps it to the page.
    QString settingsPage;
    /// Search query that lands on the feature's settings group, translated in the
    /// context that owns the group title so both lookups agree.
    QString settingsQuery;
};

/// Every feature that can be introduced, in the order the user should see them.
const QVector<FeatureIntro> &featureIntroCatalog();

/// The catalog entry with @a id, or nullptr when the catalog no longer ships it.
const FeatureIntro *findFeatureIntro(const QString &id);

#endif // FEATURE_INTRO_H
