#include "deck_preview_color_identity_filter_widget.h"

#include "../../cards/additional_info/mana_symbol_widget.h"
#include "deck_preview_widget.h"

#include <QMouseEvent>

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
    connect(this, &DeckPreviewColorIdentityFilterWidget::activeColorsChanged, parent,
            &VisualDeckStorageWidget::updateColorFilter);
    connect(this, &DeckPreviewColorIdentityFilterWidget::filterModeChanged, parent,
            &VisualDeckStorageWidget::updateColorFilter);

    // Call retranslateUi to set the initial text
    retranslateUi();
}

void DeckPreviewColorIdentityFilterWidget::retranslateUi()
{
    // Set the toggle button text based on the current mode
    switch (filterMode) {
        case ExactMatch:
            toggleButton->setText(tr("Mode: Exact Match"));
            break;
        case Includes:
            toggleButton->setText(tr("Mode: Includes"));
            break;
        case Excludes:
            toggleButton->setText(tr("Mode: Excludes"));
            break;
    }
    toggleButton->setToolTip(tr("Color identity filter mode (AND/OR/NOT conjunctions of filters)"));
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
        case ExactMatch:
            filterMode = Includes;
            break;
        case Includes:
            filterMode = Excludes;
            break;
        case Excludes:
            filterMode = ExactMatch;
            break;
    }

    retranslateUi(); // Update the button text
    emit filterModeChanged(filterMode);
}

void DeckPreviewColorIdentityFilterWidget::filterWidgets(QList<DeckPreviewWidget *> widgets)
{
    // Check if no colors are active
    bool noColorsActive = true;
    for (auto it = activeColors.constBegin(); it != activeColors.constEnd(); ++it) {
        if (it.value()) {
            noColorsActive = false;
            break;
        }
    }

    // If no colors are active, return the unfiltered list of widgets
    if (noColorsActive) {
        for (DeckPreviewWidget *previewWidget : widgets) {
            previewWidget->filteredByColor = false;
        }
        return;
    }

    for (const auto &widget : widgets) {
        QString colorIdentity = widget->getColorIdentity();

        bool matchesFilter = true;
        switch (filterMode) {
            case ExactMatch: {
                // Exact match mode: active colors must exactly match colorIdentity

                // Create a set of active colors
                QSet<QChar> activeColorSet;
                for (auto it = activeColors.constBegin(); it != activeColors.constEnd(); ++it) {
                    if (it.value()) {
                        activeColorSet.insert(it.key().toUpper()); // Use uppercase for uniformity
                    }
                }

                // Create a set of colors from the color identity string
                QSet<QChar> colorIdentitySet;
                for (const QChar &color : colorIdentity) {
                    colorIdentitySet.insert(color.toUpper()); // Ensure case uniformity
                }

                // Compare the sets: the sets must match exactly
                if (activeColorSet != colorIdentitySet) {
                    matchesFilter = false;
                }
                break;
            }
            case Includes:
                // Includes mode: colorIdentity must contain all active colors
                for (auto it = activeColors.constBegin(); it != activeColors.constEnd(); ++it) {
                    if (it.value() && !colorIdentity.contains(it.key())) {
                        matchesFilter = false;
                        break;
                    }
                }
                break;
            case Excludes:
                // Excludes mode: colorIdentity must contain none of the active colors
                for (auto it = activeColors.constBegin(); it != activeColors.constEnd(); ++it) {
                    if (it.value() && colorIdentity.contains(it.key())) {
                        matchesFilter = false;
                        break;
                    }
                }
                break;
        }

        widget->filteredByColor = !matchesFilter;
    }
}
