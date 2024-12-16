#include "card_size_widget.h"

#include "../../../../settings/cache_settings.h"

CardSizeWidget::CardSizeWidget(QWidget *parent, FlowWidget *flowWidget, int defaultValue) : parent(parent), flowWidget(flowWidget)
{
    cardSizeLayout = new QHBoxLayout(this);
    setLayout(cardSizeLayout);

    cardSizeLabel = new QLabel(tr("Card Size"), this);
    cardSizeLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    cardSizeSlider = new QSlider(Qt::Horizontal, this);
    cardSizeSlider->setRange(50, 250);
    cardSizeSlider->setValue(defaultValue);

    cardSizeLayout->addWidget(cardSizeLabel);
    cardSizeLayout->addWidget(cardSizeSlider);

    if (flowWidget != nullptr) {
        connect(cardSizeSlider, &QSlider::valueChanged, flowWidget, &FlowWidget::setMinimumSizeToMaxSizeHint);
    }

    connect(cardSizeSlider, &QSlider::valueChanged, this, &CardSizeWidget::updateCardSizeSetting);
}

void CardSizeWidget::updateCardSizeSetting(int newValue)
{
    SettingsCache::instance().setPrintingSelectorCardSize(newValue);
}

QSlider *CardSizeWidget::getSlider() const
{
    return cardSizeSlider;
}
