/**
 * @file flow_widget.cpp
 * @brief Implementation of FlowWidget — a QWidget hosting a FlowLayout inside an
 *        optional QScrollArea.
 */

#include "flow_widget.h"

#include <QHBoxLayout>
#include <QResizeEvent>
#include <QScrollArea>
#include <QSizePolicy>
#include <QWidget>

/**
 * @brief Constructs a FlowWidget.
 *
 * When both scroll policies are Qt::ScrollBarAlwaysOff the scroll area is
 * omitted entirely and the container is placed directly in the main layout.
 *
 * @param parent            Parent widget.
 * @param _flowDirection    Qt::Horizontal for row-wrapping, Qt::Vertical for column-wrapping.
 * @param horizontalPolicy  Horizontal scroll-bar policy.
 * @param verticalPolicy    Vertical scroll-bar policy.
 */
FlowWidget::FlowWidget(QWidget *parent,
                       const Qt::Orientation _flowDirection,
                       const Qt::ScrollBarPolicy horizontalPolicy,
                       const Qt::ScrollBarPolicy verticalPolicy)
    : QWidget(parent), scrollArea(nullptr), flowDirection(_flowDirection)

{
    // Top-level size policy
    // Horizontal flow: expand horizontally, let height be determined by wrapping.
    // Vertical flow:   expand vertically, let width be determined by wrapping.
    if (flowDirection == Qt::Horizontal) {
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    } else {
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    }

    mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(mainLayout);

    const bool useScrollArea = (horizontalPolicy != Qt::ScrollBarAlwaysOff || verticalPolicy != Qt::ScrollBarAlwaysOff);

    // Scroll area (optional)
    if (useScrollArea) {
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

    // Container widget (holds the FlowLayout)
    container = new QWidget(useScrollArea ? static_cast<QWidget *>(scrollArea) : this);

    // The container should be willing to grow in both axes; its actual size is
    // governed by the FlowLayout's sizeHint / heightForWidth, not by a fixed policy.
    container->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    container->setMinimumSize(0, 0);

    flowLayout = new FlowLayout(container, flowDirection);
    container->setLayout(flowLayout);

    if (useScrollArea) {
        scrollArea->setWidget(container);
        mainLayout->addWidget(scrollArea);
    } else {
        mainLayout->addWidget(container);
    }
}

/**
 * @brief Adds a widget to the flow layout within the FlowWidget.
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
 * @brief Removes all widgets from the flow layout and deletes them.
 *
 * If the layout pointer has somehow been lost it is recreated before returning.
 */
void FlowWidget::clearLayout()
{
    if (flowLayout) {
        QLayoutItem *item;
        while ((item = flowLayout->takeAt(0))) {
            if (item->widget())
                item->widget()->deleteLater();
            delete item;
        }
    } else {
        // Defensive fallback: recreate the layout if it was deleted externally.
        flowLayout = new FlowLayout(container, flowDirection);
        container->setLayout(flowLayout);
    }
}

/**
 * @brief Marks the flow layout as dirty so Qt recomputes item positions.
 *
 * We do NOT call adjustSize() or activate() here:
 *  - adjustSize() would freeze geometry by calling setFixedSize internally.
 *  - activate() called inside a resize event can cause synchronous re-entrancy.
 * Qt automatically calls setGeometry on the layout after a resize, so simply
 * invalidating is sufficient.
 */
void FlowWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    qCDebug(FlowWidgetSizeLog) << "resizeEvent:" << event->size();

    if (flowLayout) {
        flowLayout->invalidate();
    }
}

void FlowWidget::setSpacing(int hSpacing, int vSpacing)
{
    flowLayout->setHorizontalMargin(hSpacing);
    flowLayout->setVerticalMargin(vSpacing);
    flowLayout->invalidate();
}

/**
 * @brief Sets every child widget's minimum size to the largest sizeHint in the layout.
 *
 * Useful for toolbars or button bars where all items should be the same size.
 */
void FlowWidget::setMinimumSizeToMaxSizeHint()
{
    QSize maxSize(0, 0);

    // Iterate over all widgets in the flow layout to find the maximum sizeHint
    for (int i = 0; i < flowLayout->count(); ++i) {
        QLayoutItem *item = flowLayout->itemAt(i);
        if (item && item->widget()) {
            maxSize = maxSize.expandedTo(item->widget()->sizeHint());
        }
    }

    // Set the minimum size for all widgets to the max sizeHint
    for (int i = 0; i < flowLayout->count(); ++i) {
        QLayoutItem *item = flowLayout->itemAt(i);
        if (item && item->widget()) {
            item->widget()->setMinimumSize(maxSize);
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