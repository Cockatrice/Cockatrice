#include "pill_painter.h"

#include <QApplication>
#include <QFontMetrics>
#include <QPainter>
#include <QRect>
#include <QStyle>
#include <QStyleOptionViewItem>

namespace
{
constexpr qreal PILL_ALPHA = 0.30;
} // namespace

bool pill_painter::paint(QPainter *painter,
                         const QStyleOptionViewItem &option,
                         QPalette::ColorRole tintRole,
                         QPalette::ColorRole textRole,
                         const QString &text)
{
    const QRect pillRect = option.rect.adjusted(PAD_X / 2, PAD_Y / 2, -PAD_X / 2, -PAD_Y / 2);
    if (pillRect.width() <= 0 || pillRect.height() <= 0) {
        return false;
    }

    if ((option.state & QStyle::State_Selected) == 0) {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);
        QColor pill = option.palette.color(tintRole);
        pill.setAlphaF(PILL_ALPHA);
        painter->setPen(Qt::NoPen);
        painter->setBrush(pill);
        painter->drawRoundedRect(pillRect, pillRect.height() / 2.0, pillRect.height() / 2.0);
        painter->restore();
    }

    QStyle *style = option.widget != nullptr ? option.widget->style() : QApplication::style();
    const QString elided = option.fontMetrics.elidedText(text, option.textElideMode, pillRect.width());
    style->drawItemText(painter, pillRect, Qt::AlignCenter, option.palette, true, elided, textRole);
    return true;
}
