#include "alt_table_delegate.h"

#include <QApplication>
#include <QModelIndex>
#include <QPainter>
#include <QPalette>
#include <QStyle>
#include <QStyleOptionViewItem>

namespace
{
constexpr qreal PILL_ALPHA = 0.30;
constexpr int PILL_PAD_X = 10;
constexpr int PILL_PAD_Y = 4;
constexpr int ICON_TEXT_GAP = 4;
} // namespace

AltTableDelegate::AltTableDelegate(int viaColumn, int warnsColumn, int bansColumn, int sharedColumn, QObject *parent)
    : QStyledItemDelegate(parent), viaColumn(viaColumn), warnsColumn(warnsColumn), bansColumn(bansColumn),
      sharedColumn(sharedColumn)
{
}

void AltTableDelegate::setVpnThreshold(int threshold)
{
    vpnThreshold = threshold;
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

    auto paintPill = [&](QPalette::ColorRole tintRole, QPalette::ColorRole textRole, const QIcon &icon,
                         const QString &pillText) {
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

        QRect textRect = pillRect;
        if (!icon.isNull()) {
            const int iconSize = qMin(pillRect.height(), pillRect.width());
            if (iconSize > 0) {
                const QRect iconRect(pillRect.left() + ICON_TEXT_GAP, pillRect.top(), iconSize, iconSize);
                icon.paint(painter, iconRect);
                textRect.setLeft(iconRect.right() + ICON_TEXT_GAP);
            }
        }
        const QString elided = opt.fontMetrics.elidedText(pillText, opt.textElideMode, textRect.width());
        style->drawItemText(painter, textRect, Qt::AlignCenter, opt.palette, true, elided, textRole);
    };

    if (index.column() == viaColumn) {
        bool sharedOk = false;
        const unsigned shared = index.sibling(index.row(), sharedColumn).data(Qt::DisplayRole).toUInt(&sharedOk);
        const bool dimmed = sharedOk && text.startsWith(QStringLiteral("ip:")) && shared > 1;
        const bool vpn =
            sharedOk && text.startsWith(QStringLiteral("ip:")) && shared > static_cast<unsigned>(vpnThreshold);
        const QPalette::ColorRole textRole = (dimmed && !selected) ? QPalette::PlaceholderText : emphasisRole;
        if (text == QStringLiteral("email")) {
            paintPill(QPalette::Link, emphasisRole, QIcon(), text);
            return;
        }
        if (text.startsWith(QStringLiteral("clientid:"))) {
            paintPill(QPalette::Highlight, emphasisRole, QIcon(), text);
            return;
        }
        if (text.startsWith(QStringLiteral("ip:"))) {
            if (vpn) {
                paintPill(QPalette::Highlight, textRole, style->standardIcon(QStyle::SP_MessageBoxWarning), text);
                return;
            }
            paintPill(QPalette::PlaceholderText, textRole, QIcon(), text);
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
        return hint + QSize(PILL_PAD_X + 2 * ICON_TEXT_GAP, PILL_PAD_Y);
    }
    if (index.column() == warnsColumn || index.column() == bansColumn) {
        return hint + QSize(2 * ICON_TEXT_GAP, 0);
    }
    return hint;
}
