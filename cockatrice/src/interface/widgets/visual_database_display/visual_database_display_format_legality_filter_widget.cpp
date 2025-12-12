#include "visual_database_display_format_legality_filter_widget.h"

#include "../../../filters/filter_tree_model.h"

#include <QPushButton>
#include <QSpinBox>
#include <QTimer>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/filters/filter_tree.h>

VisualDatabaseDisplayFormatLegalityFilterWidget::VisualDatabaseDisplayFormatLegalityFilterWidget(
    QWidget *parent,
    FilterTreeModel *_filterModel)
    : QWidget(parent), filterModel(_filterModel)
{
    allFormatsWithCount = CardDatabaseManager::query()->getAllFormatsWithCount();

    setMaximumHeight(75);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);

    layout = new QHBoxLayout(this);
    setLayout(layout);
    layout->setContentsMargins(0, 1, 0, 1);
    layout->setSpacing(1);
    layout->setAlignment(Qt::AlignTop);

    flowWidget = new FlowWidget(this, Qt::Horizontal, Qt::ScrollBarAlwaysOff, Qt::ScrollBarAsNeeded);
    layout->addWidget(flowWidget);

    // Create the spinbox
    spinBox = new QSpinBox(this);
    spinBox->setMinimum(1);
    spinBox->setMaximum(getMaxMainTypeCount()); // Set the max value dynamically
    spinBox->setValue(150);
    layout->addWidget(spinBox);
    connect(spinBox, qOverload<int>(&QSpinBox::valueChanged), this,
            &VisualDatabaseDisplayFormatLegalityFilterWidget::updateFormatButtonsVisibility);

    // Create the toggle button for Exact Match/Includes mode
    toggleButton = new QPushButton(this);
    toggleButton->setCheckable(true);
    layout->addWidget(toggleButton);
    connect(toggleButton, &QPushButton::toggled, this,
            &VisualDatabaseDisplayFormatLegalityFilterWidget::updateFilterMode);
    connect(filterModel, &FilterTreeModel::layoutChanged, this, [this]() {
        QTimer::singleShot(100, this, &VisualDatabaseDisplayFormatLegalityFilterWidget::syncWithFilterModel);
    });

    createFormatButtons();   // Populate buttons initially
    updateFilterMode(false); // Initialize toggle button text

    retranslateUi();
}

void VisualDatabaseDisplayFormatLegalityFilterWidget::retranslateUi()
{
    spinBox->setToolTip(tr("Do not display formats with less than this amount of cards in the database"));
    toggleButton->setToolTip(tr("Filter mode (AND/OR/NOT conjunctions of filters)"));
}

void VisualDatabaseDisplayFormatLegalityFilterWidget::createFormatButtons()
{
    // Iterate through main types and create buttons
    for (auto it = allFormatsWithCount.begin(); it != allFormatsWithCount.end(); ++it) {
        auto *button = new QPushButton(it.key(), flowWidget);
        button->setCheckable(true);
        button->setStyleSheet("QPushButton { background-color: lightgray; border: 1px solid gray; padding: 5px; }"
                              "QPushButton:checked { background-color: green; color: white; }");

        flowWidget->addWidget(button);
        formatButtons[it.key()] = button;

        // Connect toggle signal
        connect(button, &QPushButton::toggled, this,
                [this, mainType = it.key()](bool checked) { handleFormatToggled(mainType, checked); });
    }
    updateFormatButtonsVisibility(); // Ensure visibility is updated initially
}

void VisualDatabaseDisplayFormatLegalityFilterWidget::updateFormatButtonsVisibility()
{
    int threshold = spinBox->value(); // Get the current spinbox value

    // Iterate through buttons and hide/disable those below the threshold
    for (auto it = formatButtons.begin(); it != formatButtons.end(); ++it) {
        bool visible = allFormatsWithCount[it.key()] >= threshold;
        it.value()->setVisible(visible);
        it.value()->setEnabled(visible);
    }
}

