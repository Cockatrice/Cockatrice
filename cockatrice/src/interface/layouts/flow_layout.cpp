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
FlowLayout::FlowLayout(QWidget *parent,
                       const Qt::Orientation _flowDirection,
                       const int margin,
                       const int hSpacing,
                       const int vSpacing)
    : QLayout(parent), flowDirection(_flowDirection), horizontalMargin(hSpacing), verticalMargin(vSpacing)
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
    if (flowDirection == Qt::Vertical) {
        int height = 0;
        int rowWidth = 0;
        int rowHeight = 0;

        for (const QLayoutItem *item : items) {
            if (item == nullptr || item->isEmpty()) {
                continue;
            }

            int itemWidth = item->sizeHint().width() + horizontalSpacing();
            if (rowWidth + itemWidth > width) {
                height += rowHeight + verticalSpacing();
                rowWidth = itemWidth;
                rowHeight = item->sizeHint().height();
            } else {
                rowWidth += itemWidth;
                rowHeight = qMax(rowHeight, item->sizeHint().height());
            }
        }
        height += rowHeight; // Add height of the last row
        return height;
    } else {
        int width = 0;
        int colWidth = 0;
        int colHeight = 0;

        for (const QLayoutItem *item : items) {
            if (item == nullptr || item->isEmpty()) {
                continue;
            }

            int itemHeight = item->sizeHint().height();
            if (colHeight + itemHeight > width) {
                width += colWidth;
                colHeight = itemHeight;
                colWidth = item->sizeHint().width();
            } else {
                colHeight += itemHeight;
                colWidth = qMax(colWidth, item->sizeHint().width());
            }
        }
        width += colWidth; // Add width of the last column
        return width;
    }
}

/**
 * @brief Arranges layout items in rows within the specified rectangle bounds.
 * @param rect The area within which to position layout items.
 */
void FlowLayout::setGeometry(const QRect &rect)
{
    QLayout::setGeometry(rect); // Sets the geometry of the layout based on the given rectangle.

    if (flowDirection == Qt::Horizontal) {
        // If we have a parent scroll area, we're clamped to that, else we use our own rectangle.
        const int availableWidth = getParentScrollAreaWidth() == 0 ? rect.width() : getParentScrollAreaWidth();

        const int totalHeight = layoutAllRows(rect.x(), rect.y(), availableWidth);

        if (QWidget *parentWidgetPtr = parentWidget()) {
            parentWidgetPtr->setFixedSize(availableWidth, totalHeight);
        }
    } else {
        const int availableHeight = qMax(rect.height(), getParentScrollAreaHeight());

        const int totalWidth = layoutAllColumns(rect.x(), rect.y(), availableHeight);

        if (QWidget *parentWidgetPtr = parentWidget()) {
            parentWidgetPtr->setFixedSize(totalWidth, availableHeight);
        }
    }
}

/**
 * @brief Lays out items into rows according to the available width, starting from a given origin.
 *        Each row is arranged within `availableWidth`, wrapping to a new row as necessary.
 * @param originX The x-coordinate for the layout start position.
 * @param originY The y-coordinate for the layout start position.
 * @param availableWidth The width within which each row is constrained.
 * @return The total height after arranging all rows.
 */
