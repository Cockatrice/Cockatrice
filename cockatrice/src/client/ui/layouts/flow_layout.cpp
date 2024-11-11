/**
 * @file flow_layout.cpp
 * @brief Implementation of the FlowLayout class, a custom layout for dynamically organizing widgets
 * in rows within the constraints of available width or parent scroll areas.
 */

#include "flow_layout.h"

#include "../widgets/general/layout_containers/flow_widget.h"

#include <QDebug>
#include <QLayoutItem>
#include <QScrollArea>
#include <QStyle>

/**
 * @brief Constructs a FlowLayout instance with the specified parent widget, margin, and spacing values.
 * @param parent The parent widget for this layout.
 * @param margin The layout margin.
 * @param hSpacing The horizontal spacing between items.
 * @param vSpacing The vertical spacing between items.
 */
FlowLayout::FlowLayout(QWidget *parent, int margin, int hSpacing, int vSpacing)
    : QLayout(parent), m_hSpace(hSpacing), m_vSpace(vSpacing)
{
    setContentsMargins(margin, margin, margin, margin);
}

/**
 * @brief Destructor for FlowLayout, which cleans up all items in the layout.
 */
FlowLayout::~FlowLayout()
{
    QLayoutItem *item;
    while ((item = takeAt(0))) {
        delete item;
    }
}

/**
 * @brief Indicates the layout's support for expansion in both horizontal and vertical directions.
 * @return The orientations (Qt::Horizontal | Qt::Vertical) this layout can expand to fill.
 */
Qt::Orientations FlowLayout::expandingDirections() const
{
    return Qt::Horizontal | Qt::Vertical;
}

/**
 * @brief Indicates that this layout's height depends on its width.
 * @return True, as the layout adjusts its height to fit the specified width.
 */
bool FlowLayout::hasHeightForWidth() const
{
    return true;
}

/**
 * @brief Calculates the required height to display all items within the specified width.
 * @param width The available width for arranging items.
 * @return The total height needed to fit all items in rows constrained by the specified width.
 */
int FlowLayout::heightForWidth(int width) const
{
    int height = 0;
    int rowWidth = 0;
    int rowHeight = 0;

    for (QLayoutItem *item : items) {
        int itemWidth = item->sizeHint().width() + horizontalSpacing();
        if (rowWidth + itemWidth > width) { // Start a new row if the row width exceeds available width
            height += rowHeight + verticalSpacing();
            rowWidth = itemWidth;
            rowHeight = item->sizeHint().height() + verticalSpacing();
        } else {
            rowWidth += itemWidth;
            rowHeight = qMax(rowHeight, item->sizeHint().height());
        }
    }
    height += rowHeight; // Add the final row's height
    return height;
}

/**
 * @brief Arranges layout items in rows within the specified rectangle bounds.
 * @param rect The area within which to position layout items.
 */
void FlowLayout::setGeometry(const QRect &rect)
{
    QLayout::setGeometry(rect);
    int left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);
    QRect adjustedRect = rect.adjusted(+left, +top, -right, -bottom);
    int availableWidth = qMax(adjustedRect.width(), getParentScrollAreaWidth());
    int totalHeight = layoutRows(adjustedRect.x(), adjustedRect.y(), availableWidth);

    QWidget *parentWidgetPtr = parentWidget();
    if (parentWidgetPtr) {
        parentWidgetPtr->setMinimumSize(availableWidth, totalHeight);
    }
}

/**
 * @brief Arranges items in rows based on the available width.
 * @param originX The starting x-coordinate for the row layout.
 * @param originY The starting y-coordinate for the row layout.
 * @param availableWidth The available width to lay out items.
 * @return The y-coordinate of the final row's end position.
 */
int FlowLayout::layoutRows(int originX, int originY, int availableWidth)
{
    QVector<QLayoutItem *> rowItems;
    int currentXPosition = originX;
    int currentYPosition = originY;

    int rowWidth = 0;
    int rowHeight = 0;

    for (QLayoutItem *item : items) {
        QSize itemSize = item->sizeHint();
        int itemWidth = itemSize.width() + horizontalSpacing();

        if (currentXPosition + itemWidth > availableWidth) {
            layoutRow(rowItems, originX, currentYPosition);
            rowItems.clear();
            currentXPosition = originX;
            currentYPosition += rowHeight + verticalSpacing();
            rowWidth = 0;
            rowHeight = 0;
        }
        rowItems.append(item);
        rowWidth += itemSize.width() + horizontalSpacing();
        rowHeight = qMax(rowHeight, itemSize.height());
        currentXPosition += itemSize.width() + horizontalSpacing();
    }

    layoutRow(rowItems, originX, currentYPosition);

    return currentYPosition + rowHeight;
}

