#include "visual_database_display_sub_type_filter_widget.h"

#include "../../../../game/cards/card_database_manager.h"
#include "../../../../game/filters/filter_tree.h"
#include "../../../../game/filters/filter_tree_model.h"

#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTimer>

VisualDatabaseDisplaySubTypeFilterWidget::VisualDatabaseDisplaySubTypeFilterWidget(QWidget *parent,
                                                                                   FilterTreeModel *_filterModel)
    : QWidget(parent), filterModel(_filterModel)
{
    allSubCardTypesWithCount = CardDatabaseManager::getInstance()->getAllSubCardTypesWithCount();

    setMinimumWidth(300);

    layout = new QVBoxLayout(this);
    setLayout(layout);

    // Create and setup the spinbox
    spinBox = new QSpinBox(this);
    spinBox->setMinimum(1);
    spinBox->setMaximum(getMaxSubTypeCount());
    spinBox->setValue(150);
    layout->addWidget(spinBox);
    connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged), this,
            &VisualDatabaseDisplaySubTypeFilterWidget::updateSubTypeButtonsVisibility);

    // Create search box
    searchBox = new QLineEdit(this);
    searchBox->setPlaceholderText(tr("Search subtypes..."));
    layout->addWidget(searchBox);
    connect(searchBox, &QLineEdit::textChanged, this,
            &VisualDatabaseDisplaySubTypeFilterWidget::updateSubTypeButtonsVisibility);

    flowWidget = new FlowWidget(this, Qt::Horizontal, Qt::ScrollBarAlwaysOff, Qt::ScrollBarAsNeeded);
    flowWidget->setMaximumHeight(300);
    layout->addWidget(flowWidget);

    // Toggle button setup (Exact Match / Includes mode)
    toggleButton = new QPushButton(this);
    toggleButton->setCheckable(true);
    layout->addWidget(toggleButton);
    connect(toggleButton, &QPushButton::toggled, this, &VisualDatabaseDisplaySubTypeFilterWidget::updateFilterMode);
    connect(filterModel, &FilterTreeModel::layoutChanged, this, [this]() {
        QTimer::singleShot(100, this, &VisualDatabaseDisplaySubTypeFilterWidget::syncWithFilterModel);
    });

    createSubTypeButtons();  // Populate buttons initially
    updateFilterMode(false); // Initialize the toggle button text

    retranslateUi();
}

void VisualDatabaseDisplaySubTypeFilterWidget::retranslateUi()
{
    spinBox->setToolTip(tr("Do not display card sub-types with less than this amount of cards in the database"));
    toggleButton->setToolTip(tr("Filter mode (AND/OR/NOT conjunctions of filters)"));
}

void VisualDatabaseDisplaySubTypeFilterWidget::createSubTypeButtons()
{
    // Iterate through sub types and create buttons
    for (auto it = allSubCardTypesWithCount.begin(); it != allSubCardTypesWithCount.end(); ++it) {
        auto *button = new QPushButton(it.key(), flowWidget);
        button->setCheckable(true);
        button->setStyleSheet("QPushButton { background-color: lightgray; border: 1px solid gray; padding: 5px; }"
                              "QPushButton:checked { background-color: green; color: white; }");

        flowWidget->addWidget(button);
        typeButtons[it.key()] = button;

        // Connect toggle signal for each button
        connect(button, &QPushButton::toggled, this,
                [this, subType = it.key()](bool checked) { handleSubTypeToggled(subType, checked); });
    }
    updateSubTypeButtonsVisibility(); // Ensure visibility is updated initially
}

void VisualDatabaseDisplaySubTypeFilterWidget::updateSubTypeButtonsVisibility()
{
    int threshold = spinBox->value();
    QString filterText = searchBox->text().trimmed().toLower();

    // Iterate through buttons and hide/disable those below the threshold
    for (auto it = typeButtons.begin(); it != typeButtons.end(); ++it) {
        QString subType = it.key().toLower();
        bool isActive = activeSubTypes.value(it.key(), false);
        bool visible = isActive || (allSubCardTypesWithCount[it.key()] >= threshold &&
                                    (filterText.isEmpty() || subType.contains(filterText)));

        it.value()->setVisible(visible);
        it.value()->setEnabled(visible);
    }
}

