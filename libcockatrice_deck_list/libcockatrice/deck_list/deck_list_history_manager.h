#ifndef COCKATRICE_DECK_LIST_HISTORY_MANAGER_H
#define COCKATRICE_DECK_LIST_HISTORY_MANAGER_H

#include "deck_list.h"
#include "deck_list_memento.h"

#include <QObject>
#include <QSharedPointer>
#include <QStack>

class DeckListHistoryManager : public QObject
{
    Q_OBJECT

signals:
    void undoRedoStateChanged();

public:
    explicit DeckListHistoryManager(QObject *parent = nullptr) : QObject(parent)
    {
    }

    void save(DeckListMemento *memento)
    {
        undoStack.push(memento);
        redoStack.clear();
        emit undoRedoStateChanged();
    }

    bool canUndo() const
    {
        return !undoStack.isEmpty();
    }
    bool canRedo() const
    {
        return !redoStack.isEmpty();
    }

    DeckListMemento *undo(DeckList *deck)
    {
        if (undoStack.isEmpty()) {
            return nullptr;
        }

        // Peek at the memento we are going to restore
        DeckListMemento *mementoToRestore = undoStack.top();

        // Save current state for redo using the same reason as the memento we're restoring
        DeckListMemento *currentState = deck->createMemento(mementoToRestore->getReason());
        redoStack.push(currentState);

        // Pop the last state from undo stack and restore it
        DeckListMemento *memento = undoStack.pop();
        deck->restoreMemento(memento);

        emit undoRedoStateChanged();
        return memento;
    }

    DeckListMemento *redo(DeckList *deck)
    {
        if (redoStack.isEmpty()) {
            return nullptr;
        }

        // Peek at the memento we are going to restore
        DeckListMemento *mementoToRestore = redoStack.top();

        // Save current state for undo using the same reason as the memento we're restoring
        DeckListMemento *currentState = deck->createMemento(mementoToRestore->getReason());
        undoStack.push(currentState);

        // Pop the next state from redo stack and restore it
        DeckListMemento *memento = redoStack.pop();
        deck->restoreMemento(memento);

        emit undoRedoStateChanged();
        return memento;
    }

    QStack<DeckListMemento *> getRedoStack() const
    {
        return redoStack;
    }

    QStack<DeckListMemento *> getUndoStack() const
    {
        return undoStack;
    }

private:
    QStack<DeckListMemento *> undoStack;
    QStack<DeckListMemento *> redoStack;
};

#endif // COCKATRICE_DECK_LIST_HISTORY_MANAGER_H
