#include "moderation_tab_style.h"

#include <QPalette>
#include <QStyleOption>

namespace
{
constexpr int UNDERLINE_HEIGHT = 2;
constexpr qreal HOVER_WASH_ALPHA = 0.08;
} // namespace

ModerationTabStyle::ModerationTabStyle(QStyle *base) : QProxyStyle(base)
{
}

void ModerationTabStyle::drawControl(ControlElement element,
                                     const QStyleOption *option,
                                     QPainter *painter,
                                     const QWidget *widget) const
{
    if (element != CE_TabBarTabShape || option == nullptr || painter == nullptr) {
        QProxyStyle::drawControl(element, option, painter, widget);
        return;
    }

    const auto *tabOpt = qstyleoption_cast<const QStyleOptionTab *>(option);
    if (tabOpt == nullptr) {
        QProxyStyle::drawControl(element, option, painter, widget);
        return;
    }

    painter->save();
    painter->fillRect(tabOpt->rect, tabOpt->palette.window());

    const bool enabled = (tabOpt->state & State_Enabled) != 0;
    if ((tabOpt->state & State_Selected) != 0) {
        const QRect underline(tabOpt->rect.left(), tabOpt->rect.bottom() - UNDERLINE_HEIGHT + 1,
                              tabOpt->rect.width(), UNDERLINE_HEIGHT);
        painter->fillRect(underline, tabOpt->palette.color(QPalette::Highlight));
    } else if (enabled && (tabOpt->state & State_MouseOver) != 0) {
        QColor wash = tabOpt->palette.color(QPalette::Highlight);
        wash.setAlphaF(HOVER_WASH_ALPHA);
        painter->fillRect(tabOpt->rect, wash);
    }
    painter->restore();
}
