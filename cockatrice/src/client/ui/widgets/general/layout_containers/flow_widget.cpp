/**
 * @file flow_widget.cpp
 * @brief Implementation of the FlowWidget class for organizing widgets in a flow layout within a scrollable area.
 */

#include "flow_widget.h"

#include <QHBoxLayout>
#include <QResizeEvent>
#include <QWidget>
#include <qscrollarea.h>
#include <qsizepolicy.h>

/**
 * @brief Constructs a FlowWidget with a scrollable layout.
 *
 * @param parent The parent widget of this FlowWidget.
 * @param horizontalPolicy The horizontal scroll bar policy for the scroll area.
 * @param verticalPolicy The vertical scroll bar policy for the scroll area.
 */
FlowWidget::FlowWidget(QWidget *parent,
                       const Qt::Orientation _flowDirection,
                       const Qt::ScrollBarPolicy horizontalPolicy,
                       const Qt::ScrollBarPolicy verticalPolicy)
    : QWidget(parent), flowDirection(_flowDirection)
{
    // Main Widget and Layout
    if (_flowDirection == Qt::Horizontal) {
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
        setMinimumWidth(0);
    } else {
        setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
        setMinimumHeight(0);
    }
    mainLayout = new QHBoxLayout(this);
    setLayout(mainLayout);

    if (horizontalPolicy != Qt::ScrollBarAlwaysOff || verticalPolicy != Qt::ScrollBarAlwaysOff) {
        // Scroll Area, which should expand as much as possible, since it should be the only direct child widget.
        scrollArea = new QScrollArea(this);
        scrollArea->setWidgetResizable(true);
        scrollArea->setMinimumSize(0, 0);
        scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        // Set scrollbar policies
        scrollArea->setHorizontalScrollBarPolicy(horizontalPolicy);
        scrollArea->setVerticalScrollBarPolicy(verticalPolicy);
    } else {
        scrollArea = nullptr;
    }

    // Flow Layout inside the scroll area
    if (horizontalPolicy == Qt::ScrollBarAlwaysOff && verticalPolicy == Qt::ScrollBarAlwaysOff) {
        container = new QWidget(this);
    } else {
        container = new QWidget(scrollArea);
    }

    flowLayout = new FlowLayout(container, flowDirection);

    container->setLayout(flowLayout);
    // The container should expand as much as possible, trusting the scrollArea to constrain it.
    if (_flowDirection == Qt::Horizontal) {
        container->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        container->setMinimumWidth(0);
    } else {
        container->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        container->setMinimumHeight(0);
    }

    // Use the FlowLayout container directly if we disable the ScrollArea
    if (horizontalPolicy == Qt::ScrollBarAlwaysOff && verticalPolicy == Qt::ScrollBarAlwaysOff) {
        mainLayout->addWidget(container);
    } else {
        scrollArea->setWidget(container);
        mainLayout->addWidget(scrollArea);
    }
}

/**
 * @brief Adds a widget to the flow layout within the FlowWidget.
 *
 * Adjusts the widget's size policy based on the scroll bar policies.
 *
 * @param widget_to_add The widget to add to the flow layout.
 */
void FlowWidget::addWidget(QWidget *widget_to_add) const
{
    flowLayout->addWidget(widget_to_add);
}

void FlowWidget::insertWidgetAtIndex(QWidget *toInsert, int index)
{
    flowLayout->insertWidgetAtIndex(toInsert, index);
    update();
}

void FlowWidget::removeWidget(QWidget *widgetToRemove) const
{
    flowLayout->removeWidget(widgetToRemove);
}

/**
 * @brief Clears all widgets from the flow layout.
 *
 * Deletes each widget and layout item, and recreates the flow layout if it was removed.
 */
void FlowWidget::clearLayout()
{
    if (flowLayout != nullptr) {
        QLayoutItem *item;
        while ((item = flowLayout->takeAt(0)) != nullptr) {
            item->widget()->deleteLater(); // Delete the widget
            delete item;                   // Delete the layout item
        }
    } else {
        flowLayout = new FlowLayout(container, flowDirection);
        container->setLayout(flowLayout);
    }
}

/**
 * @brief Handles resize events for the FlowWidget.
 *
 * Triggers layout recalculation and adjusts the scroll area content size.
 *
 * @param event The resize event containing the new size information.
 */
void FlowWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    qCDebug(FlowWidgetSizeLog) << event->size();

    // Trigger the layout to recalculate
    if (flowLayout != nullptr) {
        flowLayout->invalidate(); // Marks the layout as dirty and requires recalculation
        flowLayout->activate();   // Recalculate the layout based on the new size
    }

    // Ensure the scroll area and its content adjust correctly
    if (scrollArea != nullptr && scrollArea->widget() != nullptr) {
        qCDebug(FlowWidgetSizeLog) << "Got a scrollarea: " << scrollArea->widget()->size();
        scrollArea->widget()->adjustSize();
    } else {
        container->adjustSize();
    }
}

/**
 * @brief Sets the minimum size for all widgets inside the FlowWidget to the maximum sizeHint of all of them.
 */
void FlowWidget::setMinimumSizeToMaxSizeHint()
{
    QSize maxSize(0, 0); // Initialize to a zero size

    // Iterate over all widgets in the flow layout to find the maximum sizeHint
    for (int i = 0; i < flowLayout->count(); ++i) {
        if (QLayoutItem *item = flowLayout->itemAt(i)) {
            if (QWidget *widget = item->widget()) {
                // Update the max size based on the sizeHint of each widget
                QSize widgetSizeHint = widget->sizeHint();
                maxSize.setWidth(qMax(maxSize.width(), widgetSizeHint.width()));
                maxSize.setHeight(qMax(maxSize.height(), widgetSizeHint.height()));
            }
        }
    }

    // Set the minimum size for all widgets to the max sizeHint
    for (int i = 0; i < flowLayout->count(); ++i) {
        if (QLayoutItem *item = flowLayout->itemAt(i)) {
            if (QWidget *widget = item->widget()) {
                widget->setMinimumSize(maxSize);
            }
        }
    }
}

QLayoutItem *FlowWidget::itemAt(int index) const
{
    return flowLayout->itemAt(index);
}

int FlowWidget::count() const
{
    return flowLayout->count();
}