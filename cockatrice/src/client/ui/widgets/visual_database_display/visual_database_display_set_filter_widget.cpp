#include "visual_database_display_set_filter_widget.h"

#include "../../../../game/cards/card_database_manager.h"
#include "../../../../game/filters/filter_tree.h"
#include "../../../../game/filters/filter_tree_model.h"

#include <QLineEdit>
#include <QPushButton>
#include <QTimer>
#include <algorithm>

VisualDatabaseDisplaySetFilterWidget::VisualDatabaseDisplaySetFilterWidget(QWidget *parent,
                                                                           FilterTreeModel *_filterModel)
    : QWidget(parent), filterModel(_filterModel)
{
    setMinimumWidth(300);
    setMaximumHeight(300);

    layout = new QVBoxLayout(this);
    setLayout(layout);

    searchBox = new QLineEdit(this);
    searchBox->setPlaceholderText(tr("Search sets..."));
    layout->addWidget(searchBox);
    connect(searchBox, &QLineEdit::textChanged, this,
            &VisualDatabaseDisplaySetFilterWidget::updateSetButtonsVisibility);

    flowWidget = new FlowWidget(this, Qt::Horizontal, Qt::ScrollBarAlwaysOff, Qt::ScrollBarAsNeeded);
    layout->addWidget(flowWidget);

    // Create the toggle button for Exact Match/Includes mode
    toggleButton = new QPushButton(this);
    toggleButton->setCheckable(true);
    layout->addWidget(toggleButton);
    connect(toggleButton, &QPushButton::toggled, this, &VisualDatabaseDisplaySetFilterWidget::updateFilterMode);
    connect(filterModel, &FilterTreeModel::layoutChanged, this,
            [this]() { QTimer::singleShot(100, this, &VisualDatabaseDisplaySetFilterWidget::syncWithFilterModel); });

    createSetButtons(); // Populate buttons initially
    retranslateUi();
}

void VisualDatabaseDisplaySetFilterWidget::retranslateUi()
{
    toggleButton->setText(exactMatchMode ? tr("Mode: Exact Match") : tr("Mode: Includes"));
}

void VisualDatabaseDisplaySetFilterWidget::createSetButtons()
{
    SetList shared_pointerses = CardDatabaseManager::getInstance()->getSetList();

    // Sort by release date
    std::sort(shared_pointerses.begin(), shared_pointerses.end(),
              [](const auto &a, const auto &b) { return a->getReleaseDate() > b->getReleaseDate(); });

    int setsToPreactivate = 10;
    int setsActivated = 0;

    for (const auto &shared_pointer : shared_pointerses) {
        QString shortName = shared_pointer->getShortName();
        QString longName = shared_pointer->getLongName();

        auto *button = new QPushButton(longName + " (" + shortName + ")", flowWidget);
        button->setCheckable(true);
        button->setStyleSheet("QPushButton { background-color: lightgray; border: 1px solid gray; padding: 5px; }"
                              "QPushButton:checked { background-color: green; color: white; }");

        flowWidget->addWidget(button);
        setButtons[shortName] = button;

        // Connect toggle signal
        connect(button, &QPushButton::toggled, this,
                [this, shortName](bool checked) { handleSetToggled(shortName, checked); });
        if (setsActivated < setsToPreactivate) {
            setsActivated++;
            activeSets[shortName] = true;
            button->setChecked(true);
        }
    }
    updateSetFilter();
    updateSetButtonsVisibility(); // Ensure visibility is updated initially
}

void VisualDatabaseDisplaySetFilterWidget::updateSetButtonsVisibility()
{
    QString filterText = searchBox->text().trimmed().toLower();

    for (const QString &setName : setButtons.keys()) {
        QPushButton *buttonForSet = setButtons[setName];
        QString shortName = setName.toLower();
        QString longName = buttonForSet->text().toLower();
        bool alwaysVisible = activeSets.contains(setName) && activeSets[setName];
        bool visible =
            alwaysVisible || filterText.isEmpty() || shortName.contains(filterText) || longName.contains(filterText);
        buttonForSet->setVisible(visible);
    }
}

void VisualDatabaseDisplaySetFilterWidget::handleSetToggled(const QString &setShortName, bool active)
{
    activeSets[setShortName] = active;

    if (setButtons.contains(setShortName)) {
        setButtons[setShortName]->setChecked(active);
    }

    updateSetFilter();
}

void VisualDatabaseDisplaySetFilterWidget::updateSetFilter()
{
    // Clear existing filters related to sets
    filterModel->blockSignals(true);
    filterModel->filterTree()->blockSignals(true);
    filterModel->clearFiltersOfType(CardFilter::Attr::AttrSet);

    if (exactMatchMode) {
        // Exact Match: Only selected sets are allowed
        QSet<QString> selectedSets;
        for (const auto &set : activeSets.keys()) {
            if (activeSets[set]) {
                selectedSets.insert(set);
            }
        }

        if (!selectedSets.isEmpty()) {
            // Require all selected sets (TypeAnd)
            for (const auto &set : selectedSets) {
                QString setString = set;
                filterModel->addFilter(new CardFilter(setString, CardFilter::Type::TypeAnd, CardFilter::Attr::AttrSet));
            }

            // Exclude any other sets (TypeAndNot)
            for (const auto &set : setButtons.keys()) {
                if (!selectedSets.contains(set)) {
                    QString setString = set;
                    filterModel->addFilter(
                        new CardFilter(setString, CardFilter::Type::TypeAndNot, CardFilter::Attr::AttrSet));
                }
            }
        }
    } else {
        // Default Includes Mode (TypeOr) - match any selected sets
        for (const auto &set : activeSets.keys()) {
            if (activeSets[set]) {
                QString setString = set;
                filterModel->addFilter(new CardFilter(setString, CardFilter::Type::TypeOr, CardFilter::Attr::AttrSet));
            }
        }
    }
    filterModel->blockSignals(false);
    filterModel->filterTree()->blockSignals(false);

    emit filterModel->filterTree()->changed();
    emit filterModel->layoutChanged();
}

void VisualDatabaseDisplaySetFilterWidget::syncWithFilterModel()
{
    // Clear activeSets
    activeSets.clear();

    // Read the current filters in filterModel
    auto currentFilters = filterModel->getFiltersOfType(CardFilter::Attr::AttrSet);

    // Determine if we're in Exact Match mode or Includes mode
    QSet<QString> selectedSets;
    QSet<QString> excludedSets;
    for (const auto &filter : currentFilters) {
        if (filter->type() == CardFilter::Type::TypeAnd) {
            selectedSets.insert(filter->term());
        } else if (filter->type() == CardFilter::Type::TypeAndNot) {
            excludedSets.insert(filter->term());
        } else if (filter->type() == CardFilter::Type::TypeOr) {
            selectedSets.insert(filter->term());
        }
    }

    // Determine exact match mode based on filter structure
    bool newExactMatchMode = !excludedSets.isEmpty();

    if (newExactMatchMode != exactMatchMode) {
        exactMatchMode = newExactMatchMode;
        toggleButton->setText(exactMatchMode ? tr("Mode: Exact Match") : tr("Mode: Includes"));
    }

    // Sync button states with selected sets
    for (const auto &key : setButtons.keys()) {
        bool active = selectedSets.contains(key);
        activeSets[key] = active;
        setButtons[key]->setChecked(active);
    }
}

void VisualDatabaseDisplaySetFilterWidget::updateFilterMode(bool checked)
{
    exactMatchMode = checked;
    updateSetFilter();
    retranslateUi();
}
