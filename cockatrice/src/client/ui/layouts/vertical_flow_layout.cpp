#include "vertical_flow_layout.h"

/**
 * @brief Constructs a VerticalFlowLayout instance with the specified parent widget.
 *        This layout arranges items in rows within the given width, automatically adjusting its height.
 * @param parent The parent widget to which this layout belongs.
 * @param margin The layout margin.
 * @param hSpacing The horizontal spacing between items.
 * @param vSpacing The vertical spacing between items.
 */
VerticalFlowLayout::VerticalFlowLayout(QWidget *parent, int margin, int hSpacing, int vSpacing)
    : FlowLayout(parent, margin, hSpacing, vSpacing)
{
}

/**
 * @brief Destructor for VerticalFlowLayout, responsible for cleaning up layout items.
 */
VerticalFlowLayout::~VerticalFlowLayout()
{
    QLayoutItem *item;
    while ((item = FlowLayout::takeAt(0))) {
        delete item;
    }
}

/**
 * @brief Calculates the required height to display all items, given a specified width.
 *        This method arranges items into rows and determines the total height needed.
 * @param width The available width for arranging layout items.
 * @return The total height required to fit all items, organized in rows constrained by the given width.
 */
int VerticalFlowLayout::heightForWidth(int width) const
{
    int height = 0;
    int rowWidth = 0;
    int rowHeight = 0;

    for (QLayoutItem *item : items) {
        if (!(item == nullptr || item->isEmpty())) {
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
    }
    height += rowHeight; // Add height of the last row
    return height;
}

/**
 * @brief Sets the geometry of the layout items, arranging them in rows within the given width.
 * @param rect The rectangle area defining the layout space.
 */
void VerticalFlowLayout::setGeometry(const QRect &rect)
{
    // If we have a parent scroll area, we're clamped to that, else we use our own rectangle.
    int availableWidth = getParentScrollAreaWidth() == 0 ? rect.width() : getParentScrollAreaWidth();

    int totalHeight = layoutAllRows(rect.x(), rect.y(), availableWidth);

    QWidget *parentWidgetPtr = parentWidget();
    if (parentWidgetPtr) {
        parentWidgetPtr->setMinimumSize(availableWidth, totalHeight);
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
int VerticalFlowLayout::layoutAllRows(int originX, int originY, int availableWidth)
{
    QVector<QLayoutItem *> rowItems; // Holds items for the current row
    int currentXPosition = originX;  // Tracks the x-coordinate while placing items
    int currentYPosition = originY;  // Tracks the y-coordinate, moving down after each row

    int rowHeight = 0; // Tracks the maximum height of items in the current row

    for (QLayoutItem *item : items) {
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
void VerticalFlowLayout::layoutSingleRow(const QVector<QLayoutItem *> &rowItems, int x, int y)
{
    for (QLayoutItem *item : rowItems) {
        if (!(item == nullptr || item->isEmpty())) {
            // Get the maximum allowed size for the item
            QSize itemMaxSize = item->widget()->maximumSize();
            // Constrain the item's width and height to its size hint or maximum size
            int itemWidth = qMin(item->sizeHint().width(), itemMaxSize.width());
            int itemHeight = qMin(item->sizeHint().height(), itemMaxSize.height());
            // Set the item's geometry based on the computed size and position
            item->setGeometry(QRect(QPoint(x, y), QSize(itemWidth, itemHeight)));
            // Move the x-position to the right, leaving space for horizontal spacing
            x += itemWidth + horizontalSpacing();
        }
    }
}
