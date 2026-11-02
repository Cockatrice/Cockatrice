#include "deck_list_history_manager.h"

#include "libcockatrice/deck_list/deck_list.h"
#include "libcockatrice/deck_list/deck_list_memento.h"

#include <QString>

DeckListHistoryManager::SavePoint DeckListHistoryManager::createSavePoint(const DeckList &deck) const
{
    return {documentId, deck.writeToString_Native()};
}

bool DeckListHistoryManager::markSaved(const SavePoint &point)
{
    if (point.documentId != documentId) {
        return false;
    }
    savedContents = point.contents;
    return true;
}

void DeckListHistoryManager::invalidateSavedState()
{
    savedContents.reset();
}

bool DeckListHistoryManager::isModified(const DeckList &deck) const
{
    // The deck hash intentionally omits metadata, printings, and custom zones.
    // Only the complete native representation can describe a saved editor state.
    return !savedContents || *savedContents != deck.writeToString_Native();
}

void DeckListHistoryManager::save(const DeckListMemento &memento)
{
    undoStack.push(memento);
    redoStack.clear();
    emit undoRedoStateChanged();
}

void DeckListHistoryManager::clear()
{
    ++documentId;
    invalidateSavedState();
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
