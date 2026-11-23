#include "feature_intro_badge.h"

#include "../../../client/features/feature_intro.h"
#include "../../../client/features/feature_intro_service.h"

#include <libcockatrice/utility/qt_utils.h>

FeatureIntroBadge::FeatureIntroBadge(const QString &featureId, QWidget *parent) : QToolButton(parent), id(featureId)
{
    setObjectName(QStringLiteral("featureIntroBadge"));
    setText(tr("New"));
    setAutoRaise(true);
    setCursor(Qt::PointingHandCursor);
    // Keyboard reachable: the badge is the only passive pointer at the setting.
    setFocusPolicy(Qt::TabFocus);
    setFont(QtUtils::boldFont(font()));

    // Accent colour, no background: it annotates the group title without competing
    // with the controls underneath, and it follows any theme's Highlight.
    QPalette badgePalette = palette();
    badgePalette.setColor(QPalette::ButtonText, palette().color(QPalette::Highlight));
    setPalette(badgePalette);

    if (const FeatureIntro *intro = findFeatureIntro(featureId)) {
        setToolTip(intro->badgeTooltip);
        setAccessibleName(tr("New feature: %1").arg(intro->title));
    }

    refresh();
}

void FeatureIntroBadge::refresh()
{
    setVisible(findFeatureIntro(id) != nullptr && isFeatureIntroNew(id));
}