/**
 * @brief Helper function for arranging a single row of items within specified bounds.
 * @param rowItems Items to be arranged in the row.
 * @param x The x-coordinate for starting the row.
 * @param y The y-coordinate for starting the row.
 */
void FlowLayout::layoutRow(const QVector<QLayoutItem *> &rowItems, int x, int y)
{
    for (QLayoutItem *item : rowItems) {
        QSize itemMaxSize = item->widget()->maximumSize();
        int itemWidth = qMin(item->sizeHint().width(), itemMaxSize.width());
        int itemHeight = qMin(item->sizeHint().height(), itemMaxSize.height());
        item->setGeometry(QRect(QPoint(x, y), QSize(itemWidth, itemHeight)));
        x += itemWidth + horizontalSpacing();
    }
}

/**
 * @brief Returns the preferred size for this layout.
 * @return The maximum of all item size hints as a QSize.
 */
QSize FlowLayout::sizeHint() const
{
    QSize size;
    for (QLayoutItem *item : items) {
        size = size.expandedTo(item->sizeHint());
    }
    return size.isValid() ? size : QSize(0, 0);
}

/**
 * @brief Returns the minimum size required to display all layout items.
 * @return The minimum QSize needed by the layout.
 */
QSize FlowLayout::minimumSize() const
{
    QSize size;
    for (QLayoutItem *item : items) {
        size = size.expandedTo(item->minimumSize());
    }

    size.setWidth(qMin(size.width(), getParentScrollAreaWidth()));
    size.setHeight(qMin(size.height(), getParentScrollAreaHeight()));

    return size.isValid() ? size : QSize(0, 0);
}

/**
 * @brief Adds a new item to the layout.
 * @param item The layout item to add.
 */
void FlowLayout::addItem(QLayoutItem *item)
{
    items.append(item);
}

/**
 * @brief Retrieves the count of items in the layout.
 * @return The number of layout items.
 */
int FlowLayout::count() const
{
    return items.size();
}

/**
 * @brief Returns the layout item at the specified index.
 * @param index The index of the item to retrieve.
 * @return A pointer to the item at the specified index, or nullptr if out of range.
 */
QLayoutItem *FlowLayout::itemAt(int index) const
{
    return items.value(index);
}

/**
 * @brief Removes and returns the item at the specified index.
 * @param index The index of the item to remove.
 * @return A pointer to the removed item, or nullptr if out of range.
 */
QLayoutItem *FlowLayout::takeAt(int index)
{
    return (index >= 0 && index < items.size()) ? items.takeAt(index) : nullptr;
}

/**
 * @brief Gets the horizontal spacing between items.
 * @return The horizontal spacing if set, otherwise a smart default.
 */
int FlowLayout::horizontalSpacing() const
{
    return (m_hSpace >= 0) ? m_hSpace : smartSpacing(QStyle::PM_LayoutHorizontalSpacing);
}

/**
 * @brief Gets the vertical spacing between items.
 * @return The vertical spacing if set, otherwise a smart default.
 */
int FlowLayout::verticalSpacing() const
{
    return (m_vSpace >= 0) ? m_vSpace : smartSpacing(QStyle::PM_LayoutVerticalSpacing);
}

/**
 * @brief Calculates smart spacing based on the parent widget style.
 * @param pm The pixel metric to calculate.
 * @return The calculated spacing value.
 */
int FlowLayout::smartSpacing(QStyle::PixelMetric pm) const
{
    QObject *parent = this->parent();
    if (!parent) {
        return -1;
    } else if (parent->isWidgetType()) {
        QWidget *pw = static_cast<QWidget *>(parent);
        return pw->style()->pixelMetric(pm, nullptr, pw);
    } else {
        return static_cast<QLayout *>(parent)->spacing();
    }
}

/**
 * @brief Gets the width of the parent scroll area, if any.
 * @return The width of the scroll area's viewport, or 0 if not found.
 */
int FlowLayout::getParentScrollAreaWidth() const
{
    QWidget *parent = parentWidget();
    while (parent) {
        if (QScrollArea *scrollArea = qobject_cast<QScrollArea *>(parent)) {
            return scrollArea->viewport()->width();
        }
        parent = parent->parentWidget();
    }
    return 0;
}

/**
 * @brief Gets the height of the parent scroll area, if any.
 * @return The height of the scroll area's viewport, or 0 if not found.
 */
int FlowLayout::getParentScrollAreaHeight() const
{
    QWidget *parent = parentWidget();
    while (parent) {
        if (QScrollArea *scrollArea = qobject_cast<QScrollArea *>(parent)) {
            return scrollArea->viewport()->height();
        }
        parent = parent->parentWidget();
    }
    return 0;
}
