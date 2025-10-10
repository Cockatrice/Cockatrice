#include "visual_database_display_main_type_filter_widget.h"

#include "../../../filters/filter_tree.h"
#include "../../../filters/filter_tree_model.h"

#include <QPushButton>
#include <QSpinBox>
#include <QTimer>
#include <libcockatrice/card/database/card_database_manager.h>

VisualDatabaseDisplayMainTypeFilterWidget::VisualDatabaseDisplayMainTypeFilterWidget(QWidget *parent,
                                                                                     FilterTreeModel *_filterModel)
    : QWidget(parent), filterModel(_filterModel)
{
    allMainCardTypesWithCount = CardDatabaseManager::query()->getAllMainCardTypesWithCount();
    // Get all main card types with their count

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
    connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged), this,
            &VisualDatabaseDisplayMainTypeFilterWidget::updateMainTypeButtonsVisibility);

    // Create the toggle button for Exact Match/Includes mode
    toggleButton = new QPushButton(this);
    toggleButton->setCheckable(true);
    layout->addWidget(toggleButton);
    connect(toggleButton, &QPushButton::toggled, this, &VisualDatabaseDisplayMainTypeFilterWidget::updateFilterMode);
    connect(filterModel, &FilterTreeModel::layoutChanged, this, [this]() {
        QTimer::singleShot(100, this, &VisualDatabaseDisplayMainTypeFilterWidget::syncWithFilterModel);
    });

    createMainTypeButtons(); // Populate buttons initially
    updateFilterMode(false); // Initialize toggle button text

    retranslateUi();
}

void VisualDatabaseDisplayMainTypeFilterWidget::retranslateUi()
{
    spinBox->setToolTip(tr("Do not display card main-types with less than this amount of cards in the database"));
    toggleButton->setToolTip(tr("Filter mode (AND/OR/NOT conjunctions of filters)"));
}

void VisualDatabaseDisplayMainTypeFilterWidget::createMainTypeButtons()
{
    // Iterate through main types and create buttons
    for (auto it = allMainCardTypesWithCount.begin(); it != allMainCardTypesWithCount.end(); ++it) {
        auto *button = new QPushButton(it.key(), flowWidget);
        button->setCheckable(true);
        button->setStyleSheet("QPushButton { background-color: lightgray; border: 1px solid gray; padding: 5px; }"
                              "QPushButton:checked { background-color: green; color: white; }");

        flowWidget->addWidget(button);
        typeButtons[it.key()] = button;

        // Connect toggle signal
        connect(button, &QPushButton::toggled, this,
                [this, mainType = it.key()](bool checked) { handleMainTypeToggled(mainType, checked); });
    }
    updateMainTypeButtonsVisibility(); // Ensure visibility is updated initially
}

void VisualDatabaseDisplayMainTypeFilterWidget::updateMainTypeButtonsVisibility()
{
    int threshold = spinBox->value(); // Get the current spinbox value

    // Iterate through buttons and hide/disable those below the threshold
    for (auto it = typeButtons.begin(); it != typeButtons.end(); ++it) {
        bool visible = allMainCardTypesWithCount[it.key()] >= threshold;
        it.value()->setVisible(visible);
        it.value()->setEnabled(visible);
    }
}

int VisualDatabaseDisplayMainTypeFilterWidget::getMaxMainTypeCount() const
{
    int maxCount = 1;
    for (auto it = allMainCardTypesWithCount.begin(); it != allMainCardTypesWithCount.end(); ++it) {
        maxCount = qMax(maxCount, it.value());
    }
    return maxCount;
}

void VisualDatabaseDisplayMainTypeFilterWidget::handleMainTypeToggled(const QString &mainType, bool active)
{
    activeMainTypes[mainType] = active;

    if (typeButtons.contains(mainType)) {
        typeButtons[mainType]->setChecked(active);
    }

    updateMainTypeFilter();
}

void VisualDatabaseDisplayMainTypeFilterWidget::updateMainTypeFilter()
{
    // Clear existing filters related to main type
    filterModel->blockSignals(true);
    filterModel->filterTree()->blockSignals(true);
    filterModel->clearFiltersOfType(CardFilter::Attr::AttrMainType);

    if (exactMatchMode) {
        // Exact Match: Only selected main types are allowed
        QSet<QString> selectedTypes;
        for (const auto &type : activeMainTypes.keys()) {
            if (activeMainTypes[type]) {
                selectedTypes.insert(type);
            }
        }

        if (!selectedTypes.isEmpty()) {
            // Require all selected types (TypeAnd)
            for (const auto &type : selectedTypes) {
                QString typeString = type;
                filterModel->addFilter(
                    new CardFilter(typeString, CardFilter::Type::TypeAnd, CardFilter::Attr::AttrMainType));
            }

            // Exclude any other types (TypeAndNot)
            for (const auto &type : typeButtons.keys()) {
                if (!selectedTypes.contains(type)) {
                    QString typeString = type;
                    filterModel->addFilter(
                        new CardFilter(typeString, CardFilter::Type::TypeAndNot, CardFilter::Attr::AttrMainType));
                }
            }
        }
    } else {
        // Default Includes Mode (TypeOr) - match any selected main types
        for (const auto &type : activeMainTypes.keys()) {
            if (activeMainTypes[type]) {
                QString typeString = type;
                filterModel->addFilter(
                    new CardFilter(typeString, CardFilter::Type::TypeAnd, CardFilter::Attr::AttrMainType));
            }
        }
    }

    filterModel->blockSignals(false);
    filterModel->filterTree()->blockSignals(false);

    emit filterModel->filterTree()->changed();
    emit filterModel->layoutChanged();
}

void VisualDatabaseDisplayMainTypeFilterWidget::updateFilterMode(bool checked)
{
    exactMatchMode = checked;
    toggleButton->setText(exactMatchMode ? tr("Mode: Exact Match") : tr("Mode: Includes"));
    updateMainTypeFilter();
}

void VisualDatabaseDisplayMainTypeFilterWidget::syncWithFilterModel()
{
    // Temporarily block signals for each button to prevent toggling while updating button states
    for (auto it = typeButtons.begin(); it != typeButtons.end(); ++it) {
        it.value()->blockSignals(true);
    }

    // Uncheck all buttons
    for (auto it = typeButtons.begin(); it != typeButtons.end(); ++it) {
        it.value()->setChecked(false);
    }

    // Get active filters for main types
    QSet<QString> activeTypes;
    for (const auto &filter : filterModel->getFiltersOfType(CardFilter::AttrMainType)) {
        if (filter->type() == CardFilter::Type::TypeAnd) {
            activeTypes.insert(filter->term());
        }
    }

    // Check the buttons for active types
    for (const auto &type : activeTypes) {
        activeMainTypes[type] = true;
        if (typeButtons.contains(type)) {
            typeButtons[type]->setChecked(true);
        }
    }

    // Re-enable signal emissions for each button
    for (auto it = typeButtons.begin(); it != typeButtons.end(); ++it) {
        it.value()->blockSignals(false);
    }

    // Update the visibility of buttons
    updateMainTypeButtonsVisibility();
}
