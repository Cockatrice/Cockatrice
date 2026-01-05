#include "deck_state_manager.h"

#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/deck_list/deck_list_history_manager.h>

DeckStateManager::DeckStateManager(QObject *parent)
    : QObject(parent), deckList(QSharedPointer<DeckList>(new DeckList)),
      deckListModel(new DeckListModel(this, deckList)), historyManager(new DeckListHistoryManager(this))
{
    connect(historyManager, &DeckListHistoryManager::undoRedoStateChanged, this, [this] {
        setModified(true);
        emit historyChanged();
    });
    connect(deckListModel, &DeckListModel::rowsInserted, this, &DeckStateManager::uniqueCardsChanged);
    connect(deckListModel, &DeckListModel::rowsRemoved, this, &DeckStateManager::uniqueCardsChanged);
}

const DeckList &DeckStateManager::getDeckList() const
{
    return *deckList.get();
}

LoadedDeck DeckStateManager::toLoadedDeck() const
{
    return {getDeckList(), lastLoadInfo};
}

DeckList::Metadata const &DeckStateManager::getMetadata() const
{
    return deckList->getMetadata();
}

QString DeckStateManager::getSimpleDeckName() const
{
    return deckList->getMetadata().name.simplified();
}

QString DeckStateManager::getDeckHash() const
{
    return deckList->getDeckHash();
}

bool DeckStateManager::isModified() const
{
    return modified;
}

void DeckStateManager::setModified(bool state)
{
    if (state == modified) {
        return;
    }

    modified = state;
    emit isModifiedChanged(modified);
}

bool DeckStateManager::isBlankNewDeck() const
{
    return !isModified() && deckList->isBlankDeck();
}

void DeckStateManager::replaceDeck(const LoadedDeck &deck)
{
    lastLoadInfo = deck.lastLoadInfo;
    deckList = QSharedPointer<DeckList>(new DeckList(deck.deckList));
    deckListModel->setDeckList(deckList);

    historyManager->clear();

    setModified(false);
    emit deckReplaced();
}

void DeckStateManager::clearDeck()
{
    replaceDeck(LoadedDeck());
}

bool DeckStateManager::modifyDeck(const QString &reason, const std::function<bool(DeckListModel *)> &operation)
{
    DeckListMemento memento = deckList->createMemento(reason);
    bool success = operation(deckListModel);

    if (success) {
        historyManager->save(memento);
        doCardModified();
    }

    return success;
}

QModelIndex DeckStateManager::modifyDeck(const QString &reason,
                                         const std::function<QModelIndex(DeckListModel *)> &operation)
{
    DeckListMemento memento = deckList->createMemento(reason);
    QModelIndex idx = operation(deckListModel);

    if (idx.isValid()) {
        historyManager->save(memento);
        doCardModified();
    }

    return idx;
}

void DeckStateManager::setName(const QString &name)
{
    QString previous = deckList->getName();
    if (previous == name) {
        return;
    }

    requestHistorySave(tr("Rename deck to \"%1\" from \"%2\"").arg(name).arg(previous));
    deckList->setName(name);

    doMetadataModified();
}

void DeckStateManager::setComments(const QString &comments)
{
    QString previous = deckList->getComments();
    if (previous == comments) {
        return;
    }

    requestHistorySave(tr("Updated comments (was %1 chars, now %2 chars)").arg(previous.size()).arg(comments.size()));
    deckList->setComments(comments);

    doMetadataModified();
}

void DeckStateManager::setBannerCard(const CardRef &bannerCard)
{
    CardRef previous = deckList->getBannerCard();
    if (previous == bannerCard) {
        return;
    }

    requestHistorySave(tr("Set banner card to %1 (%2)").arg(bannerCard.name).arg(bannerCard.providerId));
    deckList->setBannerCard(bannerCard);

    doMetadataModified();
}

void DeckStateManager::setTags(const QStringList &tags)
{
    QStringList previous = deckList->getTags();
    if (previous == tags) {
        return;
    }

    requestHistorySave(tr("Tags changed"));
    deckList->setTags(tags);

    doMetadataModified();
}

void DeckStateManager::setFormat(const QString &format)
{
    if (deckList->getMetadata().gameFormat == format) {
        return;
    }

    requestHistorySave(tr("Set format to %1").arg(format));
    deckListModel->setActiveFormat(format);

    doMetadataModified();
}

QModelIndex DeckStateManager::addCard(const ExactCard &card, const QString &zoneName)
{
    if (!card) {
        return {};
    }

    QString zone = card.getInfo().getIsToken() ? DECK_ZONE_TOKENS : zoneName;

    QString reason = tr("Added (%1): %2 (%3) %4")
                         .arg(zone, card.getName(), card.getPrinting().getSet()->getCorrectedShortName(),
                              card.getPrinting().getProperty("num"));

    QModelIndex idx = modifyDeck(reason, [&card, &zone](auto model) { return model->addCard(card, zone); });

    if (idx.isValid()) {
        emit focusIndexChanged(idx);
    }

    return idx;
}

