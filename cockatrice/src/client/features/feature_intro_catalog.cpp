#include "feature_intro.h"

#include <QCoreApplication>

namespace
{
/**
 * @brief The hover zoom and tabbed card panel rework (PR #7700).
 *
 * Both triggers used to be opt-in and are now `Hover` by default, which is the
 * exact shape that goes unnoticed: the feature is already on, so nothing tells
 * the user it exists and nothing tells them how to get the old hover behavior
 * back. `introducedIn` names the line the work landed in, not the beta it first
 * appeared in, so users already on a `3.2.0` prerelease are baselined while users
 * arriving from `3.1.x` are invited.
 */
FeatureIntro hoverCardPanelIntro()
{
    FeatureIntro intro;
    intro.id = QStringLiteral("cards-hover-panel");
    intro.revision = 1;
    intro.shape = FeatureIntroShape::Rework;
    intro.introducedIn = QStringLiteral("3.2.0");

    intro.title = QCoreApplication::translate("FeatureIntro", "Hover zoom and a tabbed card panel");
    intro.body = QCoreApplication::translate(
        "FeatureIntro",
        "Hover a card in a game to zoom it in place. Pause a moment longer and a tabbed panel opens with its "
        "details, text and actions.");
    intro.acceptLabel = QCoreApplication::translate("FeatureIntro", "Got it");
    intro.declineLabel = QCoreApplication::translate("FeatureIntro", "Use classic");
    intro.declinedOutcome = QCoreApplication::translate("FeatureIntro", "Hover zoom and the card panel are off.");
    intro.badgeTooltip =
        QCoreApplication::translate("FeatureIntro", "New: hover a card in a game to zoom it and open the card panel.");
    intro.settingsPage = QStringLiteral("user-interface");
    // Translated in the context that owns the group title, so the search text and the
    // group title can never disagree in a language where they would differ.
    intro.settingsQuery = QCoreApplication::translate("UserInterfaceSettingsPage", "Hover panel settings");
    return intro;
}
} // namespace

const QVector<FeatureIntro> &featureIntroCatalog()
{
    static const QVector<FeatureIntro> catalog = {hoverCardPanelIntro()};
    return catalog;
}

const FeatureIntro *findFeatureIntro(const QString &id)
{
    for (const FeatureIntro &intro : featureIntroCatalog()) {
        if (intro.id == id) {
            return &intro;
        }
    }
    return nullptr;
}
