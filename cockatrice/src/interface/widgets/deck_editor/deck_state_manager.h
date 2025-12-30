#ifndef COCKATRICE_DECK_STATE_MANAGER_H
#define COCKATRICE_DECK_STATE_MANAGER_H

#include "../../deck_loader/loaded_deck.h"
#include "deck_list_model.h"

#include <QSharedPointer>
#include <libcockatrice/deck_list/deck_list.h>

class DeckListHistoryManager;

/**
 * @brief This class centralizes the management of the state of the deck in the deck editor tab.
 * It is responsible for owning and managing the DeckListModel, underlying DeckList, load info, and edit history.
 *
 * Although this class provides getters for the underlying DeckListModel, you should generally refrain from directly
 * modifying the returned model. Outside modifications to the deck state should be done through @link
 * DeckStateManager::modifyDeck and the metadata setters.
 * Those methods ensure that the history is recorded and correct signals are emitted.
 */
class DeckStateManager : public QObject
{
    Q_OBJECT

    LoadedDeck::LoadInfo lastLoadInfo;
    QSharedPointer<DeckList> deckList;
    DeckListModel *deckListModel;
    DeckListHistoryManager *historyManager;

    bool modified = false;

public:
    explicit DeckStateManager(QObject *parent = nullptr);

    /**
     * Gets the underlying HistoryManager.
     * @return The DeckListHistoryManager instance
     */
    DeckListHistoryManager *getHistoryManager() const
    {
        return historyManager;
    }

    /**
     * @brief Gets the underlying DeckListModel.
     * You should generally refrain modifying the returned model directly.
     * However, it's fine (and intended) to perform queries on the returned model.
     * @return The DeckListModel instance
     */
    DeckListModel *getModel() const
    {
        return deckListModel;
    }

    /**
     * @brief Gets a view of the current deck.
     */
    const DeckList &getDeckList() const;

    /**
     * @brief Creates a LoadedDeck containing the contents of the current deck and the current LoadInfo.
     *
     * @return A new LoadedDeck instance.
     */
    LoadedDeck toLoadedDeck() const;

    /**
     * @brief Gets a view of the metadata in the DeckList
     */
    DeckList::Metadata const &getMetadata() const;

    /**
     * @brief Gets the deck's simplified name.
     */
    QString getSimpleDeckName() const;

    /**
     * @brief Gets the deck hash.
     */
    QString getDeckHash() const;

    /**
     * @brief Checks if the deck has been modified since it was last saved
     */
    bool isModified() const;

    /**
     * @brief Sets the new isModified state, emitting a signal if the state changed.
     * This class will automatically update its isModified state, but you may need to set it manually to handle, for
     * example, saving.
     * @param state The state
     */
    void setModified(bool state);

    /**
     * @brief Checks if the deck state is as if it was a new deck
     */
    bool isBlankNewDeck() const;

    /**
     * @brief Overwrites the current deck with a new deck, resetting all history
     * @param deck The new deck.
     */
    void replaceDeck(const LoadedDeck &deck);

    /**
     * @brief Resets the deck to a blank new deck, resetting all history.
     */
    void clearDeck();

    /**
     * @brief Sets the lastLoadInfo.
     * @param loadInfo The lastLoadInfo
     */
    void setLastLoadInfo(const LoadedDeck::LoadInfo &loadInfo)
    {
        lastLoadInfo = loadInfo;
    }

    /**
     * @brief Modifies the cards in the deck, in a wrapped operation that is saved to the history.
     *
     * The operation is a function that accepts a DeckListModel that it operates upon, and returns a bool.
     *
     * This method will pass the underlying DeckListModel into the operation function. The function can call methods on
     * the model to modify the deck.
     * The function should return a bool to indicate success/failure.
     *
     * If the operation returns true, the state of the deck before the operation is ran is saved to the history, and the
     * isModified state is updated.
     * If the operation returns false, the history and isModified state is not updated.
     *
     * Note that even if the operation fails, any modifications to the model will already have been made.
     * It's recommended for the operation to always return true if any modification has already been made to the model,
     * as not doing that may cause the state to become desynced.
     *
     * @param reason The reason to display in the history
     * @param operation The modification operation.
     * @return The bool returned from the operation
     */
    bool modifyDeck(const QString &reason, const std::function<bool(DeckListModel *)> &operation);

