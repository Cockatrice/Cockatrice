/**
 * @file settings_search_delegate.cpp
 * @brief Implementation of the custom settings search result delegate
 * @ingroup Dialogs
 */
#include "settings_search_delegate.h"

#include "settings_search_model.h"

#include <QPainter>

SettingsSearchDelegate::SettingsSearchDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
}

void SettingsSearchDelegate::setPageNames(const QStringList &names)
{
    pageNames = names;
}

void SettingsSearchDelegate::paint(QPainter *painter,
                                   const QStyleOptionViewItem &option,
                                   const QModelIndex &index) const
{
    painter->save();

    SettingsSearchEntry entry = index.data(SettingsSearchModel::EntryRole).value<SettingsSearchEntry>();

    bool isSelected = option.state & QStyle::State_Selected;
    bool isHovered = option.state & QStyle::State_MouseOver;

    // Background
    QColor bgColor = isSelected  ? option.palette.color(QPalette::Highlight)
                     : isHovered ? option.palette.color(QPalette::Midlight)
                                 : option.palette.color(QPalette::Base);
    painter->fillRect(option.rect, bgColor);

    int leftMargin = 12;
    int topMargin = 8;
    int rightMargin = 12;
    int bottomMargin = 4;

    QRect contentRect = option.rect.adjusted(leftMargin, topMargin, -rightMargin, -bottomMargin);
    int yPos = contentRect.top();

    // Breadcrumb: "Page > Group"
    QFont breadcrumbFont = option.font;
    breadcrumbFont.setPointSize(breadcrumbFont.pointSize() - 1);
    breadcrumbFont.setBold(true);

    QColor breadcrumbColor =
        isSelected ? option.palette.color(QPalette::HighlightedText) : option.palette.color(QPalette::Text);
    breadcrumbColor.setAlpha(180);

    QString pageName;
    if (entry.pageIndex >= 0 && entry.pageIndex < pageNames.size()) {
        pageName = pageNames[entry.pageIndex];
    } else {
        pageName = QString::number(entry.pageIndex);
    }

    QString breadcrumbText = QStringLiteral("%1 > %2").arg(pageName, entry.groupTitle);
    painter->setFont(breadcrumbFont);
    painter->setPen(breadcrumbColor);
    painter->drawText(QRect(contentRect.left(), yPos, contentRect.width(), 20), Qt::AlignLeft | Qt::AlignVCenter,
                      breadcrumbText);
    yPos += 20;

    // Setting label
    QFont labelFont = option.font;
    labelFont.setPointSize(labelFont.pointSize() + 1);

    QColor labelColor =
        isSelected ? option.palette.color(QPalette::HighlightedText) : option.palette.color(QPalette::Text);

    painter->setFont(labelFont);
    painter->setPen(labelColor);
    painter->drawText(QRect(contentRect.left(), yPos, contentRect.width(), 24), Qt::AlignLeft | Qt::AlignVCenter,
                      entry.widgetLabel);
    yPos += 24;

    // Bottom separator
    QPen separatorPen(option.palette.color(QPalette::Mid), 1);
    painter->setPen(separatorPen);
    painter->drawLine(option.rect.left() + leftMargin, option.rect.bottom(), option.rect.right() - rightMargin,
                      option.rect.bottom());

    painter->restore();
}

QSize SettingsSearchDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);
    return QSize(option.rect.width(), 56);
}
