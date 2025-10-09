#include "overlap_widget.h"

#include "../../../layouts/flow_layout.h"

#include <QWidget>
#include <libcockatrice/deck_list/deck_list_model.h>

/**
 * @class OverlapWidget
 * @brief A widget for managing overlapping child widgets.
 *
 * The OverlapWidget class is a QWidget subclass that utilizes the OverlapLayout
 * to arrange its child widgets in an overlapping manner. This widget allows
 * configuration of overlap percentage, maximum columns, maximum rows, and layout
 * direction, making it suitable for displaying elements that can partially stack
 * over each other. The widget automatically manages resizing and re-layout of its
 * child widgets based on the available space and specified parameters.
 */

/**
 * @brief Constructs an OverlapWidget with specified layout parameters.
 *
 * Initializes the OverlapWidget with the given overlap percentage, maximum number
 * of columns and rows, and layout direction. Sets size policies to ensure the widget
 * can expand as needed. A new OverlapLayout is created and assigned to manage the
 * layout of child widgets.
 *
 * @param overlapPercentage The percentage of overlap between child widgets (0-100).
 * @param maxColumns The maximum number of columns for the layout (0 for unlimited).
 * @param maxRows The maximum number of rows for the layout (0 for unlimited).
 * @param direction The orientation of the layout, either Qt::Horizontal or Qt::Vertical.
 * @param adjustOnResize If the overlap widgets should adjust its max columns/rows on resize to fit.
 * @param parent The parent widget of this OverlapWidget.
 */
OverlapWidget::OverlapWidget(QWidget *parent,
                             const int overlapPercentage,
                             const int maxColumns,
                             const int maxRows,
                             const Qt::Orientation direction,
                             const bool adjustOnResize)
    : QWidget(parent), overlapPercentage(overlapPercentage), maxColumns(maxColumns), maxRows(maxRows),
      direction(direction), adjustOnResize(adjustOnResize)
{
    overlapLayout = new OverlapLayout(this, overlapPercentage, maxColumns, maxRows, direction, Qt::Horizontal);
    setLayout(overlapLayout);
}

/**
 * @brief Adds a widget to the overlap layout.
 *
 * This method appends the specified widget to the internal OverlapLayout, allowing
 * it to be arranged with the existing child widgets. The widget's visibility and
 * behavior will be managed by the layout.
 *
 * @param widgetToAdd A pointer to the QWidget to be added to the layout.
 */
void OverlapWidget::addWidget(QWidget *widgetToAdd) const
{
    overlapLayout->addWidget(widgetToAdd);
}

void OverlapWidget::insertWidgetAtIndex(QWidget *toInsert, int index)
{
    overlapLayout->insertWidgetAtIndex(toInsert, index);
    update();
}

void OverlapWidget::removeWidget(QWidget *widgetToRemove) const
{
    overlapLayout->removeWidget(widgetToRemove);
}

/**
 * @brief Clears all widgets from the layout and deletes them.
 *
 * This method removes all child widgets from the OverlapLayout, deleting both the
 * widget instances and their corresponding layout items. This ensures that the layout
 * is empty and can be reused or refreshed as needed.
 */
void OverlapWidget::clearLayout()
{
    if (overlapLayout != nullptr) {
        QLayoutItem *item;
        while ((item = overlapLayout->takeAt(0)) != nullptr) {
            item->widget()->deleteLater();
            delete item;
        }
    }

    // If layout is null, create a new layout; otherwise, reuse the existing one
    if (overlapLayout == nullptr) {
        overlapLayout = new OverlapLayout(this, overlapPercentage, maxColumns, maxRows, direction);
        this->setLayout(overlapLayout);
    }
}

/**
 * @brief Handles resizing events for the widget.
 *
 * This overridden method is called when the widget is resized. It invokes layout
 * recalculation to ensure that the child widgets are correctly arranged based on the
 * new dimensions. It marks the layout as dirty and activates it to reflect the changes.
 *
 * @param event The resize event containing the new size information.
 */
void OverlapWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    // Trigger the layout to recalculate
    if (overlapLayout != nullptr) {
        overlapLayout->invalidate(); // Marks the layout as dirty and requires recalculation
        overlapLayout->activate();   // Recalculate the layout based on the new size
    }

    if (adjustOnResize) {
        adjustMaxColumnsAndRows();
    }
}

/**
 * @brief Dynamically adjusts maxColumns and maxRows based on widget size and layout direction.
 *
 * This function calculates the maximum number of columns or rows that can fit within
 * the widget's width and height, depending on the layout direction. It then updates
 * the OverlapLayout with these calculated values to ensure the layout is optimized.
 */
void OverlapWidget::adjustMaxColumnsAndRows()
{
    if (direction == Qt::Vertical) {
        // Calculate columns based on width for vertical layout
        const int calculatedColumns = overlapLayout->calculateMaxColumns();
        maxColumns = calculatedColumns;
        overlapLayout->setMaxColumns(calculatedColumns);

        // Calculate rows based on total item count and columns
        const int calculatedRows = overlapLayout->calculateRowsForColumns(calculatedColumns);
        maxRows = calculatedRows;
        overlapLayout->setMaxRows(calculatedRows);
    } else {
        // Calculate rows based on height for horizontal layout
        const int calculatedRows = overlapLayout->calculateMaxRows();
        maxRows = calculatedRows;
        overlapLayout->setMaxRows(calculatedRows);

        // Calculate columns based on total item count and rows
        const int calculatedColumns = overlapLayout->calculateColumnsForRows(calculatedRows);
        maxColumns = calculatedColumns;
        overlapLayout->setMaxColumns(calculatedColumns);
    }

    overlapLayout->invalidate();
    overlapLayout->activate();
}

/**
 * @brief Updates the maximum number of overlapping items based on new value.
 *
 * This method updates the maximum number of columns or rows for the overlap layout
 * based on the given new value. It adjusts the layout direction accordingly and
 * triggers a size adjustment for the widget, ensuring the layout reflects the changes.
 *
 * @param newValue The new maximum number of overlapping items allowed in the layout.
 */
void OverlapWidget::maxOverlapItemsChanged(const int newValue)
{
    if (direction == Qt::Horizontal) {
        maxRows = 0;
        overlapLayout->setMaxRows(0);

        maxColumns = newValue;
        overlapLayout->setMaxColumns(newValue);
    } else {
        maxRows = newValue;
        overlapLayout->setMaxRows(newValue);

        maxColumns = 0;
        overlapLayout->setMaxColumns(0);
    }
    this->adjustSize();
    overlapLayout->invalidate();
}

/**
 * @brief Changes the layout direction based on the specified new direction.
 *
 * This method modifies the layout direction of the OverlapLayout based on the input
 * string. It updates the direction and triggers a size adjustment for the widget.
 * Valid inputs are "Qt::Horizontal" and "Qt::Vertical".
 *
 * @param newDirection The new layout direction as a QString.
 */
void OverlapWidget::overlapDirectionChanged(const QString &newDirection)
{
    if (newDirection.compare("Qt::Horizontal", Qt::CaseInsensitive) == 0) {
        direction = Qt::Horizontal;
        overlapLayout->setDirection(direction);
    } else if (newDirection.compare("Qt::Vertical", Qt::CaseInsensitive) == 0) {
        direction = Qt::Vertical;
        overlapLayout->setDirection(direction);
    }
    this->adjustSize();
    overlapLayout->invalidate();
}
