#include "moderation_header_style.h"

#include <QApplication>
#include <QPainter>
#include <QPalette>
#include <QStyleOption>

namespace
{
constexpr qreal HOVER_WASH_ALPHA = 0.12;
constexpr int SORT_ARROW_GAP = 4;
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

    const bool hasSortIndicator = headerOpt->sortIndicator != QStyleOptionHeader::None;
    QStyleOptionHeader arrowOpt;
    if (hasSortIndicator) {
        arrowOpt = *headerOpt;
        arrowOpt.rect = proxy()->subElementRect(SE_HeaderArrow, headerOpt, widget);
        textRect.setRight(qMin(textRect.right(), arrowOpt.rect.left() - SORT_ARROW_GAP));
    }

    const QString elided = headerOpt->fontMetrics.elidedText(headerOpt->text, Qt::ElideRight, textRect.width());
    QStyle *style = widget != nullptr ? widget->style() : QApplication::style();
    style->drawItemText(painter, textRect, headerOpt->textAlignment, headerOpt->palette, enabled, elided,
                        QPalette::WindowText);

    if (hasSortIndicator) {
        proxy()->drawPrimitive(PE_IndicatorHeaderArrow, &arrowOpt, painter, widget);
    }
}
