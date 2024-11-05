#include "overlap_layout.h"

#include <QDebug>

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
 * @param maxColumns The maximum number of columns allowed in the layout when in horizontal orientation (0 for unlimited).
 * @param maxRows The maximum number of rows allowed in the layout when in vertical orientation (0 for unlimited).
 * @param direction The orientation direction of the layout, either Qt::Horizontal or Qt::Vertical.
 * @param parent The parent widget of this layout.
 */
OverlapLayout::OverlapLayout(int overlapPercentage,
                             int maxColumns,
                             int maxRows,
                             Qt::Orientation direction,
                             QWidget *parent)
    : QLayout(parent), overlapPercentage(overlapPercentage), maxColumns(maxColumns), maxRows(maxRows),
      direction(direction)
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
    itemList.append(item);
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
    return itemList.size();
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
QLayoutItem *OverlapLayout::itemAt(int index) const
{
    return itemList.value(index);
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
QLayoutItem *OverlapLayout::takeAt(int index)
{
    if (index >= 0 && index < itemList.size()) {
        return itemList.takeAt(index);
    }
    return nullptr;
}

/**
 * @brief Sets the geometry for the layout items, arranging them with the specified overlap.
 *
 * Positions each QLayoutItem within the specified rectangle `rect`, respecting
 * the layout’s direction, overlap percentage, and row/column limits. The method
 * calculates the maximum allowable size for each item based on the parent widget’s dimensions.
 * Items are then arranged with an offset based on the overlap percentage, either horizontally
 * or vertically, depending on the layout direction. The geometry of each item is then set
 * within the calculated bounds.
 *
 * @param rect The rectangle defining the area within which the layout should arrange items.
 */
void OverlapLayout::setGeometry(const QRect &rect)
{
    QLayout::setGeometry(rect);

    if (itemList.isEmpty())
        return;

    QWidget *parentWidget = this->parentWidget();
    if (parentWidget) {
        int availableWidth = parentWidget->width();
        int availableHeight = parentWidget->height();

        QMargins margins = parentWidget->contentsMargins();
        availableWidth -= margins.left() + margins.right();
        availableHeight -= margins.top() + margins.bottom();

        int maxItemWidth = 0;
        int maxItemHeight = 0;

        for (const QLayoutItem *item : itemList) {
            if (item->widget()) {
                QSize itemSize = item->widget()->sizeHint();
                maxItemWidth = std::max(maxItemWidth, itemSize.width());
                maxItemHeight = std::max(maxItemHeight, itemSize.height());
            }
        }

        int overlapOffsetWidth = (direction == Qt::Horizontal) ? (maxItemWidth * overlapPercentage / 100) : 0;
        int overlapOffsetHeight = (direction == Qt::Vertical) ? (maxItemHeight * overlapPercentage / 100) : 0;

        int columns = (direction == Qt::Horizontal)
                          ? (maxColumns > 0 ? std::min(maxColumns, (availableWidth + overlapOffsetWidth) /
                                                                       (maxItemWidth - overlapOffsetWidth))
                                            : INT_MAX)
                          : INT_MAX;
        int rows = (direction == Qt::Vertical)
                       ? (maxRows > 0 ? std::min(maxRows, (availableHeight + overlapOffsetHeight) /
                                                              (maxItemHeight - overlapOffsetHeight))
                                      : INT_MAX)
                       : INT_MAX;

        int currentRow = 0;
        int currentColumn = 0;

        for (int i = 0; i < itemList.size(); ++i) {
            QLayoutItem *item = itemList.at(i);
            int xPos = rect.left() + currentColumn * (maxItemWidth - overlapOffsetWidth);
            int yPos = rect.top() + currentRow * (maxItemHeight - overlapOffsetHeight);
            item->setGeometry(QRect(xPos, yPos, maxItemWidth, maxItemHeight));

            if (direction == Qt::Horizontal) {
                currentColumn++;
                if (currentColumn >= columns) {
                    currentColumn = 0;
                    currentRow++;
                }
            } else {
                currentRow++;
                if (currentRow >= rows) {
                    currentRow = 0;
                    currentColumn++;
                }
            }
        }
    }
}

/**
 * @brief Calculates the preferred size for the layout, considering overlap and orientation.
 *
 * Determines an optimal size for the layout by calculating the area needed to arrange all
 * items with the desired overlap. This calculation considers the number of rows/columns,
 * item size, overlap offset, and the layout's direction. Useful for setting a size hint
 * when the layout is used within other widgets.
 *
 * @return The preferred layout size as a QSize object.
 */
QSize OverlapLayout::calculatePreferredSize() const
{
    QSize preferredSize(0, 0);

    int maxItemWidth = 0;
    int maxItemHeight = 0;

    for (const QLayoutItem *item : itemList) {
        if (item->widget()) {
            QSize itemSize = item->widget()->sizeHint();
            maxItemWidth = std::max(maxItemWidth, itemSize.width());
            maxItemHeight = std::max(maxItemHeight, itemSize.height());
        }
    }

    int extra_for_overlap = (100 - overlapPercentage);
    int overlapOffsetWidth =
        (direction == Qt::Horizontal) ? static_cast<int>(std::round((maxItemWidth / 100.0) * extra_for_overlap)) : 0;
    int overlapOffsetHeight =
        (direction == Qt::Vertical) ? static_cast<int>(std::round((maxItemHeight / 100.0) * extra_for_overlap)) : 0;

    int totalWidth = 0;
    int totalHeight = 0;

    if (direction == Qt::Horizontal) {
        int numColumns = (maxColumns > 0) ? maxColumns : itemList.size();
        int extra_space_for_overlaps = overlapOffsetWidth * (numColumns - 1);
        totalWidth = maxItemWidth + extra_space_for_overlaps;

        int numRows = (maxColumns > 0) ? ceil(itemList.size() / static_cast<double>(maxRows)) : 1;
        totalHeight = maxItemHeight * numRows - (numRows - 1) * overlapOffsetHeight;
    } else if (direction == Qt::Vertical) {
        int numColumns = (maxRows != 0) ? ceil(itemList.size() / static_cast<double>(maxRows)) : 1;
        totalWidth = maxItemWidth * numColumns - (numColumns - 1) * overlapOffsetWidth;

        int numRows = (maxRows > 0) ? maxRows : itemList.size();
        int extra_space_for_overlaps = overlapOffsetHeight * (numRows - 1);
        totalHeight = maxItemHeight + extra_space_for_overlaps;
    }

    preferredSize = QSize(totalWidth, totalHeight);

    return preferredSize;
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
 * @param new_direction The new orientation direction (Qt::Horizontal or Qt::Vertical).
 */
void OverlapLayout::setDirection(Qt::Orientation new_direction)
{
    direction = new_direction;
}

/**
 * @brief Sets the maximum number of columns for horizontal orientation.
 *
 * @param newValue New maximum column count.
 */
void OverlapLayout::setMaxColumns(int newValue)
{
    maxColumns = newValue;
}

/**
 * @brief Sets the maximum number of rows for vertical orientation.
 *
 * @param newValue New maximum row count.
 */
void OverlapLayout::setMaxRows(int newValue)
{
    maxRows = newValue;
}
