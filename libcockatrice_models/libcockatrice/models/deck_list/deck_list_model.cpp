#include "deck_list_model.h"

#include <libcockatrice/card/database/card_database_manager.h>

DeckListModel::DeckListModel(QObject *parent)
    : QAbstractItemModel(parent), lastKnownColumn(1), lastKnownOrder(Qt::AscendingOrder)
{
    deckList = new DeckList;
    deckList->setParent(this);
    root = new InnerDecklistNode;
}

DeckListModel::~DeckListModel()
{
    delete root;
}

QString DeckListModel::getGroupCriteriaForCard(CardInfoPtr info) const
{
    if (!info) {
        return "unknown";
    }

    switch (activeGroupCriteria) {
        case DeckListModelGroupCriteria::MAIN_TYPE:
            return info->getMainCardType();
        case DeckListModelGroupCriteria::MANA_COST:
            return info->getCmc();
        case DeckListModelGroupCriteria::COLOR:
            return info->getColors() == "" ? "Colorless" : info->getColors();
        default:
            return "unknown";
    }
}

void DeckListModel::rebuildTree()
{
    beginResetModel();
    root->clearTree();

    InnerDecklistNode *listRoot = deckList->getRoot();

    for (int i = 0; i < listRoot->size(); i++) {
        auto *currentZone = dynamic_cast<InnerDecklistNode *>(listRoot->at(i));
        auto *node = new InnerDecklistNode(currentZone->getName(), root);

        for (int j = 0; j < currentZone->size(); j++) {
            auto *currentCard = dynamic_cast<DecklistCardNode *>(currentZone->at(j));

            // TODO: better sanity checking
            if (currentCard == nullptr) {
                continue;
            }

            CardInfoPtr info = CardDatabaseManager::query()->getCardInfo(currentCard->getName());
            QString groupCriteria = getGroupCriteriaForCard(info);

            auto *groupNode = dynamic_cast<InnerDecklistNode *>(node->findChild(groupCriteria));

            if (!groupNode) {
                groupNode = new InnerDecklistNode(groupCriteria, node);
            }

            new DecklistModelCardNode(currentCard, groupNode);
        }
    }

    endResetModel();
}

int DeckListModel::rowCount(const QModelIndex &parent) const
{
    // debugIndexInfo("rowCount", parent);
    auto *node = getNode<InnerDecklistNode *>(parent);
    if (node) {
        return node->size();
    } else {
        return 0;
    }
}

int DeckListModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 5;
}

QVariant DeckListModel::data(const QModelIndex &index, int role) const
{
    // debugIndexInfo("data", index);
    if (!index.isValid()) {
        return {};
    }

    if (index.column() >= columnCount()) {
        return {};
    }

    auto *node = static_cast<AbstractDecklistNode *>(index.internalPointer());
    auto *card = dynamic_cast<DecklistModelCardNode *>(node);

    // Group node
    if (!card) {
        const auto *group = dynamic_cast<InnerDecklistNode *>(node);

        switch (role) {
            case Qt::DisplayRole:
            case Qt::EditRole: {
                switch (index.column()) {
                    case DeckListModelColumns::CARD_AMOUNT:
                        return group->recursiveCount(true);
                    case DeckListModelColumns::CARD_NAME:
                        if (role == Qt::DisplayRole) {
                            return group->getVisibleName();
                        }
                        return group->getName();
                    case DeckListModelColumns::CARD_SET:
                        return group->getCardSetShortName();
                    case DeckListModelColumns::CARD_COLLECTOR_NUMBER:
                        return group->getCardCollectorNumber();
                    case DeckListModelColumns::CARD_PROVIDER_ID:
                        return group->getCardProviderId();
                    default:
                        return {};
                }
            }
            case DeckRoles::IsCardRole:
                return false;

            case DeckRoles::DepthRole:
                return group->depth();

                // legality does not apply to group nodes
            case DeckRoles::IsLegalRole:
                return true;

            default:
                return {};
        }
    }

    // Card node
    switch (role) {
        case Qt::DisplayRole:
        case Qt::EditRole:
            switch (index.column()) {
                case DeckListModelColumns::CARD_AMOUNT:
                    return card->getNumber();
                case DeckListModelColumns::CARD_NAME:
                    return card->getName();
                case DeckListModelColumns::CARD_SET:
                    return card->getCardSetShortName();
                case DeckListModelColumns::CARD_COLLECTOR_NUMBER:
                    return card->getCardCollectorNumber();
                case DeckListModelColumns::CARD_PROVIDER_ID:
                    return card->getCardProviderId();
                default:
                    return {};
            }

        case DeckRoles::IsCardRole: {
            return true;
        }

        case DeckRoles::DepthRole: {
            return card->depth();
        }

        default: {
            return {};
        }
    }
}

