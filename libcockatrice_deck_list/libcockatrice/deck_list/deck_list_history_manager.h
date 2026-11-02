#ifndef COCKATRICE_DECK_LIST_HISTORY_MANAGER_H
#define COCKATRICE_DECK_LIST_HISTORY_MANAGER_H

#include "deck_list_memento.h"

#include <QObject>
#include <QStack>
#include <qtmetamacros.h>
#include <optional>

class DeckList;

class DeckListHistoryManager : public QObject
{
    Q_OBJECT

signals:
    void undoRedoStateChanged();

public:
    explicit DeckListHistoryManager(QObject *parent = nullptr) : QObject(parent)
    {
    }

    // A saved snapshot belongs to one loaded document, even when another document
    // later has identical contents. Undo/redo never changes this identity.
    struct SavePoint
    {
        quint64 documentId;
        QString contents;
    };

    [[nodiscard]] SavePoint createSavePoint(const DeckList &deck) const;
    bool markSaved(const SavePoint &point);
    void invalidateSavedState();
    [[nodiscard]] bool isModified(const DeckList &deck) const;

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
    /**
     * @brief Moves one state from @p source to @p target, applying it to @p deck.
     *
     * Used by both undo (undoStack -> redoStack) and redo (redoStack -> undoStack).
     */
    void restoreAndSwap(QStack<DeckListMemento> &source, QStack<DeckListMemento> &target, DeckList *deck);

    quint64 documentId = 0;
    std::optional<QString> savedContents;
    QStack<DeckListMemento> undoStack;
    QStack<DeckListMemento> redoStack;
};

#endif // COCKATRICE_DECK_LIST_HISTORY_MANAGER_H
