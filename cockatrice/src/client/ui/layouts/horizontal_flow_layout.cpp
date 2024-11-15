#include "horizontal_flow_layout.h"

/**
 * @brief Constructs a HorizontalFlowLayout instance with the specified parent widget.
 *        This layout arranges items in columns within the given height, automatically adjusting its width.
 * @param parent The parent widget to which this layout belongs.
 * @param margin The layout margin.
 * @param hSpacing The horizontal spacing between items.
 * @param vSpacing The vertical spacing between items.
 */
HorizontalFlowLayout::HorizontalFlowLayout(QWidget *parent, int margin, int hSpacing, int vSpacing)
    : FlowLayout(parent, margin, hSpacing, vSpacing)
{
}

/**
 * @brief Destructor for HorizontalFlowLayout, responsible for cleaning up layout items.
 */
HorizontalFlowLayout::~HorizontalFlowLayout()
{
    QLayoutItem *item;
    while ((item = FlowLayout::takeAt(0))) {
        delete item;
    }
}

/**
 * @brief Calculates the required width to display all items, given a specified height.
 *        This method arranges items into columns and determines the total width needed.
 * @param height The available height for arranging layout items.
 * @return The total width required to fit all items, organized in columns constrained by the given height.
 */
int HorizontalFlowLayout::heightForWidth(int height) const
{
    int width = 0;
    int colWidth = 0;
    int colHeight = 0;

    for (QLayoutItem *item : items) {
        if (!(item == nullptr || item->isEmpty())) {
            int itemHeight = item->sizeHint().height();
            if (colHeight + itemHeight > height) {
                width += colWidth;
                colHeight = itemHeight;
                colWidth = item->sizeHint().width();
            } else {
                colHeight += itemHeight;
                colWidth = qMax(colWidth, item->sizeHint().width());
            }
        }
    }
    width += colWidth; // Add width of the last column
    return width;
}

/**
 * @brief Sets the geometry of the layout items, arranging them in columns within the given height.
 * @param rect The rectangle area defining the layout space.
 */
void HorizontalFlowLayout::setGeometry(const QRect &rect)
{
    int availableHeight = qMax(rect.height(), getParentScrollAreaHeight());

    int totalWidth = layoutAllColumns(rect.x(), rect.y(), availableHeight);

    QWidget *parentWidgetPtr = parentWidget();
    if (parentWidgetPtr) {
        parentWidgetPtr->setMinimumSize(totalWidth, availableHeight);
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
int HorizontalFlowLayout::layoutAllColumns(int originX, int originY, int availableHeight)
{
    QVector<QLayoutItem *> colItems; // Holds items for the current column
    int currentXPosition = originX;  // Tracks the x-coordinate while placing items
    int currentYPosition = originY;  // Tracks the y-coordinate, resetting for each new column

    int colWidth = 0; // Tracks the maximum width of items in the current column

    for (QLayoutItem *item : items) {
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
void HorizontalFlowLayout::layoutSingleColumn(const QVector<QLayoutItem *> &colItems, int x, int y)
{
    for (QLayoutItem *item : colItems) {
        if (!(item == nullptr || item->isEmpty())) {
            // Get the maximum allowed size for the item
            QSize itemMaxSize = item->widget()->maximumSize();
            // Constrain the item's width and height to its size hint or maximum size
            int itemWidth = qMin(item->sizeHint().width(), itemMaxSize.width());
            int itemHeight = qMin(item->sizeHint().height(), itemMaxSize.height());
            // Set the item's geometry based on the computed size and position
            item->setGeometry(QRect(QPoint(x, y), QSize(itemWidth, itemHeight)));
            // Move the y-position down by the item's height to place the next item below
            y += itemHeight;
        }
    }
}
