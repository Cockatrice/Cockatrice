#include "feature_intro_planner.h"

#include <QVersionNumber>

namespace
{
/**
 * @brief The numeric prefix of a version string.
 *
 * `VERSION_STRING` carries a prerelease and a build date
 * (`3.2.0-beta.3 (2026-11-10)`). The suffix never changes whether a user has the
 * feature: it landed in the `3.2.0` line either way, so only the numbers matter.
 */
QVersionNumber numericVersion(const QString &version)
{
    return QVersionNumber::fromString(version);
}

/**
 * @brief Whether a feature introduced in @a introducedIn is news for this install.
 *
 * A clean install has no previous version and is not news to anyone: everything it
 * contains was always there. An install that already ran is asked about every entry
 * whose feature this build contains, whether or not the previous version did.
 * Adding a catalog entry is therefore the decision to announce it once, which is
 * what makes a change that lands mid-release-line visible instead of only to users
 * who cross that line.
 */
bool isNewsForInstall(const QString &previousVersion, const QString &introducedIn, const QString &currentVersion)
{
    if (previousVersion.isEmpty()) {
        return false;
    }

    const QVersionNumber introduced = numericVersion(introducedIn);
    const QVersionNumber current = numericVersion(currentVersion);
    if (introduced.isNull() || current.isNull()) {
        // An unparsable version would otherwise compare as "older than everything"
        // and queue the entry on every launch.
        return false;
    }

    return introduced <= current;
}
} // namespace

FeatureIntroPlan planFeatureIntros(const QVector<FeatureIntro> &catalog,
                                   const QHash<QString, FeatureIntroState> &knownStates,
                                   const QSet<QString> &configuredIds,
                                   const QString &previousVersion,
                                   const QString &currentVersion)
{
    FeatureIntroPlan plan;
    plan.writes.reserve(catalog.size());

    for (const FeatureIntro &entry : catalog) {
        const bool configured = configuredIds.contains(entry.id);
        const auto stored = knownStates.constFind(entry.id);

        FeatureIntroState state;
        bool write = false;

        if (stored == knownStates.cend()) {
            write = true;
            state.revision = entry.revision;
            if (configured) {
                state.decision = FeatureIntroDecision::Skipped;
            } else if (isNewsForInstall(previousVersion, entry.introducedIn, currentVersion)) {
                state.decision = FeatureIntroDecision::Pending;
            } else {
                state.decision = FeatureIntroDecision::Baseline;
            }
        } else {
            state = *stored;
            if (state.revision < entry.revision) {
                // The feature changed under the user: ask once more, then record the
                // revision so the next launch finds nothing to do.
                write = true;
                state.decision = configured ? FeatureIntroDecision::Skipped : FeatureIntroDecision::Pending;
                state.revision = entry.revision;
                state.decidedAt = 0;
            }
        }

        if (write) {
            plan.writes.append({entry.id, state});
        }

        if (state.decision == FeatureIntroDecision::Pending) {
            plan.due.append(entry.id);
        }
    }

    return plan;
}
