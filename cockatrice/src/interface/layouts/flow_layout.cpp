/**
 * @file flow_layout.cpp
 * @brief Implementation of FlowLayout — a QLayout that wraps child widgets into rows
 *        (Qt::Horizontal flow) or columns (Qt::Vertical flow).
 *
 * Design contract (following Qt layout conventions):
 *   - setGeometry()  places children inside the given rect. Nothing else.
 *   - sizeHint()     reports the unconstrained preferred size (all items in one line).
 *   - minimumSize()  reports the minimum size (largest single item).
 *   - heightForWidth() reports the height needed for a given width (horizontal flow only).
 *
 * The layout never calls setFixedSize() or adjustSize() on its parent widget;
 * that is the responsibility of the parent widget / scroll area.
 */

#include "flow_layout.h"

#include "../widgets/general/layout_containers/flow_widget.h"

#include <QDebug>
#include <QLayoutItem>
#include <QScrollArea>
#include <QStyle>
#include <QWidgetItem>

FlowLayout::FlowLayout(QWidget *parent,
                       const Qt::Orientation flowDirection,
                       const int margin,
                       const int hSpacing,
                       const int vSpacing)
    : QLayout(parent), flowDirection(flowDirection), horizontalMargin(hSpacing), verticalMargin(vSpacing)
{
    setContentsMargins(margin, margin, margin, margin);
}

FlowLayout::~FlowLayout()
{
    QLayoutItem *item;
    while ((item = FlowLayout::takeAt(0))) {
        delete item;
    }
}

/**
 * @brief Reports which axis the layout can expand along.
 *
 * A horizontally-flowing layout expands horizontally (and wraps vertically,
 * but that is governed by heightForWidth, not by this flag).
 * A vertically-flowing layout expands vertically.
 */
Qt::Orientations FlowLayout::expandingDirections() const
{
    return (flowDirection == Qt::Horizontal) ? Qt::Horizontal : Qt::Vertical;
}

/**
 * @brief Height-for-width is only meaningful for horizontal (wrapping) flow.
 */
bool FlowLayout::hasHeightForWidth() const
{
    return flowDirection == Qt::Horizontal;
}

/**
 * @brief Returns the height required to display all items within @p width.
 *
 * Only valid for horizontal flow; returns -1 otherwise so Qt ignores it.
 * Spacing is counted once between adjacent items, never before the first
 * or after the last.
 */
int FlowLayout::heightForWidth(const int width) const
{
    if (flowDirection != Qt::Horizontal)
        return -1;

    int totalHeight = 0;
    int rowUsedWidth = 0;
    int rowHeight = 0;

    for (const QLayoutItem *item : items) {
        if (!item || item->isEmpty()) {
            continue;
        }

        const QSize itemSize = item->sizeHint();
        // Spacing is only inserted between items, not before the first.
        const int spaceX = (rowUsedWidth > 0) ? horizontalSpacing() : 0;

        if (rowUsedWidth > 0 && rowUsedWidth + spaceX + itemSize.width() > width) {
            // This item overflows the current row — commit the row and start a new one.
            totalHeight += rowHeight + verticalSpacing();
            rowUsedWidth = itemSize.width();
            rowHeight = itemSize.height();
        } else {
            rowUsedWidth += spaceX + itemSize.width();
            rowHeight = qMax(rowHeight, itemSize.height());
        }
    }

    return totalHeight + rowHeight; // Include the final (possibly only) row.
}

/**
 * @brief Places all children within @p rect.
 *
 * This is the only method that may move/resize children.  It does NOT resize
 * the parent widget; that would break Qt's layout protocol.
 */
void FlowLayout::setGeometry(const QRect &rect)
{
    QLayout::setGeometry(rect);

    if (flowDirection == Qt::Horizontal) {
        layoutAllRows(rect.x(), rect.y(), rect.width());
    } else {
        layoutAllColumns(rect.x(), rect.y(), rect.height());
    }
}

QSize FlowLayout::sizeHint() const
{
    return (flowDirection == Qt::Horizontal) ? calculateSizeHintHorizontal() : calculateSizeHintVertical();
}

QSize FlowLayout::minimumSize() const
{
    return (flowDirection == Qt::Horizontal) ? calculateMinimumSizeHorizontal() : calculateMinimumSizeVertical();
}

