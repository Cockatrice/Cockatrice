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
FlowLayout::FlowLayout(QWidget *parent, const int margin, const int hSpacing, const int vSpacing)
    : QLayout(parent), horizontalMargin(hSpacing), verticalMargin(vSpacing)
{
    setContentsMargins(margin, margin, margin, margin);
}

/**
 * @brief Destructor for FlowLayout, which cleans up all items in the layout.
 */
FlowLayout::~FlowLayout()
{
    QLayoutItem *item;
    while ((item = FlowLayout::takeAt(0))) {
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
int FlowLayout::heightForWidth(const int width) const
{
    int height = 0;
    int rowWidth = 0;
    int rowHeight = 0;

    for (const QLayoutItem *item : items) {
        if (item == nullptr || item->isEmpty()) {
            continue;
        }

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
    QLayout::setGeometry(rect); // Sets the geometry of the layout based on the given rectangle.

    int left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom); // Retrieves the layout's content margins.

    // Adjust the rectangle to exclude margins.
    const QRect adjustedRect = rect.adjusted(+left, +top, -right, -bottom);

    // Calculate the available width for items, considering either the adjusted rectangle's width
    // or the parent scroll area width, if applicable.
    const int availableWidth = qMax(adjustedRect.width(), getParentScrollAreaWidth());

    // Arrange all rows of items within the available width and get the total height used.
    const int totalHeight = layoutAllRows(adjustedRect.x(), adjustedRect.y(), availableWidth);

    // If the layout's parent is a QWidget, update its minimum size to ensure it can accommodate
    // the arranged items' dimensions.
    if (QWidget *parentWidgetPtr = parentWidget()) {
        parentWidgetPtr->setMinimumSize(availableWidth, totalHeight);
    }
}

/**
 * @brief Arranges items in rows based on the available width.
 *        Items are added to a row until the row's width exceeds `availableWidth`.
 *        Then, a new row is started.
 * @param originX The starting x-coordinate for the row layout.
 * @param originY The starting y-coordinate for the row layout.
 * @param availableWidth The available width to lay out items.
 * @return The y-coordinate of the final row's end position.
 */
int FlowLayout::layoutAllRows(const int originX, const int originY, const int availableWidth)
{
    QVector<QLayoutItem *> rowItems; // Temporary storage for items in the current row.
    int currentXPosition = originX;  // Tracks the x-coordinate for placing items in the current row.
    int currentYPosition = originY;  // Tracks the y-coordinate, updated after each row.

    int rowHeight = 0; // Tracks the maximum height of items in the current row.

    // Iterate through all layout items to arrange them.
    for (QLayoutItem *item : items) {
        if (item == nullptr || item->isEmpty()) {
            continue;
        }

        QSize itemSize = item->sizeHint(); // The suggested size for the item.
        const int itemWidth = itemSize.width() + horizontalSpacing();

        // Check if the item fits in the current row's remaining width.
        if (currentXPosition + itemWidth > availableWidth) {
            // If not, layout the current row and start a new row.
            layoutSingleRow(rowItems, originX, currentYPosition);
            rowItems.clear();                                  // Clear the temporary storage for the new row.
            currentXPosition = originX;                        // Reset x-position to the start of the new row.
            currentYPosition += rowHeight + verticalSpacing(); // Move y-position down for the new row.
            rowHeight = 0;                                     // Reset row height for the new row.
        }

        // Add the item to the current row.
        rowItems.append(item);
        rowHeight = qMax(rowHeight, itemSize.height());             // Update the row height to the tallest item.
        currentXPosition += itemSize.width() + horizontalSpacing(); // Move x-position for the next item.
    }

    // Layout the final row if there are remaining items.
    layoutSingleRow(rowItems, originX, currentYPosition);

    currentYPosition += rowHeight; // Add the final row's height
    return currentYPosition;
}

/**
 * @brief Helper function for arranging a single row of items within specified bounds.
 * @param rowItems Items to be arranged in the row.
 * @param x The x-coordinate for starting the row.
 * @param y The y-coordinate for starting the row.
 */
void FlowLayout::layoutSingleRow(const QVector<QLayoutItem *> &rowItems, int x, const int y)
{
    // Iterate through each item in the row and position it.
    for (QLayoutItem *item : rowItems) {
        if (item == nullptr || item->isEmpty()) {
            continue;
        }

        QSize itemMaxSize = item->widget()->maximumSize(); // Get the item's maximum allowable size.
        // Constrain the item's width and height to its size hint or maximum size.
        int itemWidth = qMin(item->sizeHint().width(), itemMaxSize.width());
        int itemHeight = qMin(item->sizeHint().height(), itemMaxSize.height());
        // Set the item's geometry based on the calculated size and position.
        item->setGeometry(QRect(QPoint(x, y), QSize(itemWidth, itemHeight)));
        // Move the x-position for the next item, including horizontal spacing.
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
    for (const QLayoutItem *item : items) {
        if (item != nullptr) {
            if (!item->isEmpty()) {
                size = size.expandedTo(item->sizeHint());
            }
        }
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
    for (const QLayoutItem *item : items) {
        if (item != nullptr) {
            if (!item->isEmpty()) {
                size = size.expandedTo(item->minimumSize());
            }
        }
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
    if (item != nullptr) {
        items.append(item);
    }
}

/**
 * @brief Retrieves the count of items in the layout.
 * @return The number of layout items.
 */
int FlowLayout::count() const
{
    return static_cast<int>(items.size());
}

/**
 * @brief Returns the layout item at the specified index.
 * @param index The index of the item to retrieve.
 * @return A pointer to the item at the specified index, or nullptr if out of range.
 */
QLayoutItem *FlowLayout::itemAt(const int index) const
{
    return (index >= 0 && index < items.size()) ? items.value(index) : nullptr;
}

/**
 * @brief Removes and returns the item at the specified index.
 * @param index The index of the item to remove.
 * @return A pointer to the removed item, or nullptr if out of range.
 */
QLayoutItem *FlowLayout::takeAt(const int index)
{
    return (index >= 0 && index < items.size()) ? items.takeAt(index) : nullptr;
}

/**
 * @brief Gets the horizontal spacing between items.
 * @return The horizontal spacing if set, otherwise a smart default.
 */
int FlowLayout::horizontalSpacing() const
{
    return (horizontalMargin >= 0) ? horizontalMargin : smartSpacing(QStyle::PM_LayoutHorizontalSpacing);
}

/**
 * @brief Gets the vertical spacing between items.
 * @return The vertical spacing if set, otherwise a smart default.
 */
int FlowLayout::verticalSpacing() const
{
    return (verticalMargin >= 0) ? verticalMargin : smartSpacing(QStyle::PM_LayoutVerticalSpacing);
}

/**
 * @brief Calculates smart spacing based on the parent widget style.
 * @param pm The pixel metric to calculate.
 * @return The calculated spacing value.
 */
int FlowLayout::smartSpacing(const QStyle::PixelMetric pm) const
{
    QObject *parent = this->parent();

    if (!parent) {
        return -1;
    }

    if (parent->isWidgetType()) {
        const auto *pw = dynamic_cast<QWidget *>(parent);
        return pw->style()->pixelMetric(pm, nullptr, pw);
    }

    return dynamic_cast<QLayout *>(parent)->spacing();
}

/**
 * @brief Gets the width of the parent scroll area, if any.
 * @return The width of the scroll area's viewport, or 0 if not found.
 */
int FlowLayout::getParentScrollAreaWidth() const
{
    QWidget *parent = parentWidget();

    while (parent) {
        if (const auto *scrollArea = qobject_cast<QScrollArea *>(parent)) {
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
        if (const auto *scrollArea = qobject_cast<QScrollArea *>(parent)) {
            return scrollArea->viewport()->height();
        }
        parent = parent->parentWidget();
    }

    return 0;
}