void DeckListModel::emitBackgroundUpdates(const QModelIndex &parent)
{
    int rows = rowCount(parent);
    if (rows == 0)
        return;

    QModelIndex topLeft = index(0, 0, parent);
    QModelIndex bottomRight = index(rows - 1, columnCount() - 1, parent);
    emit dataChanged(topLeft, bottomRight, {Qt::BackgroundRole});

    for (int r = 0; r < rows; ++r) {
        QModelIndex child = index(r, 0, parent);
        emitBackgroundUpdates(child);
    }
}

QVariant DeckListModel::headerData(const int section, const Qt::Orientation orientation, const int role) const
{
    if ((role != Qt::DisplayRole) || (orientation != Qt::Horizontal)) {
        return {};
    }

    if (section >= columnCount()) {
        return {};
    }

    switch (section) {
        case DeckListModelColumns::CARD_AMOUNT:
            return tr("Count");
        case DeckListModelColumns::CARD_NAME:
            return tr("Card");
        case DeckListModelColumns::CARD_SET:
            return tr("Set");
        case DeckListModelColumns::CARD_COLLECTOR_NUMBER:
            return tr("Number");
        case DeckListModelColumns::CARD_PROVIDER_ID:
            return tr("Provider ID");
        default:
            return {};
    }
}

QModelIndex DeckListModel::index(int row, int column, const QModelIndex &parent) const
{
    // debugIndexInfo("index", parent);
    if (!hasIndex(row, column, parent)) {
        return {};
    }

    auto *parentNode = getNode<InnerDecklistNode *>(parent);
    return row >= parentNode->size() ? QModelIndex() : createIndex(row, column, parentNode->at(row));
}

QModelIndex DeckListModel::parent(const QModelIndex &ind) const
{
    if (!ind.isValid()) {
        return {};
    }

    return nodeToIndex(static_cast<AbstractDecklistNode *>(ind.internalPointer())->getParent());
}

Qt::ItemFlags DeckListModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    Qt::ItemFlags result = Qt::ItemIsEnabled;
    result |= Qt::ItemIsSelectable;

    return result;
}

void DeckListModel::emitRecursiveUpdates(const QModelIndex &index)
{
    if (!index.isValid()) {
        return;
    }

    emit dataChanged(index, index);
    emitRecursiveUpdates(index.parent());
}

bool DeckListModel::setData(const QModelIndex &index, const QVariant &value, const int role)
{
    auto *node = getNode<DecklistModelCardNode *>(index);
    if (!node || (role != Qt::EditRole)) {
        return false;
    }

    switch (index.column()) {
        case DeckListModelColumns::CARD_AMOUNT:
            node->setNumber(value.toInt());
            break;
        case DeckListModelColumns::CARD_NAME:
            node->setName(value.toString());
            break;
        case DeckListModelColumns::CARD_SET:
            node->setCardSetShortName(value.toString());
            break;
        case DeckListModelColumns::CARD_COLLECTOR_NUMBER:
            node->setCardCollectorNumber(value.toString());
            break;
        case DeckListModelColumns::CARD_PROVIDER_ID:
            node->setCardProviderId(value.toString());
            break;
        default:
            return false;
    }

    emitRecursiveUpdates(index);
    deckList->refreshDeckHash();

    emit dataChanged(index, index);
    return true;
}

bool DeckListModel::removeRows(int row, int count, const QModelIndex &parent)
{
    auto *node = getNode<InnerDecklistNode *>(parent);
    if (!node) {
        return false;
    }

    if (row + count > node->size()) {
        return false;
    }

    beginRemoveRows(parent, row, row + count - 1);
    for (int i = 0; i < count; i++) {
        AbstractDecklistNode *toDelete = node->takeAt(row);
        if (auto *temp = dynamic_cast<DecklistModelCardNode *>(toDelete)) {
            deckList->deleteNode(temp->getDataNode());
        }
        delete toDelete;
    }
    endRemoveRows();

    if (node->empty() && (node != root)) {
        removeRows(parent.row(), 1, parent.parent());
    } else {
        emitRecursiveUpdates(parent);
    }

    return true;
}

InnerDecklistNode *DeckListModel::createNodeIfNeeded(const QString &name, InnerDecklistNode *parent)
{
    auto *newNode = dynamic_cast<InnerDecklistNode *>(parent->findChild(name));
    if (!newNode) {
        beginInsertRows(nodeToIndex(parent), parent->size(), parent->size());
        newNode = new InnerDecklistNode(name, parent);
        endInsertRows();
    }
    return newNode;
}