int FlowLayout::layoutAllRows(const int originX, const int originY, const int availableWidth)
{
    QVector<QLayoutItem *> rowItems; // Holds items for the current row
    int currentXPosition = originX;  // Tracks the x-coordinate while placing items
    int currentYPosition = originY;  // Tracks the y-coordinate, moving down after each row

    int rowHeight = 0; // Tracks the maximum height of items in the current row

    for (QLayoutItem *item : items) {
        if (item == nullptr || item->isEmpty()) {
            continue;
        }

        QSize itemSize = item->sizeHint();                      // The suggested size for the current item
        int itemWidth = itemSize.width() + horizontalSpacing(); // Item width plus spacing

        // Check if the current item fits in the remaining width of the current row
        if (currentXPosition + itemWidth > availableWidth) {
            // If not, layout the current row and start a new row
            layoutSingleRow(rowItems, originX, currentYPosition);
            rowItems.clear();                                  // Reset the list for the new row
            currentXPosition = originX;                        // Reset x-position to the row's start
            currentYPosition += rowHeight + verticalSpacing(); // Move y-position down to the next row
            rowHeight = 0;                                     // Reset row height for the new row
        }

        // Add the item to the current row
        rowItems.append(item);
        rowHeight = qMax(rowHeight, itemSize.height());      // Update the row's height to the tallest item
        currentXPosition += itemWidth + horizontalSpacing(); // Move x-position for the next item
    }

    // Layout the final row if there are any remaining items
    layoutSingleRow(rowItems, originX, currentYPosition);

    // Return the total height used, including the last row's height
    return currentYPosition + rowHeight;
}

/**
 * @brief Arranges a single row of items within specified x and y starting positions.
 * @param rowItems A list of items to be arranged in the row.
 * @param x The starting x-coordinate for the row.
 * @param y The starting y-coordinate for the row.
 */
void FlowLayout::layoutSingleRow(const QVector<QLayoutItem *> &rowItems, int x, const int y)
{
    for (QLayoutItem *item : rowItems) {
        if (item == nullptr || item->isEmpty()) {
            continue;
        }

        // Get the maximum allowed size for the item
        QSize itemMaxSize = item->widget()->maximumSize();
        // Constrain the item's width and height to its size hint or maximum size
        const int itemWidth = qMin(item->sizeHint().width(), itemMaxSize.width());
        const int itemHeight = qMin(item->sizeHint().height(), itemMaxSize.height());
        // Set the item's geometry based on the computed size and position
        item->setGeometry(QRect(QPoint(x, y), QSize(itemWidth, itemHeight)));
        // Move the x-position to the right, leaving space for horizontal spacing
        x += itemWidth + horizontalSpacing();
    }
}

/**
 * @brief Lays out items into columns according to the available height, starting from a given origin.
 *        Each column is arranged within `availableHeight`, wrapping to a new column as necessary.
 * @param originX The x-coordinate for the layout start position.
 * @param originY The y-coordinate for the layout start position.
 * @param availableHeight The height within which each column is constrained.
 * @return The total width after arranging all columns.
 */
int FlowLayout::layoutAllColumns(const int originX, const int originY, const int availableHeight)
{
    QVector<QLayoutItem *> colItems; // Holds items for the current column
    int currentXPosition = originX;  // Tracks the x-coordinate while placing items
    int currentYPosition = originY;  // Tracks the y-coordinate, resetting for each new column

    int colWidth = 0; // Tracks the maximum width of items in the current column

    for (QLayoutItem *item : items) {
        if (item == nullptr || item->isEmpty()) {
            continue;
        }

        QSize itemSize = item->sizeHint(); // The suggested size for the current item

        // Check if the current item fits in the remaining height of the current column
        if (currentYPosition + itemSize.height() > availableHeight) {
            // If not, layout the current column and start a new column
            layoutSingleColumn(colItems, currentXPosition, originY);
            colItems.clear();             // Reset the list for the new column
            currentYPosition = originY;   // Reset y-position to the column's start
            currentXPosition += colWidth; // Move x-position to the next column
            colWidth = 0;                 // Reset column width for the new column
        }

        // Add the item to the current column
        colItems.append(item);
        colWidth = qMax(colWidth, itemSize.width()); // Update the column's width to the widest item
        currentYPosition += itemSize.height();       // Move y-position for the next item
    }

    // Layout the final column if there are any remaining items
    layoutSingleColumn(colItems, currentXPosition, originY);

    // Return the total width used, including the last column's width
    return currentXPosition + colWidth;
}

/**
 * @brief Arranges a single column of items within specified x and y starting positions.
 * @param colItems A list of items to be arranged in the column.
 * @param x The starting x-coordinate for the column.
 * @param y The starting y-coordinate for the column.
 */
