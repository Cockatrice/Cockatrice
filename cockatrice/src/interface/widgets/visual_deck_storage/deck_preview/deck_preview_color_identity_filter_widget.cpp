#include "deck_preview_color_identity_filter_widget.h"

#include "../../cards/additional_info/mana_symbol_widget.h"
#include "../visual_deck_storage_widget.h"

#include <QSet>

DeckPreviewColorIdentityFilterWidget::DeckPreviewColorIdentityFilterWidget(VisualDeckStorageWidget *parent)
    : QWidget(parent), layout(new QHBoxLayout(this))
{
    setLayout(layout);
    layout->setSpacing(5);
    layout->setContentsMargins(0, 0, 0, 0);

    QString fullColorIdentity = "WUBRG";
    for (const QChar &color : fullColorIdentity) {
        auto *manaSymbol = new ManaSymbolWidget(this, color, false, true);
        manaSymbol->setFixedWidth(25);

        layout->addWidget(manaSymbol);

        // Initialize the activeColors map
        activeColors[color] = false;

        // Connect the color toggled signal
        connect(manaSymbol, &ManaSymbolWidget::colorToggled, this,
                &DeckPreviewColorIdentityFilterWidget::handleColorToggled);
    }

    toggleButton = new QPushButton(this);
    layout->addWidget(toggleButton);

    // Connect the button's clicked signal
    connect(toggleButton, &QPushButton::clicked, this, &DeckPreviewColorIdentityFilterWidget::updateFilterMode);

    // Call retranslateUi to set the initial text
    retranslateUi();
}

void DeckPreviewColorIdentityFilterWidget::retranslateUi()
{
    // Set the toggle button text based on the current mode
    switch (filterMode) {
        case VisualDeckStorageSortFilterProxyModel::FilterMode::ExactMatch:
            toggleButton->setText(tr("Mode: Exact Match"));
            break;
        case VisualDeckStorageSortFilterProxyModel::FilterMode::Includes:
            toggleButton->setText(tr("Mode: Includes"));
            break;
        case VisualDeckStorageSortFilterProxyModel::FilterMode::Excludes:
            toggleButton->setText(tr("Mode: Excludes"));
            break;
    }
    toggleButton->setToolTip(tr("Color identity filter mode (AND/OR/NOT conjunctions of filters)"));
}

/**
 * @brief The colors that are currently toggled on.
 */
QSet<QChar> DeckPreviewColorIdentityFilterWidget::getActiveColors() const
{
    QSet<QChar> activeColorSet;
    for (auto it = activeColors.constBegin(); it != activeColors.constEnd(); ++it) {
        if (it.value()) {
            activeColorSet.insert(it.key());
        }
    }
    return activeColorSet;
}

void DeckPreviewColorIdentityFilterWidget::handleColorToggled(QChar color, bool active)
{
    activeColors[color] = active;
    emit activeColorsChanged();
}

void DeckPreviewColorIdentityFilterWidget::updateFilterMode()
{
    // Cycle through the modes
    switch (filterMode) {
        case VisualDeckStorageSortFilterProxyModel::FilterMode::ExactMatch:
            filterMode = VisualDeckStorageSortFilterProxyModel::FilterMode::Includes;
            break;
        case VisualDeckStorageSortFilterProxyModel::FilterMode::Includes:
            filterMode = VisualDeckStorageSortFilterProxyModel::FilterMode::Excludes;
            break;
        case VisualDeckStorageSortFilterProxyModel::FilterMode::Excludes:
            filterMode = VisualDeckStorageSortFilterProxyModel::FilterMode::ExactMatch;
            break;
    }

    retranslateUi(); // Update the button text
    emit filterModeChanged(filterMode);
}