// ─── Row layout (horizontal flow) ────────────────────────────────────────────

/**
 * @brief Places all items into wrapping rows within @p availableWidth.
 * @return The y-coordinate of the bottom edge of the last row.
 */
int FlowLayout::layoutAllRows(const int originX, const int originY, const int availableWidth)
{
    QVector<QLayoutItem *> rowItems;
    int rowUsedWidth = 0; // Width consumed by items already in the current row.
    int currentY = originY;
    int rowHeight = 0;

    for (QLayoutItem *item : items) {
        if (!item || item->isEmpty()) {
            continue;
        }

        const QSize itemSize = item->sizeHint();
        // No leading space for the first item in a row.
        const int spaceX = rowItems.isEmpty() ? 0 : horizontalSpacing();

        if (!rowItems.isEmpty() && rowUsedWidth + spaceX + itemSize.width() > availableWidth) {
            // Current item does not fit — flush the current row, begin a new one.
            layoutSingleRow(rowItems, originX, currentY);
            rowItems.clear();
            currentY += rowHeight + verticalSpacing();
            rowUsedWidth = 0;
            rowHeight = 0;
        }

        // Add the item to the current row
        rowItems.append(item);
        // `rowItems.size() > 1` is equivalent to "this is not the first item in the row"
        // because we just appended above.
        rowUsedWidth += (rowItems.size() > 1 ? horizontalSpacing() : 0) + itemSize.width();
        rowHeight = qMax(rowHeight, itemSize.height());
    }

    layoutSingleRow(rowItems, originX, currentY); // Flush the final row.
    return currentY + rowHeight;
}

/**
 * @brief Sets the geometry for every item in @p rowItems, starting at (@p x, @p y).
 *
 * Each item is placed at its sizeHint, clamped to its maximumSize.
 */
void FlowLayout::layoutSingleRow(const QVector<QLayoutItem *> &rowItems, int x, const int y)
{
    for (QLayoutItem *item : rowItems) {
        if (!item || item->isEmpty()) {
            continue;
        }

        QWidget *widget = item->widget();
        if (!widget) {
            continue;
        }

        const QSize maxSize = widget->maximumSize();
        const int itemWidth = qMin(item->sizeHint().width(), maxSize.width());
        const int itemHeight = qMin(item->sizeHint().height(), maxSize.height());

        item->setGeometry(QRect(QPoint(x, y), QSize(itemWidth, itemHeight)));
        // Move the x-position to the right, leaving space for horizontal spacing
        x += itemWidth + horizontalSpacing();
    }
}

// ─── Column layout (vertical flow) ───────────────────────────────────────────

/**
 * @brief Places all items into wrapping columns within @p availableHeight.
 * @return The x-coordinate of the right edge of the last column.
 */
int FlowLayout::layoutAllColumns(const int originX, const int originY, const int availableHeight)
{
    QVector<QLayoutItem *> colItems;
    int colUsedHeight = 0; // Height consumed by items already in the current column.
    int currentX = originX;
    int colWidth = 0;

    for (QLayoutItem *item : items) {
        if (!item || item->isEmpty()) {
            continue;
        }

        const QSize itemSize = item->sizeHint();
        // No leading space for the first item in a column.
        const int spaceY = colItems.isEmpty() ? 0 : verticalSpacing();

        if (!colItems.isEmpty() && colUsedHeight + spaceY + itemSize.height() > availableHeight) {
            // Current item does not fit — flush the current column, begin a new one.
            layoutSingleColumn(colItems, currentX, originY);
            colItems.clear();
            currentX += colWidth + horizontalSpacing();
            colUsedHeight = 0;
            colWidth = 0;
        }

        colItems.append(item);
        colUsedHeight += (colItems.size() > 1 ? verticalSpacing() : 0) + itemSize.height();
        colWidth = qMax(colWidth, itemSize.width());
    }

    layoutSingleColumn(colItems, currentX, originY); // Flush the final column.
    return currentX + colWidth;
}

/**
 * @brief Sets the geometry for every item in @p colItems, starting at (@p x, @p y).
 *
 * Each item is placed at its sizeHint, clamped to its maximumSize.
 */
