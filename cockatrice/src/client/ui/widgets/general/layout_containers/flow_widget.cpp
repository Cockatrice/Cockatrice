/**
 * @file flow_widget.cpp
 * @brief Implementation of the FlowWidget class for organizing widgets in a flow layout within a scrollable area.
 */

#include "flow_widget.h"

#include "../../../layouts/flow_layout.h"
#include "../../../layouts/horizontal_flow_layout.h"
#include "../../../layouts/vertical_flow_layout.h"

#include <QHBoxLayout>
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
FlowWidget::FlowWidget(QWidget *parent, Qt::ScrollBarPolicy horizontalPolicy, Qt::ScrollBarPolicy verticalPolicy)
    : QWidget(parent)
{
    // Main Widget and Layout
    this->setMinimumSize(0, 0);
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    main_layout = new QHBoxLayout();
    this->setLayout(main_layout);

    // Flow Layout inside the scroll area
    container = new QWidget();

    if (horizontalPolicy != Qt::ScrollBarAlwaysOff && verticalPolicy == Qt::ScrollBarAlwaysOff) {
        flow_layout = new HorizontalFlowLayout(container);
    } else if (horizontalPolicy == Qt::ScrollBarAlwaysOff && verticalPolicy != Qt::ScrollBarAlwaysOff) {
        flow_layout = new VerticalFlowLayout(container);
    } else {
        flow_layout = new FlowLayout(container, 0, 0, 0);
    }

    container->setLayout(flow_layout);
    // The container should expand as much as possible, trusting the scrollArea to constrain it.
    container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    container->setMinimumSize(0, 0);

    // Scroll Area, which should expand as much as possible, since it should be the only direct child widget.
    scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setMinimumSize(0, 0);
    scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Set scrollbar policies
    scrollArea->setHorizontalScrollBarPolicy(horizontalPolicy);
    scrollArea->setVerticalScrollBarPolicy(verticalPolicy);

    // Use the FlowLayout container directly if we disable the ScrollArea
    if (horizontalPolicy == Qt::ScrollBarAlwaysOff && verticalPolicy == Qt::ScrollBarAlwaysOff) {
        main_layout->addWidget(container);
    } else {
        scrollArea->setWidget(container);
        main_layout->addWidget(scrollArea);
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
    // Adjust size policy if scrollbars are disabled
    if (scrollArea->horizontalScrollBarPolicy() == Qt::ScrollBarAlwaysOff) {
        widget_to_add->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    }
    if (scrollArea->verticalScrollBarPolicy() == Qt::ScrollBarAlwaysOff) {
        widget_to_add->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    }

    // Add the widget to the flow layout
    this->flow_layout->addWidget(widget_to_add);
}

/**
 * @brief Clears all widgets from the flow layout.
 *
 * Deletes each widget and layout item, and recreates the flow layout if it was removed.
 */
void FlowWidget::clearLayout()
{
    if (flow_layout != nullptr) {
        QLayoutItem *item;
        while ((item = flow_layout->takeAt(0)) != nullptr) {
            item->widget()->deleteLater(); // Delete the widget
            delete item;                   // Delete the layout item
        }
    } else {
        if (scrollArea->horizontalScrollBarPolicy() != Qt::ScrollBarAlwaysOff &&
            scrollArea->verticalScrollBarPolicy() == Qt::ScrollBarAlwaysOff) {
            flow_layout = new HorizontalFlowLayout(container);
        } else if (scrollArea->horizontalScrollBarPolicy() == Qt::ScrollBarAlwaysOff &&
                   scrollArea->verticalScrollBarPolicy() != Qt::ScrollBarAlwaysOff) {
            flow_layout = new VerticalFlowLayout(container);
        } else {
            flow_layout = new FlowLayout(container, 0, 0, 0);
        }
        this->container->setLayout(flow_layout);
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

    // Trigger the layout to recalculate
    if (flow_layout != nullptr) {
        flow_layout->invalidate(); // Marks the layout as dirty and requires recalculation
        flow_layout->activate();   // Recalculate the layout based on the new size
    }

    // Ensure the scroll area and its content adjust correctly
    if (scrollArea != nullptr) {
        if (scrollArea->widget() != nullptr) {
            scrollArea->widget()->adjustSize();
        }
    }
}
