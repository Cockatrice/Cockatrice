#include "overlap_layout.h"

#include <QDebug>
#include <QtMath>

/**
 * @class OverlapLayout
 * @brief Custom layout class to arrange widgets with overlapping positions.
 *
 * The OverlapLayout class is a QLayout subclass that arranges child widgets
 * in an overlapping configuration, allowing control over the overlap percentage
 * and the number of rows or columns based on the chosen layout direction. This
 * layout is particularly useful for visualizing elements that need to partially
 * stack over one another, either horizontally or vertically.
 */

/**
 * @brief Constructs an OverlapLayout with the specified parameters.
 *
 * Initializes a new OverlapLayout with the given overlap percentage, row or column limit,
 * and layout direction. The overlap percentage determines how much each widget will
 * overlap with the previous one. If maxColumns or maxRows are set to zero, it implies
 * no limit in that respective dimension.
 *
 * @param overlapPercentage An integer representing the percentage of overlap between items (0-100).
 * @param maxColumns The maximum number of columns allowed in the layout when in horizontal orientation (0 for
 * unlimited).
 * @param maxRows The maximum number of rows allowed in the layout when in vertical orientation (0 for unlimited).
 * @param direction The orientation direction of the layout, either Qt::Horizontal or Qt::Vertical.
 * @param parent The parent widget of this layout.
 */
OverlapLayout::OverlapLayout(QWidget *parent,
                             const int overlapPercentage,
                             const int maxColumns,
                             const int maxRows,
                             const Qt::Orientation _overlapDirection,
                             const Qt::Orientation _flowDirection)
    : QLayout(parent), overlapPercentage(overlapPercentage), maxColumns(maxColumns), maxRows(maxRows),
      overlapDirection(_overlapDirection), flowDirection(_flowDirection)
{
}

/**
 * @brief Destructor for OverlapLayout, ensuring cleanup of all layout items.
 *
 * Iterates through all layout items and deletes them. This prevents memory
 * leaks by removing all child QLayoutItems stored in the layout.
 */
OverlapLayout::~OverlapLayout()
{
    QLayoutItem *item;
    while ((item = OverlapLayout::takeAt(0))) {
        delete item;
    }
}

void OverlapLayout::insertWidgetAtIndex(QWidget *toInsert, int index)
{
    addChildWidget(toInsert);
    int clampedIndex = qBound(0, index, qMax(0, static_cast<int>(itemList.size())));
    itemList.insert(clampedIndex, new QWidgetItem(toInsert));

    for (int i = clampedIndex; i < itemList.size(); ++i) {
        dynamic_cast<QWidgetItem *>(itemList.at(i))->widget()->raise();
    }

    invalidate();
}

/**
 * @brief Adds a new item to the layout.
 *
 * Appends a QLayoutItem to the internal list, allowing it to be positioned within the
 * layout during the next geometry update. This method does not directly arrange the
 * items; it merely adds them to the layout’s tracking.
 *
 * @param item Pointer to the QLayoutItem being added to this layout.
 */
void OverlapLayout::addItem(QLayoutItem *item)
{
    if (item != nullptr) {
        itemList.append(item);
    }
}

/**
 * @brief Retrieves the total count of items within the layout.
 *
 * Returns the number of items stored in the layout's internal item list.
 * This count reflects how many widgets or spacers the layout is currently managing.
 *
 * @return Integer count of items in the layout.
 */
int OverlapLayout::count() const
{
    return static_cast<int>(itemList.size());
}

/**
 * @brief Provides access to a layout item at a specified index.
 *
 * Allows retrieval of a QLayoutItem from the layout’s internal list
 * by index. If the index is out of bounds, this function will return nullptr.
 *
 * @param index The index of the desired item.
 * @return Pointer to the QLayoutItem at the specified index, or nullptr if index is invalid.
 */
QLayoutItem *OverlapLayout::itemAt(const int index) const
{
    return (index >= 0 && index < itemList.size()) ? itemList.value(index) : nullptr;
}

/**
 * @brief Removes and returns a layout item at the specified index.
 *
 * Removes a QLayoutItem from the layout at the given index, reducing the layout's count.
 * If the index is invalid, this function returns nullptr without any effect.
 *
 * @param index The index of the item to remove.
 * @return Pointer to the removed QLayoutItem, or nullptr if index is invalid.
 */