DecklistModelCardNode *DeckListModel::findCardNode(const QString &cardName,
                                                   const QString &zoneName,
                                                   const QString &providerId,
                                                   const QString &cardNumber) const
{
    InnerDecklistNode *zoneNode = dynamic_cast<InnerDecklistNode *>(root->findChild(zoneName));
    if (!zoneNode) {
        return nullptr;
    }

    CardInfoPtr info = CardDatabaseManager::query()->getCardInfo(cardName);
    if (!info) {
        return nullptr;
    }

    QString groupCriteria = getGroupCriteriaForCard(info);
    InnerDecklistNode *groupNode = dynamic_cast<InnerDecklistNode *>(zoneNode->findChild(groupCriteria));
    if (!groupNode) {
        return nullptr;
    }

    return dynamic_cast<DecklistModelCardNode *>(
        groupNode->findCardChildByNameProviderIdAndNumber(cardName, providerId, cardNumber));
}

QModelIndex DeckListModel::findCard(const QString &cardName,
                                    const QString &zoneName,
                                    const QString &providerId,
                                    const QString &cardNumber) const
{
    DecklistModelCardNode *cardNode = findCardNode(cardName, zoneName, providerId, cardNumber);
    if (!cardNode) {
        return {};
    }

    return nodeToIndex(cardNode);
}

QModelIndex DeckListModel::addPreferredPrintingCard(const QString &cardName, const QString &zoneName, bool abAddAnyway)
{
    ExactCard card = CardDatabaseManager::query()->getCard({cardName});

    if (!card) {
        if (abAddAnyway) {
            // We need to keep this card added no matter what
            // This is usually called from tab_deck_editor
            // So we'll create a new CardInfo with the name
            // and default values for all fields
            card = ExactCard(CardInfo::newInstance(cardName));
        } else {
            return {};
        }
    }

    return addCard(card, zoneName);
}

QModelIndex DeckListModel::addCard(const ExactCard &card, const QString &zoneName)
{
    if (!card) {
        return {};
    }

    InnerDecklistNode *zoneNode = createNodeIfNeeded(zoneName, root);

    CardInfoPtr cardInfo = card.getCardPtr();
    PrintingInfo printingInfo = card.getPrinting();

    QString groupCriteria = getGroupCriteriaForCard(cardInfo);
    InnerDecklistNode *groupNode = createNodeIfNeeded(groupCriteria, zoneNode);

    const QModelIndex parentIndex = nodeToIndex(groupNode);
    auto *cardNode = dynamic_cast<DecklistModelCardNode *>(groupNode->findCardChildByNameProviderIdAndNumber(
        card.getName(), printingInfo.getUuid(), printingInfo.getProperty("num")));
    const auto cardSetName = printingInfo.getSet().isNull() ? "" : printingInfo.getSet()->getCorrectedShortName();

    if (!cardNode) {
        // Determine the correct index
        int insertRow = findSortedInsertRow(groupNode, cardInfo);

        auto *decklistCard = deckList->addCard(cardInfo->getName(), zoneName, insertRow, cardSetName,
                                               printingInfo.getProperty("num"), printingInfo.getProperty("uuid"));

        beginInsertRows(parentIndex, insertRow, insertRow);
        cardNode = new DecklistModelCardNode(decklistCard, groupNode, insertRow);
        endInsertRows();
    } else {
        cardNode->setNumber(cardNode->getNumber() + 1);
        cardNode->setCardSetShortName(cardSetName);
        cardNode->setCardCollectorNumber(printingInfo.getProperty("num"));
        cardNode->setCardProviderId(printingInfo.getProperty("uuid"));
        deckList->refreshDeckHash();
    }
    sort(lastKnownColumn, lastKnownOrder);
    emitRecursiveUpdates(parentIndex);
    return nodeToIndex(cardNode);
}

int DeckListModel::findSortedInsertRow(InnerDecklistNode *parent, CardInfoPtr cardInfo) const
{
    if (!cardInfo) {
        return parent->size(); // fallback: append at end
    }

    for (int i = 0; i < parent->size(); ++i) {
        auto *existingCard = dynamic_cast<DecklistModelCardNode *>(parent->at(i));
        if (!existingCard)
            continue;

        bool lessThan = false;
        switch (lastKnownColumn) {
            case 0: // ByNumber
                lessThan = lastKnownOrder == Qt::AscendingOrder
                               ? cardInfo->getProperty("collectorNumber") < existingCard->getCardCollectorNumber()
                               : cardInfo->getProperty("collectorNumber") > existingCard->getCardCollectorNumber();
                break;
            case 1: // ByName
            default:
                lessThan = lastKnownOrder == Qt::AscendingOrder
                               ? cardInfo->getName().localeAwareCompare(existingCard->getName()) < 0
                               : cardInfo->getName().localeAwareCompare(existingCard->getName()) > 0;
                break;
        }

        if (lessThan)
            return i;
    }

    return parent->size(); // insert at end if no earlier match
}

QModelIndex DeckListModel::nodeToIndex(AbstractDecklistNode *node) const
{
    if (node == nullptr || node == root) {
        return {};
    }

    return createIndex(node->getParent()->indexOf(node), 0, node);
}

