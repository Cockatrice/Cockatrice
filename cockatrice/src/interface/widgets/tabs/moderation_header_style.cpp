#include "moderation_header_style.h"

#include <QApplication>
#include <QPainter>
#include <QPalette>
#include <QStyleOption>

namespace
{
constexpr qreal HOVER_WASH_ALPHA = 0.12;
} // namespace

ModerationHeaderStyle::ModerationHeaderStyle(QStyle *base) : QProxyStyle(base)
{
}

void ModerationHeaderStyle::drawControl(ControlElement element,
                                        const QStyleOption *option,
                                        QPainter *painter,
                                        const QWidget *widget) const
{
    if (element != CE_Header || option == nullptr || painter == nullptr) {
        QProxyStyle::drawControl(element, option, painter, widget);
        return;
    }

    const auto *headerOpt = qstyleoption_cast<const QStyleOptionHeader *>(option);
    if (headerOpt == nullptr || !headerOpt->icon.isNull()) {
        QProxyStyle::drawControl(element, option, painter, widget);
        return;
    }

    painter->save();
    painter->fillRect(headerOpt->rect, headerOpt->palette.window());

    const bool enabled = (headerOpt->state & State_Enabled) != 0;
    if (enabled && (headerOpt->state & State_MouseOver) != 0) {
        QColor wash = headerOpt->palette.color(QPalette::Highlight);
        wash.setAlphaF(HOVER_WASH_ALPHA);
        painter->fillRect(headerOpt->rect, wash);
    }

    painter->setPen(headerOpt->palette.color(QPalette::Mid));
    painter->drawLine(headerOpt->rect.bottomLeft(), headerOpt->rect.bottomRight());
    painter->restore();

    QRect textRect = headerOpt->rect.adjusted(4, 0, -4, 0);
    const QString elided =
        headerOpt->fontMetrics.elidedText(headerOpt->text, Qt::ElideRight, textRect.width());
    QStyle *style = widget != nullptr ? widget->style() : QApplication::style();
    style->drawItemText(painter, textRect, Qt::AlignLeft | Qt::AlignVCenter, headerOpt->palette, enabled, elided,
                        QPalette::WindowText);

    if (headerOpt->sortIndicator != QStyleOptionHeader::None) {
        QProxyStyle::drawPrimitive(PE_IndicatorHeaderArrow, option, painter, widget);
    }
}