QLayoutItem *OverlapLayout::takeAt(const int index)
{
    return (index >= 0 && index < itemList.size()) ? itemList.takeAt(index) : nullptr;
}

/**
 * @brief Sets the geometry for the layout items, arranging them with the specified overlap.
 * @param rect The rectangle defining the area within which the layout should arrange items.
 */
void OverlapLayout::setGeometry(const QRect &rect)
{
    // Call the base class implementation to ensure standard layout behavior.
    QLayout::setGeometry(rect);

    // If there are no items to layout, exit early.
    if (itemList.isEmpty()) {
        return;
    }

    // Get the parent widget for size and margin calculations.
    const QWidget *parentWidget = this->parentWidget();
    if (!parentWidget) {
        return;
    }

    // Calculate available width and height, subtracting the parent's margins.
    int availableWidth = parentWidget->width();
    int availableHeight = parentWidget->height();
    const QMargins margins = parentWidget->contentsMargins();
    availableWidth -= margins.left() + margins.right();
    availableHeight -= margins.top() + margins.bottom();

    // Determine the maximum item width and height among all layout items.
    int maxItemWidth = 0;
    int maxItemHeight = 0;
    for (QLayoutItem *item : itemList) {
        if (item != nullptr && item->widget()) {
            QSize itemSize = item->widget()->sizeHint();
            maxItemWidth = qMax(maxItemWidth, itemSize.width());
            maxItemHeight = qMax(maxItemHeight, itemSize.height());
        }
    }

    // Calculate the overlap offsets based on the layout direction and overlap percentage.
    const int overlapOffsetWidth = (overlapDirection == Qt::Horizontal) ? (maxItemWidth * overlapPercentage / 100) : 0;
    const int overlapOffsetHeight = (overlapDirection == Qt::Vertical) ? (maxItemHeight * overlapPercentage / 100) : 0;

    // Determine the number of columns based on layout constraints and available space.
    int columns;
    if (flowDirection == Qt::Horizontal) {
        if (maxColumns > 0) {
            // Calculate the maximum possible columns given the available width and overlap.
            const int availableColumns = (availableWidth + overlapOffsetWidth) / (maxItemWidth - overlapOffsetWidth);
            // Use the smaller of maxColumns and availableColumns.
            qCDebug(OverlapLayoutLog) << " Max Columns " << maxColumns << " available columns " << availableColumns;
            columns = qMin(maxColumns, availableColumns);
            if (columns < 1) {
                columns = 1;
            }
        } else {
            // If no maxColumns constraint, allow as many columns as possible.
            columns = INT_MAX;
        }
    } else {
        // If not a horizontal layout, column count is irrelevant.
        columns = INT_MAX;
    }

    // Determine the number of rows based on layout constraints and available space.
    int rows;
    if (flowDirection == Qt::Vertical) {
        if (maxRows > 0) {
            // Calculate the maximum possible rows given the available height and overlap.
            const int availableRows = (availableHeight + overlapOffsetHeight) / (maxItemHeight - overlapOffsetHeight);
            // Use the smaller of maxRows and availableRows.
            qCDebug(OverlapLayoutLog) << " Max Rows " << maxRows << " available rows " << availableRows;
            rows = qMin(maxRows, availableRows);
            if (rows < 1) {
                rows = 1;
            }
        } else {
            // If no maxRows constraint, allow as many rows as possible.
            rows = INT_MAX;
        }
    } else {
        // If not a vertical layout, row count is irrelevant.
        rows = INT_MAX;
    }

    // Initialize row and column indices.
    int currentRow = 0;
    int currentColumn = 0;

    // Loop through all items and position them based on the calculated offsets.
    for (const auto item : itemList) {
        if (item == nullptr) {
            continue;
        }

        // Calculate the position of the current item.
        const int xPos = rect.left() + currentColumn * (maxItemWidth - overlapOffsetWidth);
        const int yPos = rect.top() + currentRow * (maxItemHeight - overlapOffsetHeight);
        item->setGeometry(QRect(xPos, yPos, maxItemWidth, maxItemHeight));

        // TODO: Figure this out properly or maybe adjust size hint to account for this?
        // Update row and column indices based on the layout direction.
        if (overlapDirection == Qt::Horizontal) {
            currentColumn++;
            if (currentColumn > qCeil(itemList.size() / rows)) {
                currentColumn = 0;
                currentRow++;
            }
        } else {
            currentRow++;
            if (currentRow > qCeil(itemList.size() / columns)) {
                currentRow = 0;
                currentColumn++;
            }
        }
    }
}

