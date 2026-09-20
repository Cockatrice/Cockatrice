#include "deck_list_history_manager.h"

void DeckListHistoryManager::save(const DeckListMemento &memento)
{
    undoStack.push(memento);
    redoStack.clear();
    emit undoRedoStateChanged();
}

void DeckListHistoryManager::clear()
{
    undoStack.clear();
    redoStack.clear();
    emit undoRedoStateChanged();
}

void DeckListHistoryManager::restoreAndSwap(QStack<DeckListMemento> &source,
                                            QStack<DeckListMemento> &target,
                                            DeckList *deck)
{
    if (source.isEmpty()) {
        return;
    }

    // The reason is read before the source is popped.
    const QString reason = source.top().getReason();

    // Save the current state so the opposite direction can return to it.
    target.push(deck->createMemento(reason));

    // Apply the state we are moving to.
    deck->restoreMemento(source.pop());

    emit undoRedoStateChanged();
}

void DeckListHistoryManager::undo(DeckList *deck)
{
    restoreAndSwap(undoStack, redoStack, deck);
}

void DeckListHistoryManager::redo(DeckList *deck)
{
    restoreAndSwap(redoStack, undoStack, deck);
}
