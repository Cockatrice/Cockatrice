#include "card_size_widget.h"
#include "../../../../settings/cache_settings.h"
#include "../printing_selector/printing_selector.h"
#include "../visual_deck_storage/visual_deck_storage_widget.h"

CardSizeWidget::CardSizeWidget(QWidget *parent, FlowWidget *flowWidget, int defaultValue)
    : parent(parent), flowWidget(flowWidget)
{
    cardSizeLayout = new QHBoxLayout(this);
    cardSizeLayout->setContentsMargins(9, 0, 9, 0);
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

    // Debounce setup
    debounceTimer.setSingleShot(true);
    connect(&debounceTimer, &QTimer::timeout, this, [this]() {
        if (qobject_cast<PrintingSelector *>(parentWidget())) {
            SettingsCache::instance().setPrintingSelectorCardSize(pendingValue);
        } else if (qobject_cast<VisualDeckStorageWidget *>(parentWidget())) {
            SettingsCache::instance().setVisualDeckStorageCardSize(pendingValue);
        }
    });

    connect(cardSizeSlider, &QSlider::valueChanged, this, &CardSizeWidget::updateCardSizeSetting);
}

void CardSizeWidget::updateCardSizeSetting(int newValue)
{
    pendingValue = newValue;
    debounceTimer.start(300); // 300ms debounce time
}

QSlider *CardSizeWidget::getSlider() const
{
    return cardSizeSlider;
}