void FlowLayout::layoutSingleColumn(const QVector<QLayoutItem *> &colItems, const int x, int y)
{
    for (QLayoutItem *item : colItems) {
        if (item == nullptr) {
            qCDebug(FlowLayoutLog) << "Item is null.";
            continue;
        }

        if (item->isEmpty()) {
            qCDebug(FlowLayoutLog) << "Skipping empty item.";
            continue;
        }

        // Debugging: Print the item's widget class name and size hint
        QWidget *widget = item->widget();
        if (widget) {
            qCDebug(FlowLayoutLog) << "Widget class:" << widget->metaObject()->className();
            qCDebug(FlowLayoutLog) << "Widget size hint:" << widget->sizeHint();
            qCDebug(FlowLayoutLog) << "Widget maximum size:" << widget->maximumSize();
            qCDebug(FlowLayoutLog) << "Widget minimum size:" << widget->minimumSize();

            // Debugging: Print child widgets
            const QObjectList &children = widget->children();
            qCDebug(FlowLayoutLog) << "Child widgets:";
            for (QObject *child : children) {
                if (QWidget *childWidget = qobject_cast<QWidget *>(child)) {
                    qCDebug(FlowLayoutLog) << "  - Child widget class:" << childWidget->metaObject()->className();
                    qCDebug(FlowLayoutLog) << "    Size hint:" << childWidget->sizeHint();
                    qCDebug(FlowLayoutLog) << "    Maximum size:" << childWidget->maximumSize();
                }
            }
        } else {
            qCDebug(FlowLayoutLog) << "Item does not have a widget.";
        }

        // Get the maximum allowed size for the item
        QSize itemMaxSize = widget->maximumSize();
        // Constrain the item's width and height to its size hint or maximum size
        const int itemWidth = qMin(item->sizeHint().width(), itemMaxSize.width());
        const int itemHeight = qMin(item->sizeHint().height(), itemMaxSize.height());
        // Debugging: Print the computed geometry
        qCDebug(FlowLayoutLog) << "Computed geometry: x=" << x << ", y=" << y << ", width=" << itemWidth
                               << ", height=" << itemHeight;

        // Set the item's geometry based on the computed size and position
        item->setGeometry(QRect(QPoint(x, y), QSize(itemWidth, itemHeight)));

        // Move the y-position down by the item's height to place the next item below
        y += itemHeight;
    }
}

/**
 * @brief Calculates the preferred size of the layout based on the flow direction.
 * @return A QSize representing the ideal dimensions of the layout.
 */
QSize FlowLayout::sizeHint() const
{
    if (flowDirection == Qt::Horizontal) {
        return calculateSizeHintHorizontal();
    } else {
        return calculateSizeHintVertical();
    }
}

/**
 * @brief Calculates the minimum size required by the layout based on the flow direction.
 * @return A QSize representing the minimum required dimensions.
 */
QSize FlowLayout::minimumSize() const
{
    if (flowDirection == Qt::Horizontal) {
        return calculateMinimumSizeHorizontal();
    } else {
        return calculateMinimumSizeVertical();
    }
}

/**
 * @brief Calculates the size hint for horizontal flow direction.
 * @return A QSize representing the preferred dimensions.
 */