void DeckListModel::sortHelper(InnerDecklistNode *node, Qt::SortOrder order)
{
    // Sort children of node and save the information needed to
    // update the list of persistent indexes.
    QVector<QPair<int, int>> sortResult = node->sort(order);

    QModelIndexList from, to;
    int columns = columnCount();
    for (int i = sortResult.size() - 1; i >= 0; --i) {
        const int fromRow = sortResult[i].first;
        const int toRow = sortResult[i].second;
        AbstractDecklistNode *temp = node->at(toRow);
        for (int j = 0; j < columns; ++j) {
            from << createIndex(fromRow, j, temp);
            to << createIndex(toRow, j, temp);
        }
    }
    changePersistentIndexList(from, to);

    // Recursion
    for (int i = node->size() - 1; i >= 0; --i) {
        auto *subNode = dynamic_cast<InnerDecklistNode *>(node->at(i));
        if (subNode) {
            sortHelper(subNode, order);
        }
    }
}

void DeckListModel::sort(int column, Qt::SortOrder order)
{
    lastKnownColumn = column;
    lastKnownOrder = order;

    emit layoutAboutToBeChanged();
    DeckSortMethod sortMethod;
    switch (column) {
        case 0:
            sortMethod = ByNumber;
            break;
        case 1:
            sortMethod = ByName;
            break;
        default:
            sortMethod = ByName;
    }

    root->setSortMethod(sortMethod);
    sortHelper(root, order);
    emit layoutChanged();
}

void DeckListModel::setActiveGroupCriteria(DeckListModelGroupCriteria::Type newCriteria)
{
    activeGroupCriteria = newCriteria;
    rebuildTree();
}

void DeckListModel::cleanList()
{
    setDeckList(new DeckList);
}

/**
 * @param _deck The deck. Takes ownership of the object
 */
void DeckListModel::setDeckList(DeckList *_deck)
{
    deckList->deleteLater();
    deckList = _deck;
    deckList->setParent(this);
    rebuildTree();
}

QList<ExactCard> DeckListModel::getCards() const
{
    QList<ExactCard> cards;
    DeckList *decklist = getDeckList();
    if (!decklist) {
        return cards;
    }
    InnerDecklistNode *listRoot = decklist->getRoot();
    if (!listRoot)
        return cards;

    for (int i = 0; i < listRoot->size(); i++) {
        InnerDecklistNode *currentZone = dynamic_cast<InnerDecklistNode *>(listRoot->at(i));
        if (!currentZone)
            continue;
        for (int j = 0; j < currentZone->size(); j++) {
            DecklistCardNode *currentCard = dynamic_cast<DecklistCardNode *>(currentZone->at(j));
            if (!currentCard)
                continue;
            for (int k = 0; k < currentCard->getNumber(); ++k) {
                ExactCard card = CardDatabaseManager::query()->getCard(currentCard->toCardRef());
                if (card) {
                    cards.append(card);
                } else {
                    qDebug() << "Card not found in database!";
                }
            }
        }
    }
    return cards;
}

QList<ExactCard> DeckListModel::getCardsForZone(const QString &zoneName) const
{
    QList<ExactCard> cards;
    DeckList *decklist = getDeckList();
    if (!decklist) {
        return cards;
    }
    InnerDecklistNode *listRoot = decklist->getRoot();
    if (!listRoot)
        return cards;

    for (int i = 0; i < listRoot->size(); i++) {
        InnerDecklistNode *currentZone = dynamic_cast<InnerDecklistNode *>(listRoot->at(i));
        if (!currentZone)
            continue;
        if (currentZone->getName() == zoneName) {
            for (int j = 0; j < currentZone->size(); j++) {
                DecklistCardNode *currentCard = dynamic_cast<DecklistCardNode *>(currentZone->at(j));
                if (!currentCard)
                    continue;
                for (int k = 0; k < currentCard->getNumber(); ++k) {
                    ExactCard card = CardDatabaseManager::query()->getCard(currentCard->toCardRef());
                    if (card) {
                        cards.append(card);
                    } else {
                        qDebug() << "Card not found in database!";
                    }
                }
            }
        }
    }
    return cards;
}

QList<QString> *DeckListModel::getZones() const
{
    QList<QString> *zones = new QList<QString>();
    DeckList *decklist = getDeckList();
    if (!decklist) {
        return zones;
    }
    InnerDecklistNode *listRoot = decklist->getRoot();
    if (!listRoot)
        return zones;

    for (int i = 0; i < listRoot->size(); i++) {
        InnerDecklistNode *currentZone = dynamic_cast<InnerDecklistNode *>(listRoot->at(i));
        if (!currentZone)
            continue;
        zones->append(currentZone->getName());
    }
    return zones;
}