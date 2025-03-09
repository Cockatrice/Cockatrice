#include "visual_database_display_sub_type_filter_widget.h"

#include "../../../../game/cards/card_database_manager.h"
#include "../../../../game/filters/filter_tree_model.h"

#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>

VisualDatabaseDisplaySubTypeFilterWidget::VisualDatabaseDisplaySubTypeFilterWidget(QWidget *parent,
                                                                                   FilterTreeModel *_filterModel)
    : QWidget(parent), filterModel(_filterModel)
{
    allSubCardTypesWithCount = CardDatabaseManager::getInstance()->getAllSubCardTypesWithCount();

    setMinimumWidth(300);

    layout = new QVBoxLayout(this);
    setLayout(layout);

    // Create the spinbox
    spinBox = new QSpinBox(this);
    spinBox->setMinimum(1);
    spinBox->setMaximum(getMaxSubTypeCount());
    spinBox->setValue(150);
    layout->addWidget(spinBox);
    connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged), this,
            &VisualDatabaseDisplaySubTypeFilterWidget::updateSubTypeButtonsVisibility);

    // Create the search box
    searchBox = new QLineEdit(this);
    searchBox->setPlaceholderText(tr("Search subtypes..."));
    layout->addWidget(searchBox);
    connect(searchBox, &QLineEdit::textChanged, this,
            &VisualDatabaseDisplaySubTypeFilterWidget::updateSubTypeButtonsVisibility);

    flowWidget = new FlowWidget(this, Qt::Horizontal, Qt::ScrollBarAlwaysOff, Qt::ScrollBarAsNeeded);
    flowWidget->setMaximumHeight(300);
    layout->addWidget(flowWidget);

    // Create the toggle button for Exact Match/Includes mode
    toggleButton = new QPushButton(this);
    toggleButton->setCheckable(true);
    layout->addWidget(toggleButton);
    connect(toggleButton, &QPushButton::toggled, this, &VisualDatabaseDisplaySubTypeFilterWidget::updateFilterMode);

    createSubTypeButtons(); // Populate buttons initially
    updateFilterMode(false);
}

void VisualDatabaseDisplaySubTypeFilterWidget::createSubTypeButtons()
{
    // Iterate through main types and create buttons
    for (auto it = allSubCardTypesWithCount.begin(); it != allSubCardTypesWithCount.end(); ++it) {
        auto *button = new QPushButton(it.key(), flowWidget);
        button->setCheckable(true);
        button->setStyleSheet("QPushButton { background-color: lightgray; border: 1px solid gray; padding: 5px; }"
                              "QPushButton:checked { background-color: green; color: white; }");

        flowWidget->addWidget(button);
        typeButtons[it.key()] = button;

        // Connect toggle signal
        connect(button, &QPushButton::toggled, this,
                [this, mainType = it.key()](bool checked) { handleSubTypeToggled(mainType, checked); });
    }
    updateSubTypeButtonsVisibility(); // Ensure visibility is updated initially
}

void VisualDatabaseDisplaySubTypeFilterWidget::updateSubTypeButtonsVisibility()
{
    int threshold = spinBox->value();
    QString filterText = searchBox->text().trimmed().toLower();

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

void VisualDatabaseDisplaySubTypeFilterWidget::handleSubTypeToggled(const QString &mainType, bool active)
{
    activeSubTypes[mainType] = active;

    if (typeButtons.contains(mainType)) {
        typeButtons[mainType]->setChecked(active);
    }

    updateSubTypeFilter();
}

void VisualDatabaseDisplaySubTypeFilterWidget::updateSubTypeFilter()
{
    // Clear existing filters related to main type
    filterModel->clearFiltersOfType(CardFilter::Attr::AttrType);

    if (exactMatchMode) {
        // Exact Match: Only selected main types are allowed
        QSet<QString> selectedTypes;
        for (const auto &type : activeSubTypes.keys()) {
            if (activeSubTypes[type]) {
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
        for (const auto &type : activeSubTypes.keys()) {
            if (activeSubTypes[type]) {
                QString typeString = type;
                filterModel->addFilter(
                    new CardFilter(typeString, CardFilter::Type::TypeAnd, CardFilter::Attr::AttrType));
            }
        }
    }
}

void VisualDatabaseDisplaySubTypeFilterWidget::updateFilterMode(bool checked)
{
    exactMatchMode = checked;
    toggleButton->setText(exactMatchMode ? tr("Mode: Exact Match") : tr("Mode: Includes"));
    updateSubTypeFilter();
}
