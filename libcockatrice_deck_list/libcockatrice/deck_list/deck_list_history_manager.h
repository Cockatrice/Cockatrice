#ifndef COCKATRICE_DECK_LIST_HISTORY_MANAGER_H
#define COCKATRICE_DECK_LIST_HISTORY_MANAGER_H

#include "deck_list.h"
#include "deck_list_memento.h"

#include <QObject>
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

    void save(const DeckListMemento &memento);

    void clear();

    [[nodiscard]] bool canUndo() const
    {
        return !undoStack.isEmpty();
    }

    [[nodiscard]] bool canRedo() const
    {
        return !redoStack.isEmpty();
    }

    void undo(DeckList *deck);

    void redo(DeckList *deck);

    [[nodiscard]] QStack<DeckListMemento> getRedoStack() const
    {
        return redoStack;
    }
    [[nodiscard]] QStack<DeckListMemento> getUndoStack() const
    {
        return undoStack;
    }

private:
    QStack<DeckListMemento> undoStack;
    QStack<DeckListMemento> redoStack;
};

#endif // COCKATRICE_DECK_LIST_HISTORY_MANAGER_H
