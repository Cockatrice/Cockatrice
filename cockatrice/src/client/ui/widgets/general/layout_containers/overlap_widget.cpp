#include "overlap_widget.h"

#include "../../../../../deck/deck_list_model.h"
#include "../../../layouts/flow_layout.h"

#include <QWidget>
#include <qsizepolicy.h>

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
 * @param parent The parent widget of this OverlapWidget.
 */
OverlapWidget::OverlapWidget(QWidget *parent,
                             int overlapPercentage,
                             int maxColumns,
                             int maxRows,
                             Qt::Orientation direction,
                             bool adjustOnResize)
    : QWidget(parent), overlapPercentage(overlapPercentage), maxColumns(maxColumns), maxRows(maxRows),
      direction(direction), adjustOnResize(adjustOnResize)
{
    this->setMinimumSize(0, 0);
    overlap_layout = new OverlapLayout(this, overlapPercentage, maxColumns, maxRows, direction);
    this->setLayout(overlap_layout);
}

/**
 * @brief Adds a widget to the overlap layout.
 *
 * This method appends the specified widget to the internal OverlapLayout, allowing
 * it to be arranged with the existing child widgets. The widget's visibility and
 * behavior will be managed by the layout.
 *
 * @param widget_to_add A pointer to the QWidget to be added to the layout.
 */
void OverlapWidget::addWidget(QWidget *widget_to_add)
{
    this->overlap_layout->addWidget(widget_to_add);
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
    if (overlap_layout != nullptr) {
        QLayoutItem *item;
        while ((item = overlap_layout->takeAt(0)) != nullptr) {
            delete item->widget(); // Delete the widget
            delete item;           // Delete the layout item
        }
    }

    // If layout is null, create a new layout; otherwise, reuse the existing one
    if (overlap_layout == nullptr) {
        overlap_layout = new OverlapLayout(this, overlapPercentage, maxColumns, maxRows, direction);
        this->setLayout(overlap_layout);
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
void OverlapWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);

    // Trigger the layout to recalculate
    if (overlap_layout != nullptr) {
        overlap_layout->invalidate(); // Marks the layout as dirty and requires recalculation
        overlap_layout->activate();   // Recalculate the layout based on the new size
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
        int calculatedColumns = overlap_layout->calculateMaxColumns();
        maxColumns = calculatedColumns;
        overlap_layout->setMaxColumns(calculatedColumns);

        // Calculate rows based on total item count and columns
        int calculatedRows = overlap_layout->calculateRowsForColumns(calculatedColumns);
        maxRows = calculatedRows;
        overlap_layout->setMaxRows(calculatedRows);
        qDebug() << "MaxColumns: " << maxColumns << " MaxRows: " << maxRows;
    } else {
        // Calculate rows based on height for horizontal layout
        int calculatedRows = overlap_layout->calculateMaxRows();
        maxRows = calculatedRows;
        overlap_layout->setMaxRows(calculatedRows);

        // Calculate columns based on total item count and rows
        int calculatedColumns = overlap_layout->calculateColumnsForRows(calculatedRows);
        maxColumns = calculatedColumns;
        overlap_layout->setMaxColumns(calculatedColumns);
        qDebug() << " MaxRows: " << maxRows << "MaxColumns: " << maxColumns;
    }

    overlap_layout->invalidate();
    overlap_layout->activate();
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
void OverlapWidget::maxOverlapItemsChanged(int newValue)
{
    if (direction == Qt::Horizontal) {
        maxRows = 0;
        overlap_layout->setMaxRows(0);

        maxColumns = newValue;
        overlap_layout->setMaxColumns(newValue);
    } else {
        maxRows = newValue;
        overlap_layout->setMaxRows(newValue);

        maxColumns = 0;
        overlap_layout->setMaxColumns(0);
    }
    this->adjustSize();
    overlap_layout->invalidate();
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
void OverlapWidget::overlapDirectionChanged(QString newDirection)
{
    if (newDirection.compare("Qt::Horizontal", Qt::CaseInsensitive) == 0) {
        direction = Qt::Horizontal;
        overlap_layout->setDirection(direction);
    } else if (newDirection.compare("Qt::Vertical", Qt::CaseInsensitive) == 0) {
        direction = Qt::Vertical;
        overlap_layout->setDirection(direction);
    }
    this->adjustSize();
    overlap_layout->invalidate();
}
