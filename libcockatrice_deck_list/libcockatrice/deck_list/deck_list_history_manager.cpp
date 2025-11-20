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

void DeckListHistoryManager::undo(DeckList *deck)
{
    if (undoStack.isEmpty())
        return;

    // Peek at the memento we are going to restore
    const DeckListMemento &mementoToRestore = undoStack.top();

    // Save current state for redo
    DeckListMemento currentState = deck->createMemento(mementoToRestore.getReason());
    redoStack.push(currentState);

    // Pop the last state from undo stack and restore it
    DeckListMemento memento = undoStack.pop();
    deck->restoreMemento(memento);

    emit undoRedoStateChanged();
}

void DeckListHistoryManager::redo(DeckList *deck)
{
    if (redoStack.isEmpty())
        return;

    // Peek at the memento we are going to restore
    const DeckListMemento &mementoToRestore = redoStack.top();

    // Save current state for undo
    DeckListMemento currentState = deck->createMemento(mementoToRestore.getReason());
    undoStack.push(currentState);

    // Pop the next state from redo stack and restore it
    DeckListMemento memento = redoStack.pop();
    deck->restoreMemento(memento);

    emit undoRedoStateChanged();
}