int VisualDatabaseDisplayFormatLegalityFilterWidget::getMaxMainTypeCount() const
{
    int maxCount = 1;
    for (auto it = allFormatsWithCount.begin(); it != allFormatsWithCount.end(); ++it) {
        maxCount = qMax(maxCount, it.value());
    }
    return maxCount;
}

void VisualDatabaseDisplayFormatLegalityFilterWidget::handleFormatToggled(const QString &format, bool active)
{
    activeFormats[format] = active;

    if (formatButtons.contains(format)) {
        formatButtons[format]->setChecked(active);
    }

    updateFormatFilter();
}

void VisualDatabaseDisplayFormatLegalityFilterWidget::updateFormatFilter()
{
    // Clear existing filters related to main type
    filterModel->blockSignals(true);
    filterModel->filterTree()->blockSignals(true);
    filterModel->clearFiltersOfType(CardFilter::Attr::AttrFormat);

    if (exactMatchMode) {
        // Exact Match: Only selected main types are allowed
        QSet<QString> selectedTypes;
        for (const auto &type : activeFormats.keys()) {
            if (activeFormats[type]) {
                selectedTypes.insert(type);
            }
        }

        if (!selectedTypes.isEmpty()) {
            // Require all selected types (TypeAnd)
            for (const auto &type : selectedTypes) {
                QString typeString = type;
                filterModel->addFilter(
                    new CardFilter(typeString, CardFilter::Type::TypeAnd, CardFilter::Attr::AttrFormat));
            }

            // Exclude any other types (TypeAndNot)
            for (const auto &type : formatButtons.keys()) {
                if (!selectedTypes.contains(type)) {
                    QString typeString = type;
                    filterModel->addFilter(
                        new CardFilter(typeString, CardFilter::Type::TypeAndNot, CardFilter::Attr::AttrFormat));
                }
            }
        }
    } else {
        // Default Includes Mode (TypeOr) - match any selected main types
        for (const auto &type : activeFormats.keys()) {
            if (activeFormats[type]) {
                QString typeString = type;
                filterModel->addFilter(
                    new CardFilter(typeString, CardFilter::Type::TypeAnd, CardFilter::Attr::AttrFormat));
            }
        }
    }

    filterModel->blockSignals(false);
    filterModel->filterTree()->blockSignals(false);

    emit filterModel->filterTree()->changed();
    emit filterModel->layoutChanged();
}

void VisualDatabaseDisplayFormatLegalityFilterWidget::updateFilterMode(bool checked)
{
    exactMatchMode = checked;
    toggleButton->setText(exactMatchMode ? tr("Mode: Exact Match") : tr("Mode: Includes"));
    updateFormatFilter();
}

void VisualDatabaseDisplayFormatLegalityFilterWidget::syncWithFilterModel()
{
    // Temporarily block signals for each button to prevent toggling while updating button states
    for (auto it = formatButtons.begin(); it != formatButtons.end(); ++it) {
        it.value()->blockSignals(true);
    }

    // Uncheck all buttons
    for (auto it = formatButtons.begin(); it != formatButtons.end(); ++it) {
        it.value()->setChecked(false);
    }

    // Get active filters for main types
    QSet<QString> activeTypes;
    for (const auto &filter : filterModel->getFiltersOfType(CardFilter::AttrFormat)) {
        if (filter->type() == CardFilter::Type::TypeAnd) {
            activeTypes.insert(filter->term());
        }
    }

    // Check the buttons for active types
    for (const auto &type : activeTypes) {
        activeFormats[type] = true;
        if (formatButtons.contains(type)) {
            formatButtons[type]->setChecked(true);
        }
    }

    // Re-enable signal emissions for each button
    for (auto it = formatButtons.begin(); it != formatButtons.end(); ++it) {
        it.value()->blockSignals(false);
    }

    // Update the visibility of buttons
    updateFormatButtonsVisibility();
}