int VisualDatabaseDisplaySubTypeFilterWidget::getMaxSubTypeCount() const
{
    int maxCount = 1;
    for (auto it = allSubCardTypesWithCount.begin(); it != allSubCardTypesWithCount.end(); ++it) {
        maxCount = qMax(maxCount, it.value());
    }
    return maxCount;
}

void VisualDatabaseDisplaySubTypeFilterWidget::handleSubTypeToggled(const QString &subType, bool active)
{
    activeSubTypes[subType] = active;

    if (typeButtons.contains(subType)) {
        typeButtons[subType]->setChecked(active);
    }

    updateSubTypeFilter();
}

void VisualDatabaseDisplaySubTypeFilterWidget::updateSubTypeFilter()
{
    filterModel->blockSignals(true);
    // Clear existing filters related to sub types
    filterModel->clearFiltersOfType(CardFilter::Attr::AttrSubType);

    if (exactMatchMode) {
        // Exact Match: Only selected sub types are allowed
        QSet<QString> selectedTypes;
        for (const auto &type : activeSubTypes.keys()) {
            if (activeSubTypes[type]) {
                selectedTypes.insert(type);
            }
        }

        if (!selectedTypes.isEmpty()) {
            // Require all selected subtypes (TypeAnd)
            for (const auto &type : selectedTypes) {
                QString typeString = type;
                filterModel->addFilter(
                    new CardFilter(typeString, CardFilter::Type::TypeAnd, CardFilter::Attr::AttrSubType));
            }

            // Exclude any other types (TypeAndNot)
            for (const auto &type : typeButtons.keys()) {
                if (!selectedTypes.contains(type)) {
                    QString typeString = type;
                    filterModel->addFilter(
                        new CardFilter(typeString, CardFilter::Type::TypeAndNot, CardFilter::Attr::AttrSubType));
                }
            }
        }
    } else {
        // Default Includes Mode (TypeOr) - match any selected subtypes
        for (const auto &type : activeSubTypes.keys()) {
            if (activeSubTypes[type]) {
                QString typeString = type;
                filterModel->addFilter(
                    new CardFilter(typeString, CardFilter::Type::TypeAnd, CardFilter::Attr::AttrSubType));
            }
        }
    }
    filterModel->blockSignals(false);
    filterModel->filterTree()->blockSignals(false);

    emit filterModel->filterTree()->changed();
    emit filterModel->layoutChanged();
}

void VisualDatabaseDisplaySubTypeFilterWidget::updateFilterMode(bool checked)
{
    exactMatchMode = checked;
    toggleButton->setText(exactMatchMode ? tr("Mode: Exact Match") : tr("Mode: Includes"));
    updateSubTypeFilter();
}

void VisualDatabaseDisplaySubTypeFilterWidget::syncWithFilterModel()
{
    // Temporarily block signals for each button to prevent toggling while updating button states
    for (auto it = typeButtons.begin(); it != typeButtons.end(); ++it) {
        it.value()->blockSignals(true);
    }

    // Uncheck all buttons
    for (auto it = typeButtons.begin(); it != typeButtons.end(); ++it) {
        it.value()->setChecked(false);
    }

    // Get active filters for sub types
    QSet<QString> activeTypes;
    for (const auto &filter : filterModel->getFiltersOfType(CardFilter::AttrSubType)) {
        if (filter->type() == CardFilter::Type::TypeAnd) {
            activeTypes.insert(filter->term());
        }
    }

    // Check the buttons for active types
    for (const auto &type : activeTypes) {
        activeSubTypes[type] = true;
        if (typeButtons.contains(type)) {
            typeButtons[type]->setChecked(true);
        }
    }

    // Re-enable signal emissions for each button
    for (auto it = typeButtons.begin(); it != typeButtons.end(); ++it) {
        it.value()->blockSignals(false);
    }

    // Update the visibility of buttons
    updateSubTypeButtonsVisibility();
}