    /**
     * @brief Modifies the cards in the deck, in a wrapped operation that is saved to the history.
     *
     * The operation is a function that accepts a DeckListModel that it operates upon, and returns a QModelIndex.
     * If the index is invalid, then the operation is considered to be a failure.
     *
     * See the other @link DeckStateManager::modifyDeck for more info about the behavior of this method.
     *
     * @param reason The reason to display in the history
     * @param operation The modification operation.
     * @return The QModelIndex returned from the operation
     */
    QModelIndex modifyDeck(const QString &reason, const std::function<QModelIndex(DeckListModel *)> &operation);

    /// @name Metadata setters
    /// @brief These methods set the metadata. Will no-op if the new value is the same as the current value.
    /// Saves the operation to history if successful.
    ///@{
    void setName(const QString &name);
    void setComments(const QString &comments);
    void setBannerCard(const CardRef &bannerCard);
    void setTags(const QStringList &tags);
    void setFormat(const QString &format);
    ///@}

    /**
     * @brief Adds the given card to the given zone.
     * Saves the operation to history if successful.
     *
     * @param card The card to add
     * @param zoneName The zone to add the card to
     * @return The index of the added card
     */
    QModelIndex addCard(const ExactCard &card, const QString &zoneName);

    /**
     * @brief Removes 1 copy of the given card from the given zone.
     * Saves the operation to history if successful.
     *
     * @param card The card to remove
     * @param zoneName The zone to remove the card from
     * @return The index of the removed card. Will be invalid if the last copy was removed.
     */
    QModelIndex decrementCard(const ExactCard &card, const QString &zoneName);

    /**
     * @brief Swaps one copy of the card at the given index between the maindeck and sideboard.
     * No-ops if index is invalid or not a card node.
     * Saves the operation to history if successful.
     *
     * @param idx The model index
     * @return Whether the operation was successfully performed
     */
    bool swapCardAtIndex(const QModelIndex &idx);

    /**
     * @brief Removes all copies of the card at the given index.
     * No-ops if index is invalid or not a card node.
     * Saves the operation to history if successful.
     *
     * @param idx The model index
     * @return Whether the operation was successfully performed
     */
    bool removeCardAtIndex(const QModelIndex &idx);

    /**
     * @brief Increments the number of copies of the card at the given index by 1.
     * No-ops if index is invalid or not a card node.
     * Saves the operation to history if successful.
     *
     * @param idx The model index
     * @return Whether the operation was successfully performed
     */
    bool incrementCountAtIndex(const QModelIndex &idx);

    /**
     * @brief Decrements the number of copies of the card at the given index by 1.
     * No-ops if index is invalid or not a card node.
     * Saves the operation to history if successful.
     *
     * @param idx The model index
     * @return Whether the operation was successfully performed
     */
    bool decrementCountAtIndex(const QModelIndex &idx);

    /**
     * Undoes n steps of the history, setting the decklist state and updating the current step in the historyManager.
     * @param steps Number of steps to undo.
     */
    void undo(int steps = 1);

    /**
     * Redoes n steps of the history, setting the decklist state and updating the current step in the historyManager.
     * @param steps Number of steps to redo.
     */
    void redo(int steps = 1);

public slots:
    /**
     * Saves the current decklist state to history.
     * @param reason The reason that is shown in the history.
     */
    void requestHistorySave(const QString &reason);

private:
    bool offsetCountAtIndex(const QModelIndex &idx, int offset);
    void doCardModified();
    void doMetadataModified();

signals:
    /**
     * A modification has been made to the cards in the deck
     */
    void cardModified();

    /**
     * A card that wasn't previously in the deck was added to the deck, or the last copy of a card was removed from the
     * deck.
     */
    void uniqueCardsChanged();

    /**
     * A modification has been made to the metadata in the deck
     */
    void metadataModified();

    /**
     * A modification has been made to the cards or metadata in the deck
     */
    void deckModified();

    /**
     * The history has been greatly changed and needs to be reloaded.
     */
    void historyChanged();

    /**
     * The deck has been completely changed.
     */
    void deckReplaced();

    /**
     * The isModified state of the deck has changed
     * @param isModified the new state
     */
    void isModifiedChanged(bool isModified);

    /**
     * The selected card on any views connected to this deck should be changed to this index.
     * @param index The model index
     */
    void focusIndexChanged(QModelIndex index);
};

#endif // COCKATRICE_DECK_STATE_MANAGER_H