/**
 * @brief Calculates the preferred size for the layout, considering overlap and orientation.
 * @return The preferred layout size as a QSize object.
 */
QSize OverlapLayout::calculatePreferredSize() const
{
    // Get the parent widget for size and margin calculations.
    const QWidget *parentWidget = this->parentWidget();
    if (!parentWidget) {
        return QSize(0, 0);
    }

    if (itemList.isEmpty()) {
        return QSize(0, 0);
    }

    int availableWidth = parentWidget->width();
    int availableHeight = parentWidget->height();
    const QMargins margins = parentWidget->contentsMargins();
    availableWidth -= margins.left() + margins.right();
    availableHeight -= margins.top() + margins.bottom();

    // Determine the maximum item width and height among all layout items.
    int maxItemWidth = 0;
    int maxItemHeight = 0;
    for (QLayoutItem *item : itemList) {
        if (item != nullptr && item->widget()) {
            QSize itemSize = item->widget()->sizeHint();
            maxItemWidth = qMax(maxItemWidth, itemSize.width());
            maxItemHeight = qMax(maxItemHeight, itemSize.height());
        }
    }

    // Calculate the overlap offsets based on the layout direction and overlap percentage.
    const int overlapOffsetWidth = (overlapDirection == Qt::Horizontal) ? (maxItemWidth * overlapPercentage / 100) : 0;
    const int overlapOffsetHeight = (overlapDirection == Qt::Vertical) ? (maxItemHeight * overlapPercentage / 100) : 0;

    // Determine the number of columns based on layout constraints and available space.
    int columns;
    if (flowDirection == Qt::Horizontal) {
        if (maxColumns > 0) {
            // Calculate the maximum possible columns given the available width and overlap.
            const int availableColumns = (availableWidth + overlapOffsetWidth) / (maxItemWidth - overlapOffsetWidth);
            // Use the smaller of maxColumns and availableColumns.
            qCDebug(OverlapLayoutLog) << " Max Columns " << maxColumns << " available columns " << availableColumns;
            columns = qMin(maxColumns, availableColumns);
            if (columns < 1) {
                columns = 1;
            }
        } else {
            // If no maxColumns constraint, allow as many columns as possible.
            columns = INT_MAX;
        }
    } else {
        // If not a horizontal layout, column count is irrelevant.
        columns = INT_MAX;
    }

    // Determine the number of rows based on layout constraints and available space.
    int rows;
    if (flowDirection == Qt::Vertical) {
        if (maxRows > 0) {
            // Calculate the maximum possible rows given the available height and overlap.
            const int availableRows = (availableHeight + overlapOffsetHeight) / (maxItemHeight - overlapOffsetHeight);
            // Use the smaller of maxRows and availableRows.
            qCDebug(OverlapLayoutLog) << " Max Rows " << maxRows << " available rows " << availableRows;
            rows = qMin(maxRows, availableRows);
            if (rows < 1) {
                rows = 1;
            }
        } else {
            // If no maxRows constraint, allow as many rows as possible.
            rows = INT_MAX;
        }
    } else {
        // If not a vertical layout, row count is irrelevant.
        rows = INT_MAX;
    }

    if (overlapDirection == Qt::Horizontal) {
        return QSize(maxItemWidth + ((qCeil(itemList.size() / rows)) * (maxItemWidth - overlapOffsetWidth)),
                     rows * maxItemHeight);
    } else {
        return QSize(columns * maxItemWidth,
                     maxItemHeight + ((qCeil(itemList.size() / columns)) * (maxItemHeight - overlapOffsetHeight)));
    }
}

/**
 * @brief Returns the size hint for the layout, based on preferred size calculations.
 *
 * Provides a recommended size for the layout, useful for layouts that need to fit within
 * a specific parent container size. This takes into account the preferred size and
 * any specific item size requirements.
 *
 * @return The layout's recommended QSize.
 */
QSize OverlapLayout::sizeHint() const
{
    return calculatePreferredSize();
}

/**
 * @brief Provides the minimum size hint for the layout, ensuring functionality within constraints.
 *
 * Defines a minimum workable size for the layout to prevent excessive compression
 * that could distort item arrangement.
 *
 * @return The minimum QSize for this layout.
 */
