#include "visual_database_display_main_type_filter_widget.h"

#include "../../../../game/cards/card_database_manager.h"
#include "../../../../game/filters/filter_tree_model.h"

#include <QPushButton>
#include <QSpinBox>

VisualDatabaseDisplayMainTypeFilterWidget::VisualDatabaseDisplayMainTypeFilterWidget(QWidget *parent,
                                                                                     FilterTreeModel *_filterModel)
    : QWidget(parent), filterModel(_filterModel)
{
    allMainCardTypesWithCount = CardDatabaseManager::getInstance()->getAllMainCardTypesWithCount();
    // Get all main card types with their count

    setMinimumWidth(300);

    layout = new QVBoxLayout(this);
    setLayout(layout);

    // Create the spinbox
    spinBox = new QSpinBox(this);
    spinBox->setMinimum(1);
    spinBox->setMaximum(getMaxMainTypeCount()); // Set the max value dynamically
    spinBox->setValue(150);
    layout->addWidget(spinBox);
    connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged), this,
            &VisualDatabaseDisplayMainTypeFilterWidget::updateMainTypeButtonsVisibility);

    flowWidget = new FlowWidget(this, Qt::Horizontal, Qt::ScrollBarAlwaysOff, Qt::ScrollBarAlwaysOff);
    layout->addWidget(flowWidget);

    // Create the toggle button for Exact Match/Includes mode
    toggleButton = new QPushButton(this);
    toggleButton->setCheckable(true);
    layout->addWidget(toggleButton);
    connect(toggleButton, &QPushButton::toggled, this, &VisualDatabaseDisplayMainTypeFilterWidget::updateFilterMode);

    createMainTypeButtons(); // Populate buttons initially
    updateFilterMode(false); // Initialize toggle button text
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
    filterModel->clearFiltersOfType(CardFilter::Attr::AttrType);

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
                    new CardFilter(typeString, CardFilter::Type::TypeAnd, CardFilter::Attr::AttrType));
            }

            // Exclude any other types (TypeAndNot)
            for (const auto &type : typeButtons.keys()) {
                if (!selectedTypes.contains(type)) {
                    QString typeString = type;
                    filterModel->addFilter(
                        new CardFilter(typeString, CardFilter::Type::TypeAndNot, CardFilter::Attr::AttrType));
                }
            }
        }
    } else {
        // Default Includes Mode (TypeOr) - match any selected main types
        for (const auto &type : activeMainTypes.keys()) {
            if (activeMainTypes[type]) {
                QString typeString = type;
                filterModel->addFilter(
                    new CardFilter(typeString, CardFilter::Type::TypeAnd, CardFilter::Attr::AttrType));
            }
        }
    }
}

void VisualDatabaseDisplayMainTypeFilterWidget::updateFilterMode(bool checked)
{
    exactMatchMode = checked;
    toggleButton->setText(exactMatchMode ? tr("Mode: Exact Match") : tr("Mode: Includes"));
    updateMainTypeFilter();
}