void FlowLayout::layoutSingleColumn(const QVector<QLayoutItem *> &colItems, const int x, int y)
{
    for (QLayoutItem *item : colItems) {
        if (!item || item->isEmpty()) {
            qCDebug(FlowLayoutLog) << "Skipping null or empty item in column.";
            continue;
        }

        QWidget *widget = item->widget();
        if (!widget) {
            qCDebug(FlowLayoutLog) << "Item has no widget; skipping.";
            continue;
        }

        qCDebug(FlowLayoutLog) << "Widget:" << widget->metaObject()->className() << "sizeHint:" << widget->sizeHint()
                               << "maximumSize:" << widget->maximumSize() << "minimumSize:" << widget->minimumSize();

        const QSize maxSize = widget->maximumSize();
        const int itemWidth = qMin(item->sizeHint().width(), maxSize.width());
        const int itemHeight = qMin(item->sizeHint().height(), maxSize.height());

        qCDebug(FlowLayoutLog) << "Placing at x=" << x << "y=" << y << "w=" << itemWidth << "h=" << itemHeight;

        // Set the item's geometry based on the computed size and position
        item->setGeometry(QRect(QPoint(x, y), QSize(itemWidth, itemHeight)));
        y += itemHeight + verticalSpacing();
    }
}

/**
 * @brief Preferred size for horizontal flow: all items in a single row (unconstrained).
 *
 * The actual displayed height is determined by heightForWidth() once Qt knows the
 * real available width.
 */
QSize FlowLayout::calculateSizeHintHorizontal() const
{
    int totalWidth = 0;
    int maxHeight = 0;

    for (const QLayoutItem *item : items) {
        if (!item || item->isEmpty()) {
            continue;
        }
        const QSize s = item->sizeHint();
        if (totalWidth > 0) {
            totalWidth += horizontalSpacing();
        }
        totalWidth += s.width();
        maxHeight = qMax(maxHeight, s.height());
    }
    return QSize(totalWidth, maxHeight);
}

/**
 * @brief Minimum size for horizontal flow: the largest single item.
 *
 * This guarantees we can always display at least one item per row.
 */
QSize FlowLayout::calculateMinimumSizeHorizontal() const
{
    QSize size(0, 0);
    for (const QLayoutItem *item : items) {
        if (!item || item->isEmpty()) {
            qCDebug(FlowLayoutLog) << "Skipping empty item.";
            continue;
        }
        size = size.expandedTo(item->minimumSize());
    }
    return size;
}

/**
 * @brief Preferred size for vertical flow: all items in a single column (unconstrained).
 */
QSize FlowLayout::calculateSizeHintVertical() const
{
    int maxWidth = 0;
    int totalHeight = 0;

    for (const QLayoutItem *item : items) {
        if (!item || item->isEmpty()) {
            qCDebug(FlowLayoutLog) << "Skipping empty item.";
            continue;
        }
        const QSize s = item->sizeHint();
        if (totalHeight > 0) {
            totalHeight += verticalSpacing();
        }
        totalHeight += s.height();
        maxWidth = qMax(maxWidth, s.width());
    }
    return QSize(maxWidth, totalHeight);
}

/**
 * @brief Minimum size for vertical flow: the largest single item.
 */
QSize FlowLayout::calculateMinimumSizeVertical() const
{
    QSize size(0, 0);
    for (const QLayoutItem *item : items) {
        if (!item || item->isEmpty()) {
            qCDebug(FlowLayoutLog) << "Skipping empty item.";
            continue;
        }
        size = size.expandedTo(item->minimumSize());
    }
    return size;
}

/**
 * @brief Adds a new item to the layout.
 * @param item The layout item to add.
 */
void FlowLayout::addItem(QLayoutItem *item)
{
    if (item) {
        items.append(item);
    }
}

void FlowLayout::insertWidgetAtIndex(QWidget *toInsert, int index)
{
    addChildWidget(toInsert);
    const int bounded = qBound(0, index, static_cast<int>(items.size()));
    items.insert(bounded, new QWidgetItem(toInsert));
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
    QObject *p = parent();
    if (!p) {
        return -1;
    }
    if (p->isWidgetType()) {
        const auto *pw = static_cast<QWidget *>(p);
        return pw->style()->pixelMetric(pm, nullptr, pw);
    }
    return static_cast<QLayout *>(p)->spacing();
}