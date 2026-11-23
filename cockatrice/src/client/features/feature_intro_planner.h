#ifndef FEATURE_INTRO_PLANNER_H
#define FEATURE_INTRO_PLANNER_H

#include "feature_intro.h"

#include <QHash>
#include <QList>
#include <QPair>
#include <QSet>
#include <QStringList>
#include <QVector>
#include <libcockatrice/settings/feature_intro_settings.h>

/**
 * @file feature_intro_planner.h
 * @brief Pure decision of which feature introductions apply to this install.
 *
 * The planner answers two questions and nothing else, which is what makes it
 * testable without a client: what should the state store hold, and which entries
 * may still invite the user. It never reads the clock, never touches the
 * settings and never assumes it runs more than once.
 */

struct FeatureIntroPlan
{
    /// States to write, for entries the store has never seen or has seen at an
    /// older revision. Empty when the store is already current.
    QList<QPair<QString, FeatureIntroState>> writes;
    /// Entries whose decided state is still `Pending`, in catalog order.
    QStringList due;
};

/**
 * @brief Seeds and refreshes the stored state for @a catalog.
 *
 * - An install with no previous version is new, so every entry is `Baseline`:
 *   a feature that was always there is not news.
 * - An install that already ran is asked about every entry it has never seen, as
 *   long as this build contains the feature (@c introducedIn at or before the
 *   running version). Adding a catalog entry is the decision to announce it once,
 *   which is what covers a change that lands mid-release-line.
 * - An entry the user already configured is `Skipped` and never invites them.
 * - A revision bump re-queues an existing decision exactly once: the write records
 *   the new revision, so the following run finds nothing to do.
 *
 * @param catalog         shipped entries, in presentation order
 * @param knownStates     state per id, as stored; missing ids have never been seen
 * @param configuredIds   ids whose settings the user already changed by hand
 * @param previousVersion the version this install ran before, empty on a clean install
 * @param currentVersion  the running `VERSION_STRING`, prerelease suffix included
 */
FeatureIntroPlan planFeatureIntros(const QVector<FeatureIntro> &catalog,
                                   const QHash<QString, FeatureIntroState> &knownStates,
                                   const QSet<QString> &configuredIds,
                                   const QString &previousVersion,
                                   const QString &currentVersion);

#endif // FEATURE_INTRO_PLANNER_H