QSize OverlapLayout::minimumSize() const
{
    return calculatePreferredSize();
}

/**
 * @brief Sets the layout's orientation direction.
 *
 * @param _direction The new orientation direction (Qt::Horizontal or Qt::Vertical).
 */
void OverlapLayout::setDirection(const Qt::Orientation _direction)
{
    overlapDirection = _direction;
}

/**
 * @brief Sets the maximum number of columns for horizontal orientation.
 *
 * @param _maxColumns New maximum column count.
 */
void OverlapLayout::setMaxColumns(const int _maxColumns)
{
    if (_maxColumns >= 0) {
        maxColumns = _maxColumns;
    }
}

/**
 * @brief Sets the maximum number of rows for vertical orientation.
 *
 * @param _maxRows New maximum row count.
 */
void OverlapLayout::setMaxRows(const int _maxRows)
{
    if (_maxRows >= 0) {
        maxRows = _maxRows;
    }
}

/**
 * @brief Calculates the maximum number of columns for a vertical overlap layout based on the current width.
 *
 * This function determines the maximum number of columns that can fit within the layout's width
 * given the overlap percentage and item size, based on the current layout direction.
 *
 * @return Maximum number of columns that can fit within the layout width.
 */
int OverlapLayout::calculateMaxColumns() const
{
    if (overlapDirection != Qt::Vertical || itemList.isEmpty()) {
        return 1; // Only relevant if the layout direction is vertical
    }

    // Determine maximum item width
    int maxItemWidth = 0;
    for (QLayoutItem *item : itemList) {
        if (item == nullptr || !item->widget()) {
            continue;
        }

        QSize itemSize = item->widget()->sizeHint();
        maxItemWidth = qMax(maxItemWidth, itemSize.width());
    }

    const int availableWidth = parentWidget() ? parentWidget()->width() : 0;
    // Determine the maximum number of columns that can fit
    const int columns = availableWidth / maxItemWidth;

    return qMax(1, columns);
}

/**
 * @brief Calculates the maximum number of rows needed for a given number of columns in a vertical overlap layout.
 *
 * Determines how many rows are required to arrange all items given the calculated or specified number of columns.
 *
 * @param columns The number of columns available.
 * @return The total number of rows required.
 */
int OverlapLayout::calculateRowsForColumns(const int columns) const
{
    if (overlapDirection != Qt::Vertical || itemList.isEmpty() || columns <= 0) {
        return 1; // Only relevant if the layout direction is vertical and there are items
    }

    const int totalItems = static_cast<int>(itemList.size());

    return qCeil(totalItems / columns);
}

/**
 * @brief Calculates the maximum number of rows for a horizontal overlap layout based on the current height.
 *
 * This function determines the maximum number of rows that can fit within the layout's height
 * given the overlap percentage and item size, based on the current layout direction.
 *
 * @return Maximum number of rows that can fit within the layout height.
 */
int OverlapLayout::calculateMaxRows() const
{
    if (overlapDirection != Qt::Horizontal || itemList.isEmpty()) {
        return 1; // Only relevant if the layout direction is horizontal
    }

    // Determine maximum item height
    int maxItemHeight = 0;
    for (QLayoutItem *item : itemList) {
        if (item == nullptr || !item->widget()) {
            continue;
        }

        QSize itemSize = item->widget()->sizeHint();
        maxItemHeight = qMax(maxItemHeight, itemSize.height());
    }

    // Calculate the effective height of each item with the overlap applied
    const int overlapOffsetHeight = (maxItemHeight * (100 - overlapPercentage)) / 100;
    const int availableHeight = parentWidget() ? parentWidget()->height() : 0;

    // Determine the maximum number of rows that can fit
    const int rows = availableHeight / overlapOffsetHeight;

    return qMax(1, rows);
}

/**
 * @brief Calculates the maximum number of columns needed for a given number of rows in a horizontal overlap layout.
 *
 * Determines how many columns are required to arrange all items given the calculated or specified number of rows.
 *
 * @param rows The number of rows available.
 * @return The total number of columns required.
 */
int OverlapLayout::calculateColumnsForRows(const int rows) const
{
    if (overlapDirection != Qt::Horizontal || itemList.isEmpty() || rows <= 0) {
        return 1; // Only relevant if the layout direction is horizontal and there are items
    }

    const int totalItems = static_cast<int>(itemList.size());

    return qCeil(totalItems / rows);
}