#include "horizontal_flow_layout.h"

/**
 * @brief Constructs a HorizontalFlowLayout instance with the specified parent widget.
 *        This layout arranges items in columns within the given height, automatically adjusting its width.
 * @param parent The parent widget to which this layout belongs.
 */
HorizontalFlowLayout::HorizontalFlowLayout(QWidget *parent) : FlowLayout(parent)
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
 * @brief Indicates that the layout has a variable width based on its height.
 * @return True, as the layout adjusts its width according to the available height.
 */
bool HorizontalFlowLayout::hasHeightForWidth() const
{
    return true;
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
    int rowWidth = 0;
    int rowHeight = 0;

    for (QLayoutItem *item : items) {
        int itemHeight = item->sizeHint().height();
        if (rowHeight + itemHeight > height) {
            width += rowWidth;
            rowHeight = itemHeight;
            rowWidth = item->sizeHint().width();
        } else {
            rowHeight += itemHeight;
            rowWidth = qMax(rowWidth, item->sizeHint().width());
        }
    }
    width += rowWidth; // Add width of the last column
    return width;
}

/**
 * @brief Sets the geometry of the layout items, arranging them in columns within the given height.
 * @param rect The rectangle area defining the layout space.
 */
void HorizontalFlowLayout::setGeometry(const QRect &rect)
{
    int availableHeight = qMax(rect.height(), getParentScrollAreaHeight());

    int totalWidth = layoutRows(rect.x(), rect.y(), availableHeight);

    QWidget *parentWidgetPtr = parentWidget();
    if (parentWidgetPtr) {
        parentWidgetPtr->setMinimumSize(availableHeight, totalWidth);
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
int HorizontalFlowLayout::layoutRows(int originX, int originY, int availableHeight)
{
    QVector<QLayoutItem *> rowItems;
    int currentXPosition = originX;
    int currentYPosition = originY;

    int rowWidth = 0;
    int rowHeight = 0;

    for (QLayoutItem *item : items) {
        QSize itemSize = item->sizeHint();
        if (currentYPosition + itemSize.height() > availableHeight) {
            layoutRow(rowItems, currentXPosition, originY);
            rowItems.clear();
            currentYPosition = originY;
            currentXPosition += rowWidth;
            rowWidth = 0;
            rowHeight = 0;
        }
        rowItems.append(item);
        rowHeight += itemSize.height();
        rowWidth = qMax(rowWidth, itemSize.width());
        currentYPosition += itemSize.height();
    }

    layoutRow(rowItems, currentXPosition, originY);

    return currentXPosition + rowWidth;
}

/**
 * @brief Arranges a single column of items within specified x and y starting positions.
 * @param rowItems A list of items to be arranged in the column.
 * @param x The starting x-coordinate for the column.
 * @param y The starting y-coordinate for the column.
 */
void HorizontalFlowLayout::layoutRow(const QVector<QLayoutItem *> &rowItems, int x, int y)
{
    for (QLayoutItem *item : rowItems) {
        QSize itemMaxSize = item->widget()->maximumSize();
        int itemWidth = qMin(item->sizeHint().width(), itemMaxSize.width());
        int itemHeight = qMin(item->sizeHint().height(), itemMaxSize.height());
        item->setGeometry(QRect(QPoint(x, y), QSize(itemWidth, itemHeight)));
        y += itemHeight;
    }
}