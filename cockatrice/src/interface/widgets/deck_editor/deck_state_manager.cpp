#include "deck_state_manager.h"

#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/deck_list/deck_list_history_manager.h>
#include <libcockatrice/deck_list/tree/inner_deck_list_node.h>

DeckStateManager::DeckStateManager(QObject *parent)
    : QObject(parent), deckList(QSharedPointer<DeckList>(new DeckList)),
      deckListModel(new DeckListModel(this, deckList)), historyManager(new DeckListHistoryManager(this))
{
    connect(historyManager, &DeckListHistoryManager::undoRedoStateChanged, this, [this] {
        setModified(true);
        emit historyChanged();
    });
    connect(deckListModel, &DeckListModel::cardNodesChanged, this, &DeckStateManager::uniqueCardsChanged);
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

void DeckStateManager::setPlaymat(const PlaymatInfo &playmat)
{
    PlaymatInfo previous = deckList->getPlaymat();
    if (previous == playmat) {
        return;
    }

    requestHistorySave(tr("Set playmat to %1").arg(playmat.card.name));
    deckList->setPlaymat(playmat);

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

    CardSetPtr set = card.getPrinting().getSet();
    QString setName = set ? set->getCorrectedShortName() : "";

    QString reason =
        tr("Added (%1): %2 (%3) %4").arg(zone, card.getName(), setName, card.getPrinting().getProperty("num"));

    QModelIndex idx = modifyDeck(reason, [&card, &zone](auto model) { return model->addCard(card, zone); });

    if (idx.isValid()) {
        emit focusIndexChanged(idx, true);
    }

    return idx;
}

QModelIndex DeckStateManager::decrementCard(const ExactCard &card, const QString &zoneName)
{
    if (!card) {
        return {};
    }

    QString providerId = card.getPrinting().getUuid();
    QString collectorNumber = card.getPrinting().getProperty("num");

    QModelIndex idx = deckListModel->findCard(card.getName(), zoneName, providerId, collectorNumber);
    if (!idx.isValid()) {
        return {};
    }

    bool success = offsetCountAtIndex(idx, -1);

    if (!success) {
        return {};
    }

    // old index is no longer safe since rows could have been removed
    QModelIndex newIdx = deckListModel->findCard(card.getName(), zoneName, providerId, collectorNumber);
    if (newIdx.isValid()) {
        emit focusIndexChanged(newIdx, true);
    }

    return newIdx;
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
    if (!idx.isValid()) {
        return false;
    }

    QString cardName = idx.siblingAtColumn(DeckListModelColumns::CARD_NAME).data().toString();
    QString providerId = idx.siblingAtColumn(DeckListModelColumns::CARD_PROVIDER_ID).data().toString();
    QModelIndex gparent = idx.parent().parent();

    if (!gparent.isValid()) {
        return false;
    }

    QString zoneName = gparent.siblingAtColumn(DeckListModelColumns::CARD_NAME).data(Qt::EditRole).toString();
    // tokens have no swap target
    if (zoneName == DECK_ZONE_TOKENS) {
        return false;
    }
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

bool DeckStateManager::moveCardToZone(const QModelIndex &idx, const QString &targetZoneName)
{
    if (!idx.isValid()) {
        return false;
    }

    // Only actual card rows can be moved. Group or zone rows report an
    // aggregate amount and must never be deleted by this operation.
    if (!idx.data(DeckRoles::IsCardRole).toBool()) {
        return false;
    }

    QString cardName = idx.siblingAtColumn(DeckListModelColumns::CARD_NAME).data(Qt::EditRole).toString();
    QString providerId = idx.siblingAtColumn(DeckListModelColumns::CARD_PROVIDER_ID).data(Qt::DisplayRole).toString();
    int copies = idx.siblingAtColumn(DeckListModelColumns::CARD_AMOUNT).data(Qt::EditRole).toInt();

    if (copies <= 0) {
        return false;
    }

    // Tokens only live in the tokens zone and cannot be moved into decks.
    CardInfoPtr info = CardDatabaseManager::query()->getCardInfo(cardName);
    if (info && info->getIsToken()) {
        return false;
    }

    // Determine the zone the card currently lives in: the enclosing custom
    // zone, or the nearest top-level zone (board zone or legacy zone).
    QString currentZoneName;
    for (QModelIndex ancestor = idx.parent(); ancestor.isValid(); ancestor = ancestor.parent()) {
        bool isCustomZone = ancestor.data(DeckRoles::IsCustomZoneRole).toBool();
        if (isCustomZone || !ancestor.parent().isValid()) {
            currentZoneName = ancestor.siblingAtColumn(DeckListModelColumns::CARD_NAME).data(Qt::EditRole).toString();
            break;
        }
    }

    if (currentZoneName == targetZoneName) {
        return false;
    }

    QString reason = tr("Moved %1 × \"%2\" (%3) to %4")
                         .arg(copies)
                         .arg(cardName)
                         .arg(providerId)
                         .arg(InnerDecklistNode::visibleNameFromName(targetZoneName));

    return modifyDeck(reason, [&idx, &cardName, &providerId, &targetZoneName, copies](auto model) {
        if (!model->removeRow(idx.row(), idx.parent())) {
            return false;
        }

        if (ExactCard card = CardDatabaseManager::query()->getCard({cardName, providerId})) {
            for (int i = 0; i < copies; ++i) {
                model->addCard(card, targetZoneName);
            }
        } else {
            for (int i = 0; i < copies; ++i) {
                model->addPreferredPrintingCard(cardName, targetZoneName, true);
            }
        }

        return true;
    });
}

bool DeckStateManager::createCustomZone(const QString &boardZoneName, const QString &zoneName)
{
    const QString trimmedZoneName = zoneName.trimmed();
    if (trimmedZoneName.isEmpty()) {
        return false;
    }

    QString reason =
        tr("Created zone \"%1\" in %2").arg(trimmedZoneName, InnerDecklistNode::visibleNameFromName(boardZoneName));

    return modifyTree(reason, [&boardZoneName, &trimmedZoneName](DecklistNodeTree *tree) {
        return tree->addCustomZone(boardZoneName, trimmedZoneName) != nullptr;
    });
}

bool DeckStateManager::renameCustomZone(const QString &oldZoneName, const QString &newZoneName)
{
    const QString trimmedNewZoneName = newZoneName.trimmed();
    if (trimmedNewZoneName.isEmpty() || oldZoneName == trimmedNewZoneName) {
        return false;
    }

    QString reason = tr("Renamed zone \"%1\" to \"%2\"").arg(oldZoneName, trimmedNewZoneName);

    return modifyTree(reason, [&oldZoneName, &trimmedNewZoneName](DecklistNodeTree *tree) {
        return tree->renameCustomZone(oldZoneName, trimmedNewZoneName);
    });
}

bool DeckStateManager::moveCustomZone(const QString &zoneName, const QString &newBoardZoneName)
{
    const auto *tree = deckList->getTree();

    // Locate the zone through the tree's own lookup, which walks every top-level
    // zone (not just the standard boards) and covers the same-board no-op below.
    const auto *zone = tree->findCustomZoneByName(zoneName);
    if (!zone) {
        return false;
    }

    // Same-board moves are no-ops and must not pollute the history.
    const QString currentBoardName = zone->getParent() ? zone->getParent()->getName() : QString();
    if (currentBoardName == newBoardZoneName) {
        return true;
    }

    // Zone names are deck-unique among zones created through this manager, so a
    // same-named zone on the target board can only come from an imported deck.
    // Refuse the move instead of silently stacking same-named zones.
    for (const auto *targetZone : tree->getCustomZones(newBoardZoneName)) {
        if (targetZone->getName() == zoneName) {
            return false;
        }
    }

    QString reason =
        tr("Moved zone \"%1\" to %2").arg(zoneName, InnerDecklistNode::visibleNameFromName(newBoardZoneName));

    return modifyTree(reason, [&zoneName, &newBoardZoneName](DecklistNodeTree *tree) {
        return tree->moveCustomZone(zoneName, newBoardZoneName);
    });
}

bool DeckStateManager::removeCustomZone(const QString &zoneName)
{
    QString reason = tr("Deleted zone \"%1\"").arg(zoneName);

    return modifyTree(reason, [&zoneName](DecklistNodeTree *tree) { return tree->removeCustomZone(zoneName); });
}

QString DeckStateManager::validateNewZoneName(const QString &zoneName) const
{
    if (zoneName.trimmed().isEmpty()) {
        return tr("Enter a zone name.");
    }

    const QString trimmedZoneName = zoneName.trimmed();

    // The standard zone names are reserved even before they exist.
    if (trimmedZoneName == DECK_ZONE_MAIN || trimmedZoneName == DECK_ZONE_SIDE ||
        trimmedZoneName == DECK_ZONE_MAYBEBOARD || trimmedZoneName == DECK_ZONE_TOKENS) {
        return tr("This name is reserved.");
    }

    const auto *tree = deckList->getTree();

    // Reuse the tree's own uniqueness contract: any top-level zone and any
    // custom zone on *every* board claims the name (hasZoneName also reserves
    // the standard board names, which we already rejected with a dedicated
    // message above). Scanning only the standard boards here would miss a
    // custom zone an imported deck carries under `tokens`.
    if (tree->hasZoneName(trimmedZoneName)) {
        return tr("A zone with this name already exists.");
    }

    return {};
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
    emit deckReplaced();
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
    emit deckReplaced();
}

void DeckStateManager::requestHistorySave(const QString &reason)
{
    historyManager->save(deckList->createMemento(reason));
}

bool DeckStateManager::modifyTree(const QString &reason, const std::function<bool(DecklistNodeTree *)> &operation)
{
    DeckListMemento memento = deckList->createMemento(reason);
    bool success = operation(deckList->getTree());

    if (success) {
        historyManager->save(memento);
        deckListModel->rebuildTree();
        deckList->refreshDeckHash();
        emit deckListModel->deckHashChanged();
        // removeCustomZone can drop whole card sets the model never notified
        // about (rebuildTree emits no cardNodesChanged), so tell the consumers.
        emit deckListModel->cardNodesChanged();
        doCardModified();
    }

    return success;
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
