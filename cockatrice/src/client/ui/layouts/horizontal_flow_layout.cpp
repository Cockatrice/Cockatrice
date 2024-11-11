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
    int colWidth = 0;
    int colHeight = 0;

    for (QLayoutItem *item : items) {
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

    int totalWidth = layoutColumns(rect.x(), rect.y(), availableHeight);

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
int HorizontalFlowLayout::layoutColumns(int originX, int originY, int availableHeight)
{
    QVector<QLayoutItem *> colItems;
    int currentXPosition = originX;
    int currentYPosition = originY;

    int colWidth = 0;
    int colHeight = 0;

    for (QLayoutItem *item : items) {
        QSize itemSize = item->sizeHint();
        if (currentYPosition + itemSize.height() > availableHeight) {
            layoutColumn(colItems, currentXPosition, originY);
            colItems.clear();
            currentYPosition = originY;
            currentXPosition += colWidth;
            colWidth = 0;
            colHeight = 0;
        }
        colItems.append(item);
        colHeight += itemSize.height();
        colWidth = qMax(colWidth, itemSize.width());
        currentYPosition += itemSize.height();
    }

    layoutColumn(colItems, currentXPosition, originY);

    return currentXPosition + colWidth;
}

/**
 * @brief Arranges a single column of items within specified x and y starting positions.
 * @param colItems A list of items to be arranged in the column.
 * @param x The starting x-coordinate for the column.
 * @param y The starting y-coordinate for the column.
 */
void HorizontalFlowLayout::layoutColumn(const QVector<QLayoutItem *> &colItems, int x, int y)
{
    for (QLayoutItem *item : colItems) {
        QSize itemMaxSize = item->widget()->maximumSize();
        int itemWidth = qMin(item->sizeHint().width(), itemMaxSize.width());
        int itemHeight = qMin(item->sizeHint().height(), itemMaxSize.height());
        item->setGeometry(QRect(QPoint(x, y), QSize(itemWidth, itemHeight)));
        y += itemHeight;
    }
}