QSize FlowLayout::calculateSizeHintHorizontal() const
{
    int maxWidth = 0;     // Tracks the maximum width needed
    int totalHeight = 0;  // Tracks the total height across all rows
    int rowHeight = 0;    // Tracks the height of the current row
    int currentWidth = 0; // Tracks the current row's width

    const int availableWidth = getParentScrollAreaWidth() == 0 ? parentWidget()->width() : getParentScrollAreaWidth();

    qCDebug(FlowLayoutLog) << "Calculating horizontal size hint. Available width:" << availableWidth;

    for (const QLayoutItem *item : items) {
        if (!item || item->isEmpty()) {
            qCDebug(FlowLayoutLog) << "Skipping empty item.";
            continue;
        }

        QSize itemSize = item->sizeHint();
        int itemWidth = itemSize.width() + horizontalSpacing();
        qCDebug(FlowLayoutLog) << "Processing item. Size:" << itemSize << "Width with spacing:" << itemWidth;

        if (currentWidth + itemWidth > availableWidth) {
            qCDebug(FlowLayoutLog) << "Row overflow. Current width:" << currentWidth << "Row height:" << rowHeight;
            maxWidth = qMax(maxWidth, currentWidth);
            totalHeight += rowHeight + verticalSpacing();
            qCDebug(FlowLayoutLog) << "Updated total height:" << totalHeight << "Max width so far:" << maxWidth;

            currentWidth = 0;
            rowHeight = 0;
        }

        currentWidth += itemWidth;
        rowHeight = qMax(rowHeight, itemSize.height());
        qCDebug(FlowLayoutLog) << "Updated current width:" << currentWidth << "Updated row height:" << rowHeight;
    }

    // Account for the final row
    maxWidth = qMax(maxWidth, currentWidth);
    totalHeight += rowHeight;
    qCDebug(FlowLayoutLog) << "Final total height:" << totalHeight << "Final max width:" << maxWidth;

    return QSize(maxWidth, totalHeight);
}

/**
 * @brief Calculates the minimum size for horizontal flow direction.
 * @return A QSize representing the minimum required dimensions.
 */
QSize FlowLayout::calculateMinimumSizeHorizontal() const
{
    int maxWidth = 0;     // Tracks the maximum width of a row
    int totalHeight = 0;  // Tracks the total height across all rows
    int rowHeight = 0;    // Tracks the height of the current row
    int currentWidth = 0; // Tracks the current row's width

    const int availableWidth = getParentScrollAreaWidth() == 0 ? parentWidget()->width() : getParentScrollAreaWidth();

    qCDebug(FlowLayoutLog) << "Calculating horizontal minimum size. Available width:" << availableWidth;

    for (const QLayoutItem *item : items) {
        if (!item || item->isEmpty()) {
            qCDebug(FlowLayoutLog) << "Skipping empty item.";
            continue;
        }

        QSize itemMinSize = item->minimumSize();
        int itemWidth = itemMinSize.width() + horizontalSpacing();
        qCDebug(FlowLayoutLog) << "Processing item. Minimum size:" << itemMinSize << "Width with spacing:" << itemWidth;

        if (currentWidth + itemWidth > availableWidth) {
            qCDebug(FlowLayoutLog) << "Row overflow. Current width:" << currentWidth << "Row height:" << rowHeight;
            maxWidth = qMax(maxWidth, currentWidth);
            totalHeight += rowHeight + verticalSpacing();
            qCDebug(FlowLayoutLog) << "Updated total height:" << totalHeight << "Max width so far:" << maxWidth;

            currentWidth = 0;
            rowHeight = 0;
        }

        currentWidth += itemWidth;
        rowHeight = qMax(rowHeight, itemMinSize.height());
        qCDebug(FlowLayoutLog) << "Updated current width:" << currentWidth << "Updated row height:" << rowHeight;
    }

    // Account for the final row
    maxWidth = qMax(maxWidth, currentWidth);
    totalHeight += rowHeight;
    qCDebug(FlowLayoutLog) << "Final total height:" << totalHeight << "Final max width:" << maxWidth;

    return QSize(maxWidth, totalHeight);
}

/**
 * @brief Calculates the size hint for vertical flow direction.
 * @return A QSize representing the preferred dimensions.
 */
