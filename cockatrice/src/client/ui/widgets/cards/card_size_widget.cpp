#include "card_size_widget.h"

CardSizeWidget::CardSizeWidget(QWidget *parent, FlowWidget *flowWidget) : parent(parent), flowWidget(flowWidget)
{
    cardSizeLayout = new QHBoxLayout(this);
    setLayout(cardSizeLayout);

    cardSizeLabel = new QLabel(tr("Card Size"), this);
    cardSizeLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    cardSizeSlider = new QSlider(Qt::Horizontal, this);
    cardSizeSlider->setRange(25, 250);
    cardSizeSlider->setValue(100);

    cardSizeLayout->addWidget(cardSizeLabel);
    cardSizeLayout->addWidget(cardSizeSlider);

    if (flowWidget != nullptr) {
        connect(cardSizeSlider, &QSlider::valueChanged, flowWidget, &FlowWidget::setMinimumSizeToMaxSizeHint);
    }
}

QSlider *CardSizeWidget::getSlider() const
{
    return cardSizeSlider;
}
