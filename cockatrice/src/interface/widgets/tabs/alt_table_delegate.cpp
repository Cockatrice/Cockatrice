#include "alt_table_delegate.h"

#include "../utility/pill_painter.h"

#include <QApplication>
#include <QModelIndex>
#include <QPainter>
#include <QPalette>
#include <QStyle>
#include <QStyleOptionViewItem>

namespace
{
constexpr int ICON_TEXT_GAP = 4;
} // namespace

AltTableDelegate::AltTableDelegate(int viaColumn, int warnsColumn, int bansColumn, int sharedColumn, QObject *parent)
    : QStyledItemDelegate(parent), viaColumn(viaColumn), warnsColumn(warnsColumn), bansColumn(bansColumn),
      sharedColumn(sharedColumn)
{
}

void AltTableDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const bool selected = (option.state & QStyle::State_Selected) != 0;

    QStyleOptionViewItem opt(option);
    initStyleOption(&opt, index);
    painter->setFont(opt.font);
    QStyle *style = opt.widget != nullptr ? opt.widget->style() : QApplication::style();
    const QString text = index.data(Qt::DisplayRole).toString();
    const QPalette::ColorRole emphasisRole = selected ? QPalette::HighlightedText : QPalette::WindowText;

    if (index.column() == viaColumn) {
        bool sharedOk = false;
        const unsigned shared = index.sibling(index.row(), sharedColumn).data(Qt::DisplayRole).toUInt(&sharedOk);
        const bool ipMatch = text.startsWith(QStringLiteral("ip:"));
        // shared_count for an IP match already includes the investigated account,
        // so anything above 2 means the address is shared beyond that pair.
        const bool dimmed = sharedOk && ipMatch && shared > 2;
        const QPalette::ColorRole textRole = (dimmed && !selected) ? QPalette::PlaceholderText : emphasisRole;

        QPalette::ColorRole tintRole = QPalette::NoRole;
        if (text == QStringLiteral("email")) {
            tintRole = QPalette::Link;
        } else if (text.startsWith(QStringLiteral("clientid:"))) {
            tintRole = QPalette::Highlight;
        } else if (ipMatch) {
            tintRole = QPalette::PlaceholderText;
        }
        if (tintRole != QPalette::NoRole) {
            if (!pill_painter::paint(painter, opt, tintRole, textRole, text)) {
                QStyledItemDelegate::paint(painter, opt, index);
            }
            return;
        }
        QStyledItemDelegate::paint(painter, opt, index);
        return;
    }

    if (index.column() == warnsColumn || index.column() == bansColumn) {
        bool countOk = false;
        const int count = text.toInt(&countOk);
        if (countOk && count > 0 && !selected) {
            const QIcon icon = style->standardIcon(index.column() == bansColumn ? QStyle::SP_MessageBoxCritical
                                                                                : QStyle::SP_MessageBoxWarning);
            const int iconSize = qMax(0, opt.rect.height() - 2 * ICON_TEXT_GAP);
            QRect textRect = opt.rect;
            if (!icon.isNull() && iconSize > 0) {
                const QRect iconRect(opt.rect.left() + ICON_TEXT_GAP, opt.rect.top() + ICON_TEXT_GAP, iconSize,
                                     iconSize);
                icon.paint(painter, iconRect);
                textRect.setLeft(iconRect.right() + ICON_TEXT_GAP);
            }
            const QString elided = opt.fontMetrics.elidedText(text, opt.textElideMode, textRect.width());
            style->drawItemText(painter, textRect, Qt::AlignLeft | Qt::AlignVCenter, opt.palette, true, elided,
                                emphasisRole);
            return;
        }
    }

    QStyledItemDelegate::paint(painter, opt, index);
}

QSize AltTableDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const QSize hint = QStyledItemDelegate::sizeHint(option, index);
    if (index.column() == viaColumn) {
        return hint + QSize(pill_painter::PAD_X + 2 * ICON_TEXT_GAP, pill_painter::PAD_Y);
    }
    if (index.column() == warnsColumn || index.column() == bansColumn) {
        return hint + QSize(2 * ICON_TEXT_GAP, 0);
    }
    return hint;
}
