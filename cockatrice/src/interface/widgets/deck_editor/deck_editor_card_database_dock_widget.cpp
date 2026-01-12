#include "deck_editor_card_database_dock_widget.h"

DeckEditorCardDatabaseDockWidget::DeckEditorCardDatabaseDockWidget(AbstractTabDeckEditor *parent) : QDockWidget(parent)
{
    setObjectName("databaseDisplayDock");
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);

    createDatabaseDisplayDock(parent);

    retranslateUi();
}

void DeckEditorCardDatabaseDockWidget::createDatabaseDisplayDock(AbstractTabDeckEditor *deckEditor)
{
    databaseDisplayWidget = new DeckEditorDatabaseDisplayWidget(this, deckEditor);

    auto *frame = new QVBoxLayout;
    frame->setObjectName("databaseDisplayFrame");
    frame->addWidget(databaseDisplayWidget);

    auto *dockContents = new QWidget();
    dockContents->setObjectName("databaseDisplayDockContents");
    dockContents->setLayout(frame);
    setWidget(dockContents);

    installEventFilter(deckEditor);
    connect(this, &QDockWidget::topLevelChanged, deckEditor, &AbstractTabDeckEditor::dockTopLevelChanged);

    // connect signals
    connect(databaseDisplayWidget, &DeckEditorDatabaseDisplayWidget::cardChanged, deckEditor,
            &AbstractTabDeckEditor::updateCard);
    connect(databaseDisplayWidget, &DeckEditorDatabaseDisplayWidget::addCardToMainDeck, deckEditor,
            &AbstractTabDeckEditor::actAddCard);
    connect(databaseDisplayWidget, &DeckEditorDatabaseDisplayWidget::addCardToSideboard, deckEditor,
            &AbstractTabDeckEditor::actAddCardToSideboard);
    connect(databaseDisplayWidget, &DeckEditorDatabaseDisplayWidget::decrementCardFromMainDeck, deckEditor,
            &AbstractTabDeckEditor::actDecrementCard);
    connect(databaseDisplayWidget, &DeckEditorDatabaseDisplayWidget::decrementCardFromSideboard, deckEditor,
            &AbstractTabDeckEditor::actDecrementCardFromSideboard);
}

CardDatabase *DeckEditorCardDatabaseDockWidget::getDatabase() const
{
    return databaseDisplayWidget->databaseModel->getDatabase();
}

void DeckEditorCardDatabaseDockWidget::retranslateUi()
{
    setWindowTitle(tr("Card Database"));
}

void DeckEditorCardDatabaseDockWidget::setFilterTree(FilterTree *filterTree)
{
    databaseDisplayWidget->setFilterTree(filterTree);
}

void DeckEditorCardDatabaseDockWidget::clearAllDatabaseFilters()
{
    databaseDisplayWidget->clearAllDatabaseFilters();
}
void DeckEditorCardDatabaseDockWidget::highlightAllSearchEdit()
{
    databaseDisplayWidget->searchEdit->setSelection(0, databaseDisplayWidget->searchEdit->text().length());
}
