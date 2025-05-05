#include "visual_database_display_name_filter_widget.h"

#include "../../../tabs/abstract_tab_deck_editor.h"

#include <QHBoxLayout>

VisualDatabaseDisplayNameFilterWidget::VisualDatabaseDisplayNameFilterWidget(QWidget *parent,
                                                                             AbstractTabDeckEditor *_deckEditor,
                                                                             FilterTreeModel *_filterModel)
    : QWidget(parent), deckEditor(_deckEditor), filterModel(_filterModel)
{
    setMinimumWidth(300);
    setMaximumHeight(300);

    layout = new QVBoxLayout(this);
    setLayout(layout);

    searchBox = new QLineEdit(this);
    layout->addWidget(searchBox);

    connect(searchBox, &QLineEdit::returnPressed, this, [this]() {
        QString text = searchBox->text().trimmed();
        if (!text.isEmpty() && !activeFilters.contains(text)) {
            createNameFilter(text);
            searchBox->clear();
            updateFilterModel();
        }
    });

    // Create container for active filters
    flowWidget = new FlowWidget(this, Qt::Horizontal, Qt::ScrollBarAlwaysOff, Qt::ScrollBarAsNeeded);
    layout->addWidget(flowWidget);

    loadFromDeckButton = new QPushButton(this);
    layout->addWidget(loadFromDeckButton);

    connect(loadFromDeckButton, &QPushButton::clicked, this, &VisualDatabaseDisplayNameFilterWidget::actLoadFromDeck);
    connect(filterModel, &FilterTreeModel::layoutChanged, this,
            [this]() { QTimer::singleShot(100, this, &VisualDatabaseDisplayNameFilterWidget::syncWithFilterModel); });

    retranslateUi();
}

void VisualDatabaseDisplayNameFilterWidget::retranslateUi()
{
    searchBox->setPlaceholderText(tr("Filter by name..."));
    loadFromDeckButton->setText(tr("Load from Deck"));
    loadFromDeckButton->setToolTip(tr("Apply all card names in currently loaded deck as exact match name filters"));
}

void VisualDatabaseDisplayNameFilterWidget::actLoadFromDeck()
{
    DeckListModel *deckListModel = deckEditor->deckDockWidget->deckModel;

    if (!deckListModel)
        return;
    DeckList *decklist = deckListModel->getDeckList();
    if (!decklist)
        return;
    InnerDecklistNode *listRoot = decklist->getRoot();
    if (!listRoot)
        return;

    for (int i = 0; i < listRoot->size(); i++) {
        InnerDecklistNode *currentZone = dynamic_cast<InnerDecklistNode *>(listRoot->at(i));
        if (!currentZone)
            continue;
        for (int j = 0; j < currentZone->size(); j++) {
            DecklistCardNode *currentCard = dynamic_cast<DecklistCardNode *>(currentZone->at(j));
            if (!currentCard)
                continue;
            createNameFilter(currentCard->getName());
        }
    }
    updateFilterModel();
}

void VisualDatabaseDisplayNameFilterWidget::createNameFilter(const QString &name)
{
    if (activeFilters.contains(name))
        return;

    // Create a button for the filter
    auto *button = new QPushButton(name, flowWidget);
    button->setStyleSheet("QPushButton { background-color: lightgray; border: 1px solid gray; padding: 5px; }"
                          "QPushButton:hover { background-color: red; color: white; }");

    connect(button, &QPushButton::clicked, this, [this, name]() { removeNameFilter(name); });

    flowWidget->addWidget(button);
    activeFilters[name] = button;
}

void VisualDatabaseDisplayNameFilterWidget::removeNameFilter(const QString &name)
{
    if (activeFilters.contains(name)) {
        activeFilters[name]->deleteLater(); // Safe deletion
        activeFilters.remove(name);

        QTimer::singleShot(0, this,
                           &VisualDatabaseDisplayNameFilterWidget::updateFilterModel); // Avoid concurrent modification
    }
}

void VisualDatabaseDisplayNameFilterWidget::updateFilterModel()
{
    // Clear existing name filters
    emit filterModel->layoutAboutToBeChanged();
    filterModel->clearFiltersOfType(CardFilter::Attr::AttrName);

    filterModel->blockSignals(true);
    filterModel->filterTree()->blockSignals(true);

    for (const auto &name : activeFilters.keys()) {
        QString nameString = name;
        filterModel->addFilter(new CardFilter(nameString, CardFilter::Type::TypeOr, CardFilter::Attr::AttrName));
    }

    filterModel->blockSignals(false);
    filterModel->filterTree()->blockSignals(false);

    filterModel->blockSignals(false);
    filterModel->filterTree()->blockSignals(false);

    emit filterModel->filterTree()->changed();
    emit filterModel->layoutChanged();
}

void VisualDatabaseDisplayNameFilterWidget::syncWithFilterModel()
{
    QStringList currentFilters;
    for (const auto &filter : filterModel->getFiltersOfType(CardFilter::Attr::AttrName)) {
        if (filter->type() == CardFilter::Type::TypeOr) {
            currentFilters.append(filter->term());
        }
    }

    QStringList activeFilterList = activeFilters.keys();

    // Sort lists for efficient comparison
    std::sort(currentFilters.begin(), currentFilters.end());
    std::sort(activeFilterList.begin(), activeFilterList.end());

    if (currentFilters == activeFilterList) {
        return;
    }

    // Remove filters that are in the UI but not in the model
    for (const auto &name : activeFilterList) {
        if (!currentFilters.contains(name)) {
            removeNameFilter(name);
        }
    }

    // Add filters that are in the model but not in the UI
    for (const auto &name : currentFilters) {
        if (!activeFilters.contains(name)) {
            createNameFilter(name);
        }
    }
}
