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
 * @brief Indicates that the layout has a variable height based on its width.
 * @return True, as the layout adjusts its height according to the available width.
 */
bool VerticalFlowLayout::hasHeightForWidth() const
{
    return true;
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
}

/**
 * @brief Sets the geometry of the layout items, arranging them in rows within the given width.
 * @param rect The rectangle area defining the layout space.
 */
void VerticalFlowLayout::setGeometry(const QRect &rect)
{
    // If we have a parent scroll area, we're clamped to that, else we use our own rectangle.
    int availableWidth = getParentScrollAreaWidth() == 0 ? rect.width() : getParentScrollAreaWidth();

    int totalHeight = layoutRows(rect.x(), rect.y(), availableWidth);

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
int VerticalFlowLayout::layoutRows(int originX, int originY, int availableWidth)
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
        rowWidth += itemWidth;
        rowHeight = qMax(rowHeight, itemSize.height());
        currentXPosition += itemWidth;
    }

    layoutRow(rowItems, originX, currentYPosition);

    return currentYPosition + rowHeight;
}

/**
 * @brief Arranges a single row of items within specified x and y starting positions.
 * @param rowItems A list of items to be arranged in the row.
 * @param x The starting x-coordinate for the row.
 * @param y The starting y-coordinate for the row.
 */
void VerticalFlowLayout::layoutRow(const QVector<QLayoutItem *> &rowItems, int x, int y)
{
    for (QLayoutItem *item : rowItems) {
        QSize itemMaxSize = item->widget()->maximumSize();
        int itemWidth = qMin(item->sizeHint().width(), itemMaxSize.width());
        int itemHeight = qMin(item->sizeHint().height(), itemMaxSize.height());
        item->setGeometry(QRect(QPoint(x, y), QSize(itemWidth, itemHeight)));
        x += itemWidth + horizontalSpacing(); // Include horizontal spacing between items
    }
}