QSize FlowLayout::calculateSizeHintVertical() const
{
    int totalWidth = 0;
    int maxHeight = 0;
    int colWidth = 0;
    int currentHeight = 0;

    const int availableHeight = qMax(parentWidget()->height(), getParentScrollAreaHeight());

    qCDebug(FlowLayoutLog) << "Calculating vertical size hint. Available height:" << availableHeight;

    for (const QLayoutItem *item : items) {
        if (!item || item->isEmpty()) {
            qCDebug(FlowLayoutLog) << "Skipping empty item.";
            continue;
        }

        QSize itemSize = item->sizeHint();
        qCDebug(FlowLayoutLog) << "Processing item. Size:" << itemSize;

        if (currentHeight + itemSize.height() > availableHeight) {
            qCDebug(FlowLayoutLog) << "Column overflow. Current height:" << currentHeight
                                   << "Column width:" << colWidth;
            totalWidth += colWidth + horizontalSpacing();
            maxHeight = qMax(maxHeight, currentHeight);
            qCDebug(FlowLayoutLog) << "Updated total width:" << totalWidth << "Max height so far:" << maxHeight;

            currentHeight = 0;
            colWidth = 0;
        }

        currentHeight += itemSize.height() + verticalSpacing();
        colWidth = qMax(colWidth, itemSize.width());
        qCDebug(FlowLayoutLog) << "Updated current height:" << currentHeight << "Updated column width:" << colWidth;
    }

    // Account for the final column
    totalWidth += colWidth;
    maxHeight = qMax(maxHeight, currentHeight);
    qCDebug(FlowLayoutLog) << "Final total width:" << totalWidth << "Final max height:" << maxHeight;

    return QSize(totalWidth, maxHeight);
}

/**
 * @brief Calculates the minimum size for vertical flow direction.
 * @return A QSize representing the minimum required dimensions.
 */
QSize FlowLayout::calculateMinimumSizeVertical() const
{
    int totalWidth = 0;    // Tracks the total width across all columns
    int maxHeight = 0;     // Tracks the maximum height of a column
    int colWidth = 0;      // Tracks the width of the current column
    int currentHeight = 0; // Tracks the current column's height

    const int availableHeight = qMax(parentWidget()->height(), getParentScrollAreaHeight());

    qCDebug(FlowLayoutLog) << "Calculating vertical minimum size. Available height:" << availableHeight;

    for (const QLayoutItem *item : items) {
        if (!item || item->isEmpty()) {
            qCDebug(FlowLayoutLog) << "Skipping empty item.";
            continue;
        }

        QSize itemMinSize = item->minimumSize();
        int itemHeight = itemMinSize.height() + verticalSpacing();
        qCDebug(FlowLayoutLog) << "Processing item. Minimum size:" << itemMinSize
                               << "Height with spacing:" << itemHeight;

        if (currentHeight + itemHeight > availableHeight) {
            qCDebug(FlowLayoutLog) << "Column overflow. Current height:" << currentHeight
                                   << "Column width:" << colWidth;
            totalWidth += colWidth + horizontalSpacing();
            maxHeight = qMax(maxHeight, currentHeight);
            qCDebug(FlowLayoutLog) << "Updated total width:" << totalWidth << "Max height so far:" << maxHeight;

            currentHeight = 0;
            colWidth = 0;
        }

        currentHeight += itemHeight;
        colWidth = qMax(colWidth, itemMinSize.width());
        qCDebug(FlowLayoutLog) << "Updated current height:" << currentHeight << "Updated column width:" << colWidth;
    }

    // Account for the final column
    totalWidth += colWidth;
    maxHeight = qMax(maxHeight, currentHeight);
    qCDebug(FlowLayoutLog) << "Final total width:" << totalWidth << "Final max height:" << maxHeight;

    return QSize(totalWidth, maxHeight);
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

void FlowLayout::insertWidgetAtIndex(QWidget *toInsert, int index)
{
    addChildWidget(toInsert);

    // We don't want to fail on an index that violates the bounds, so we just clamp it.
    int boundedIndex = qBound(0, index, qMax(0, static_cast<int>(items.size())));
    items.insert(boundedIndex, new QWidgetItem(toInsert));

    invalidate();
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
