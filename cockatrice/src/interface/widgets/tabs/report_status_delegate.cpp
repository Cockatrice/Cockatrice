#include "report_status_delegate.h"

#include "../utility/pill_painter.h"

#include <QApplication>
#include <QPainter>
#include <QPalette>
#include <QStyle>
#include <QStyleOptionViewItem>

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
    } else if (status == QStringLiteral("resolved")) {
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

    const QPalette::ColorRole textRole = selected ? QPalette::HighlightedText : QPalette::WindowText;
    if (!pill_painter::paint(painter, opt, tintRole, textRole, opt.text.isEmpty() ? status : opt.text)) {
        QStyledItemDelegate::paint(painter, opt, index);
    }
}

QSize ReportStatusDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const QSize hint = QStyledItemDelegate::sizeHint(option, index);
    return hint + QSize(pill_painter::PAD_X, pill_painter::PAD_Y);
}