QModelIndex DeckStateManager::decrementCard(const ExactCard &card, const QString &zoneName)
{
    if (!card)
        return {};

    QString providerId = card.getPrinting().getUuid();
    QString collectorNumber = card.getPrinting().getProperty("num");

    QModelIndex idx = deckListModel->findCard(card.getName(), zoneName, providerId, collectorNumber);
    if (!idx.isValid()) {
        return {};
    }

    bool success = offsetCountAtIndex(idx, false);

    if (!success) {
        return {};
    }

    if (idx.isValid()) {
        emit focusIndexChanged(idx);
    }

    return idx;
}

static bool doSwapCard(DeckListModel *model,
                       const QModelIndex &idx,
                       const QString &cardName,
                       const QString &providerId,
                       const QString &otherZone)
{
    bool success = model->offsetCountAtIndex(idx, -1);
    if (!success) {
        return false;
    }

    if (ExactCard card = CardDatabaseManager::query()->getCard({cardName, providerId})) {
        model->addCard(card, otherZone);
    } else {
        // Third argument (true) says create the card no matter what, even if not in DB
        model->addPreferredPrintingCard(cardName, otherZone, true);
    }

    return true;
}

bool DeckStateManager::swapCardAtIndex(const QModelIndex &idx)
{
    if (!idx.isValid())
        return false;

    QString cardName = idx.siblingAtColumn(DeckListModelColumns::CARD_NAME).data().toString();
    QString providerId = idx.siblingAtColumn(DeckListModelColumns::CARD_PROVIDER_ID).data().toString();
    QModelIndex gparent = idx.parent().parent();

    if (!gparent.isValid())
        return false;

    QString zoneName = gparent.siblingAtColumn(DeckListModelColumns::CARD_NAME).data(Qt::EditRole).toString();
    QString otherZoneName = zoneName == DECK_ZONE_MAIN ? DECK_ZONE_SIDE : DECK_ZONE_MAIN;

    QString reason = tr("Moved to %1 1 × \"%2\" (%3)") //
                         .arg(otherZoneName)
                         .arg(cardName)
                         .arg(providerId);

    return modifyDeck(reason, [&idx, &cardName, &providerId, &otherZoneName](auto model) {
        return doSwapCard(model, idx, cardName, providerId, otherZoneName);
    });
}

bool DeckStateManager::removeCardAtIndex(const QModelIndex &idx)
{
    if (!idx.isValid() || deckListModel->hasChildren(idx)) {
        return false;
    }

    QString cardName = idx.siblingAtColumn(DeckListModelColumns::CARD_NAME).data().toString();

    QString reason = tr("Removed \"%1\" (all copies)").arg(cardName);

    return modifyDeck(reason, [&idx](auto model) { return model->removeRow(idx.row(), idx.parent()); });
}

bool DeckStateManager::incrementCountAtIndex(const QModelIndex &idx)
{
    return offsetCountAtIndex(idx, 1);
}

bool DeckStateManager::decrementCountAtIndex(const QModelIndex &idx)
{
    return offsetCountAtIndex(idx, -1);
}

bool DeckStateManager::offsetCountAtIndex(const QModelIndex &idx, int offset)
{
    if (!idx.isValid()) {
        return false;
    }

    QString cardName = idx.siblingAtColumn(DeckListModelColumns::CARD_NAME).data(Qt::EditRole).toString();
    QString providerId = idx.siblingAtColumn(DeckListModelColumns::CARD_PROVIDER_ID).data(Qt::DisplayRole).toString();

    QString reason = tr("%1 1 × \"%2\" (%3)") //
                         .arg(offset > 0 ? tr("Added") : tr("Removed"))
                         .arg(cardName)
                         .arg(providerId);

    return modifyDeck(reason, [&idx, &offset](auto model) { return model->offsetCountAtIndex(idx, offset); });
}

void DeckStateManager::undo(int steps)
{
    if (!historyManager->canUndo()) {
        return;
    }

    for (int i = 0; i < steps; i++) {
        if (!historyManager->canUndo()) {
            continue;
        }
        historyManager->undo(deckList.get());
    }

    deckListModel->rebuildTree();

    emit deckListModel->layoutChanged();
}

void DeckStateManager::redo(int steps)
{
    if (!historyManager->canRedo()) {
        return;
    }

    for (int i = 0; i < steps; i++) {
        if (!historyManager->canRedo()) {
            continue;
        }
        historyManager->redo(deckList.get());
    }

    deckListModel->rebuildTree();

    emit deckListModel->layoutChanged();
}

void DeckStateManager::requestHistorySave(const QString &reason)
{
    historyManager->save(deckList->createMemento(reason));
}

/**
 * @brief Handles updating state and emitting signals whenever the cards are modified
 */
void DeckStateManager::doCardModified()
{
    setModified(true);
    emit cardModified();
    emit deckModified();
}

/**
 * @brief Handles updating state and emitting signals whenever the metadata is modified
 */
void DeckStateManager::doMetadataModified()
{
    setModified(true);
    emit metadataModified();
    emit deckModified();
}
