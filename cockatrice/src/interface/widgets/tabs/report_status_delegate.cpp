#include "report_status_delegate.h"

#include <QApplication>
#include <QPainter>
#include <QPalette>
#include <QStyle>
#include <QStyleOptionViewItem>

namespace
{
constexpr qreal PILL_ALPHA = 0.30;
constexpr int PILL_PAD_X = 10;
constexpr int PILL_PAD_Y = 4;
} // namespace

ReportStatusDelegate::ReportStatusDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
}

void ReportStatusDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const bool selected = (option.state & QStyle::State_Selected) != 0;
    QString status = index.data(Qt::UserRole).toString();
    if (status.isEmpty()) {
        status = index.data(Qt::DisplayRole).toString();
    }

    QPalette::ColorRole tintRole = QPalette::NoRole;
    if (status == QStringLiteral("open")) {
        tintRole = QPalette::Highlight;
    } else if (status == QStringLiteral("assigned")) {
        tintRole = QPalette::Link;
    } else if (status == QStringLiteral("resolved") || status == QStringLiteral("dismissed")) {
        tintRole = QPalette::PlaceholderText;
    }

    QStyleOptionViewItem opt(option);
    initStyleOption(&opt, index);
    painter->setFont(opt.font);

    if (tintRole == QPalette::NoRole) {
        const QPalette::ColorRole textRole = selected ? QPalette::HighlightedText : QPalette::PlaceholderText;
        QStyle *fallbackStyle = opt.widget != nullptr ? opt.widget->style() : QApplication::style();
        fallbackStyle->drawItemText(painter, opt.rect, Qt::AlignLeft | Qt::AlignVCenter, opt.palette, true, opt.text,
                                    textRole);
        return;
    }

    const QRect pillRect = opt.rect.adjusted(PILL_PAD_X / 2, PILL_PAD_Y / 2, -PILL_PAD_X / 2, -PILL_PAD_Y / 2);
    if (pillRect.width() <= 0 || pillRect.height() <= 0) {
        QStyledItemDelegate::paint(painter, opt, index);
        return;
    }

    if (!selected) {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);
        QColor pill = opt.palette.color(tintRole);
        pill.setAlphaF(PILL_ALPHA);
        painter->setPen(Qt::NoPen);
        painter->setBrush(pill);
        painter->drawRoundedRect(pillRect, pillRect.height() / 2.0, pillRect.height() / 2.0);
        painter->restore();
    }

    const QString elided =
        opt.fontMetrics.elidedText(opt.text.isEmpty() ? status : opt.text, opt.textElideMode, pillRect.width());
    const QPalette::ColorRole textRole = selected ? QPalette::HighlightedText : QPalette::WindowText;
    QStyle *style = opt.widget != nullptr ? opt.widget->style() : QApplication::style();
    style->drawItemText(painter, pillRect, Qt::AlignCenter, opt.palette, true, elided, textRole);
}

QSize ReportStatusDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const QSize hint = QStyledItemDelegate::sizeHint(option, index);
    return hint + QSize(PILL_PAD_X, PILL_PAD_Y);
}
