#include "card_size_widget.h"

#include "../../../../settings/cache_settings.h"
#include "../printing_selector/printing_selector.h"
#include "../visual_deck_storage/visual_deck_storage_widget.h"


/**
 * @class CardSizeWidget
 * @brief A widget for adjusting card sizes using a slider.
 *
 * This widget allows users to dynamically change the card size in a linked FlowWidget
 * and updates the application's settings accordingly.
 */
CardSizeWidget::CardSizeWidget(QWidget *parent, FlowWidget *flowWidget, int defaultValue)
    : parent(parent), flowWidget(flowWidget)
{
    cardSizeLayout = new QHBoxLayout(this);
    cardSizeLayout->setContentsMargins(9, 0, 9, 0);
    setLayout(cardSizeLayout);

    cardSizeLabel = new QLabel(tr("Card Size"), this);
    cardSizeLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    cardSizeSlider = new QSlider(Qt::Horizontal, this);
    cardSizeSlider->setRange(50, 250);      ///< Slider range for card size adjustment.
    cardSizeSlider->setValue(defaultValue); ///< Initial slider value.

    cardSizeLayout->addWidget(cardSizeLabel);
    cardSizeLayout->addWidget(cardSizeSlider);

    if (flowWidget != nullptr) {
        connect(cardSizeSlider, &QSlider::valueChanged, flowWidget, &FlowWidget::setMinimumSizeToMaxSizeHint);
    }

    connect(cardSizeSlider, &QSlider::valueChanged, this, &CardSizeWidget::updateCardSizeSetting);
}

/**
 * @brief Updates the card size setting in the application's cache.
 *
 * @param newValue The new card size value set by the slider.
 */
void CardSizeWidget::updateCardSizeSetting(int newValue)
{
    // Check the type of the parent widget
    if ((parent = qobject_cast<PrintingSelector*>(parentWidget()))) {
        SettingsCache::instance().setPrintingSelectorCardSize(newValue);
    }
    else if ((parent = qobject_cast<VisualDeckStorageWidget*>(parentWidget()))) {
        SettingsCache::instance().setVisualDeckStorageCardSize(newValue);
    }
    else {
        qWarning() << "CardSizeWidget has an unexpected parent widget type. Did not set persistent setting.";
    }
}


/**
 * @brief Gets the slider widget used for adjusting the card size.
 *
 * @return A pointer to the QSlider object.
 */
QSlider *CardSizeWidget::getSlider() const
{
    return cardSizeSlider;
}
