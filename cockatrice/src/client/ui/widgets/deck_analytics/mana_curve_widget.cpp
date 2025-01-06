#include "mana_curve_widget.h"

#include "../general/display/bar_widget.h"

#include <QLabel>

ManaCurveWidget::ManaCurveWidget(QWidget *parent) : QWidget(parent)
{
    layout = new QHBoxLayout(this);
    this->setLayout(layout);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMinimumSize(0, 0);
}

void ManaCurveWidget::updateDisplay()
{
    // Clear the layout first
    if (layout != nullptr) {
        QLayoutItem *item;
        while ((item = layout->takeAt(0)) != nullptr) {
            delete item->widget(); // Delete the widget
            delete item;           // Delete the layout item
        }
    }

    // If layout is null, create a new layout, otherwise reuse the existing one
    if (layout == nullptr) {
        layout = new QHBoxLayout(this);
        this->setLayout(layout);
    }

    int totalSum = 0;
    for (auto entry : curve) {
        totalSum += entry.second;
    }

    // Add a bar for each CMC to the layout
    for (auto entry : curve) {
        BarWidget *barWidget = new BarWidget(QString::number(entry.first), entry.second, totalSum, this);
        layout->addWidget(barWidget);
    }

    update(); // Update the widget display
}
