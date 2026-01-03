#include "deck_list_history_manager_widget.h"

#include "deck_state_manager.h"

DeckListHistoryManagerWidget::DeckListHistoryManagerWidget(DeckStateManager *_deckStateManager,
                                                           DeckListStyleProxy *_styleProxy,
                                                           QWidget *parent)
    : QWidget(parent), deckStateManager(_deckStateManager), styleProxy(_styleProxy)
{
    layout = new QHBoxLayout(this);

    aUndo = new QAction(QString(), this);
    aUndo->setIcon(QPixmap("theme:icons/arrow_undo"));
    aUndo->setShortcut(QKeySequence::Undo);
    aUndo->setShortcutContext(Qt::ApplicationShortcut);
    connect(aUndo, &QAction::triggered, this, &DeckListHistoryManagerWidget::doUndo);

    undoButton = new QToolButton(this);
    undoButton->setDefaultAction(aUndo);

    aRedo = new QAction(QString(), this);
    aRedo->setIcon(QPixmap("theme:icons/arrow_redo"));
    aRedo->setShortcut(QKeySequence::Redo);
    aRedo->setShortcutContext(Qt::ApplicationShortcut);
    connect(aRedo, &QAction::triggered, this, &DeckListHistoryManagerWidget::doRedo);

    redoButton = new QToolButton(this);
    redoButton->setDefaultAction(aRedo);

    layout->addWidget(undoButton);
    layout->addWidget(redoButton);

    historyButton = new SettingsButtonWidget(this);
    historyButton->setButtonIcon(QPixmap("theme:icons/arrow_history"));

    historyLabel = new QLabel(this);

    historyList = new QListWidget(this);

    historyButton->addSettingsWidget(historyLabel);
    historyButton->addSettingsWidget(historyList);

    layout->addWidget(historyButton);

    connect(historyList, &QListWidget::itemClicked, this, &DeckListHistoryManagerWidget::onListClicked);

    connect(deckStateManager, &DeckStateManager::historyChanged, this, &DeckListHistoryManagerWidget::refreshList);

    refreshList();
    retranslateUi();
}

void DeckListHistoryManagerWidget::retranslateUi()
{
    undoButton->setToolTip(tr("Undo"));
    redoButton->setToolTip(tr("Redo"));
    historyButton->setToolTip(tr("Undo/Redo history"));
    historyLabel->setText(tr("Click on an entry to revert to that point in the history."));
}

void DeckListHistoryManagerWidget::refreshList()
{
    historyList->clear();

    DeckListHistoryManager *historyManager = deckStateManager->getHistoryManager();

    // Fill redo section first (oldest redo at top, newest redo closest to divider)
    const auto redoStack = historyManager->getRedoStack();
    for (int i = 0; i < redoStack.size(); ++i) { // iterate forward
        auto item = new QListWidgetItem(tr("[redo] ") + redoStack[i].getReason(), historyList);
        item->setData(Qt::UserRole, QVariant("redo"));
        item->setData(Qt::UserRole + 1, i); // index in redo stack
        item->setForeground(Qt::gray);
        historyList->addItem(item);
    }

    // Divider
    if (!historyManager->getUndoStack().isEmpty() && !historyManager->getRedoStack().isEmpty()) {
        auto divider = new QListWidgetItem("──────────", historyList);
        divider->setFlags(Qt::NoItemFlags); // not selectable
        historyList->addItem(divider);
    }

    // Fill undo section
    const auto undoStack = historyManager->getUndoStack();
    for (int i = undoStack.size() - 1; i >= 0; --i) {
        auto item = new QListWidgetItem(tr("[undo] ") + undoStack[i].getReason(), historyList);
        item->setData(Qt::UserRole, QVariant("undo"));
        item->setData(Qt::UserRole + 1, i); // index in undo stack
        historyList->addItem(item);
    }

    // Button enabled states
    undoButton->setEnabled(historyManager->canUndo());
    redoButton->setEnabled(historyManager->canRedo());
}

void DeckListHistoryManagerWidget::onListClicked(const QListWidgetItem *item)
{
    // Ignore non-selectable items (like divider)
    if (!(item->flags() & Qt::ItemIsSelectable)) {
        return;
    }

    const QString mode = item->data(Qt::UserRole).toString();
    int index = item->data(Qt::UserRole + 1).toInt();

    if (mode == "redo") {
        const auto redoStack = deckStateManager->getHistoryManager()->getRedoStack();
        int steps = redoStack.size() - index;
        deckStateManager->redo(steps);
    } else if (mode == "undo") {
        const auto undoStack = deckStateManager->getHistoryManager()->getUndoStack();
        int steps = undoStack.size() - index;
        deckStateManager->undo(steps);
    }

    refreshList();
}

void DeckListHistoryManagerWidget::doUndo()
{
    deckStateManager->undo();
}

void DeckListHistoryManagerWidget::doRedo()
{
    deckStateManager->redo();
}