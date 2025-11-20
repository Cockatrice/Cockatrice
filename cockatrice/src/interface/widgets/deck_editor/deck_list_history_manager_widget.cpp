#include "deck_list_history_manager_widget.h"

DeckListHistoryManagerWidget::DeckListHistoryManagerWidget(DeckListModel *_deckListModel,
                                                           DeckListStyleProxy *_styleProxy,
                                                           DeckListHistoryManager *manager,
                                                           QWidget *parent)
    : QWidget(parent), deckListModel(_deckListModel), styleProxy(_styleProxy), historyManager(manager)
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

    connect(historyManager, &DeckListHistoryManager::undoRedoStateChanged, this,
            &DeckListHistoryManagerWidget::refreshList);

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

void DeckListHistoryManagerWidget::setDeckListModel(DeckListModel *_deckListModel)
{
    deckListModel = _deckListModel;
}

void DeckListHistoryManagerWidget::refreshList()
{
    historyList->clear();

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

void DeckListHistoryManagerWidget::doUndo()
{
    if (!historyManager->canUndo()) {
        return;
    }

    historyManager->undo(deckListModel->getDeckList());
    deckListModel->rebuildTree();
    emit deckListModel->layoutChanged();
    emit requestDisplayWidgetSync();

    refreshList();
}

void DeckListHistoryManagerWidget::doRedo()
{
    if (!historyManager->canRedo()) {
        return;
    }

    historyManager->redo(deckListModel->getDeckList());
    deckListModel->rebuildTree();

    emit deckListModel->layoutChanged();
    emit requestDisplayWidgetSync();

    refreshList();
}

void DeckListHistoryManagerWidget::onListClicked(QListWidgetItem *item)
{
    // Ignore non-selectable items (like divider)
    if (!(item->flags() & Qt::ItemIsSelectable)) {
        return;
    }

    const QString mode = item->data(Qt::UserRole).toString();
    int index = item->data(Qt::UserRole + 1).toInt();

    if (mode == "redo") {
        const auto redoStack = historyManager->getRedoStack();
        int steps = redoStack.size() - index;
        for (int i = 0; i < steps; ++i) {
            historyManager->redo(deckListModel->getDeckList());
        }
    } else if (mode == "undo") {
        const auto undoStack = historyManager->getUndoStack();
        int steps = undoStack.size() - 1 - index;
        for (int i = 0; i < steps + 1; ++i) {
            historyManager->undo(deckListModel->getDeckList());
        }
    }

    deckListModel->rebuildTree();

    emit deckListModel->layoutChanged();
    emit requestDisplayWidgetSync();

    refreshList();
}