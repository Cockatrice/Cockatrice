#include "deck_list_model.h"

#include <libcockatrice/card/database/card_database_manager.h>

DeckListModel::DeckListModel(QObject *parent)
    : QAbstractItemModel(parent), lastKnownColumn(1), lastKnownOrder(Qt::AscendingOrder)
{
    deckList = QSharedPointer<DeckList>(new DeckList());
    root = new InnerDecklistNode;
}

DeckListModel::DeckListModel(QObject *parent, const QSharedPointer<DeckList> &deckList) : DeckListModel(parent)
{
    setDeckList(deckList);

    // forward change signals
    connect(this, &DeckListModel::cardAddedAt, this, &DeckListModel::cardsChanged);
    connect(this, &DeckListModel::cardRemoved, this, &DeckListModel::cardsChanged);
    connect(this, &DeckListModel::deckReplaced, this, &DeckListModel::cardsChanged);

    connect(this, &DeckListModel::cardNodeAddedAt, this, &DeckListModel::cardNodesChanged);
    connect(this, &DeckListModel::cardNodeRemoved, this, &DeckListModel::cardNodesChanged);
    connect(this, &DeckListModel::deckReplaced, this, &DeckListModel::cardNodesChanged);
}

DeckListModel::~DeckListModel()
{
    delete root;
}

/**
 * @brief Extract the value from the card that is used for the group criteria.
 * @param info Pointer to card information.
 * @param criteria The group criteria
 * @return String representing the value of the criteria.
 */
static QString extractGroupCriteriaValue(const CardInfoPtr &info, DeckListModelGroupCriteria::Type criteria)
{
    if (!info) {
        return "unknown";
    }

    switch (criteria) {
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

    InnerDecklistNode *listRoot = deckList->getTree()->getRoot();

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
            QString groupCriteria = extractGroupCriteriaValue(info, activeGroupCriteria);

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

        case DeckRoles::IsLegalRole: {
            return card->getFormatLegality();
        }

        default: {
            return {};
        }
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
            refreshCardFormatLegalities();
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
    emit deckHashChanged();

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
            deckList->getTree()->deleteNode(temp->getDataNode());
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

    QString groupCriteria = extractGroupCriteriaValue(info, activeGroupCriteria);
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

    QString groupCriteria = extractGroupCriteriaValue(cardInfo, activeGroupCriteria);
    InnerDecklistNode *groupNode = createNodeIfNeeded(groupCriteria, zoneNode);

    const QModelIndex parentIndex = nodeToIndex(groupNode);
    auto *cardNode = dynamic_cast<DecklistModelCardNode *>(groupNode->findCardChildByNameProviderIdAndNumber(
        card.getName(), printingInfo.getUuid(), printingInfo.getProperty("num")));
    const auto cardSetName = printingInfo.getSet().isNull() ? "" : printingInfo.getSet()->getCorrectedShortName();

    bool cardNodeAdded = false;
    if (!cardNode) {
        // Determine the correct index
        int insertRow = findSortedInsertRow(groupNode, cardInfo);

        auto *decklistCard =
            deckList->addCard(cardInfo->getName(), zoneName, insertRow, cardSetName, printingInfo.getProperty("num"),
                              printingInfo.getProperty("uuid"), cardInfo->isLegalInFormat(deckList->getGameFormat()));

        beginInsertRows(parentIndex, insertRow, insertRow);
        cardNode = new DecklistModelCardNode(decklistCard, groupNode, insertRow);
        endInsertRows();

        cardNodeAdded = true;
    } else {
        cardNode->setNumber(cardNode->getNumber() + 1);
        cardNode->setCardSetShortName(cardSetName);
        cardNode->setCardCollectorNumber(printingInfo.getProperty("num"));
        cardNode->setCardProviderId(printingInfo.getProperty("uuid"));
        deckList->refreshDeckHash();
        emit deckHashChanged();
    }
    sort(lastKnownColumn, lastKnownOrder);
    emitRecursiveUpdates(parentIndex);
    auto index = nodeToIndex(cardNode);

    if (cardNodeAdded) {
        emit cardNodeAddedAt(index);
    }

    emit cardAddedAt(index);

    return index;
}

bool DeckListModel::offsetCountAtIndex(const QModelIndex &idx, int offset)
{
    if (!idx.isValid()) {
        return false;
    }

    auto *node = static_cast<AbstractDecklistNode *>(idx.internalPointer());
    auto *card = dynamic_cast<DecklistModelCardNode *>(node);

    if (!card) {
        return false;
    }

    const QModelIndex numberIndex = idx.siblingAtColumn(DeckListModelColumns::CARD_AMOUNT);
    const int count = numberIndex.data(Qt::EditRole).toInt();
    const int newCount = count + offset;

    if (newCount <= 0) {
        removeRow(idx.row(), idx.parent());
        emit cardNodeRemoved();
    } else {
        setData(numberIndex, newCount, Qt::EditRole);
    }

    if (offset > 0) {
        emit cardAddedAt(idx);
    } else if (offset < 0) {
        emit cardRemoved();
    }

    return true;
}

bool DeckListModel::removeCardAtIndex(const QModelIndex &idx)
{
    if (!idx.isValid()) {
        return false;
    }

    auto *node = static_cast<AbstractDecklistNode *>(idx.internalPointer());
    auto *card = dynamic_cast<DecklistModelCardNode *>(node);

    if (!card) {
        return false;
    }

    bool success = removeRow(idx.row(), idx.parent());

    if (success) {
        emit cardRemoved();
    }

    return success;
}

int DeckListModel::findSortedInsertRow(const InnerDecklistNode *parent, const CardInfoPtr &cardInfo) const
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

void DeckListModel::setActiveFormat(const QString &_format)
{
    deckList->setGameFormat(_format);
    refreshCardFormatLegalities();
    emitBackgroundUpdates(QModelIndex()); // start from root
}

void DeckListModel::cleanList()
{
    setDeckList(QSharedPointer<DeckList>(new DeckList()));
}

/**
 * @param _deck The deck.
 */
void DeckListModel::setDeckList(const QSharedPointer<DeckList> &_deck)
{
    if (deckList != _deck) {
        deckList = _deck;
    }
    rebuildTree();
    refreshCardFormatLegalities();
    emit deckReplaced();
}

void DeckListModel::forEachCard(const std::function<void(InnerDecklistNode *, DecklistCardNode *)> &func)
{
    deckList->forEachCard(func);
}

QList<const DecklistCardNode *> DeckListModel::getCardNodes() const
{
    return deckList->getCardNodes();
}

QList<const DecklistCardNode *> DeckListModel::getCardNodesForZone(const QString &zoneName) const
{
    return deckList->getCardNodes({zoneName});
}

QList<QString> DeckListModel::getCardNames() const
{
    auto nodes = deckList->getCardNodes();

    QList<QString> names;
    std::transform(nodes.cbegin(), nodes.cend(), std::back_inserter(names), [](auto node) { return node->getName(); });

    return names;
}

QList<CardRef> DeckListModel::getCardRefs() const
{
    auto nodes = deckList->getCardNodes();

    QList<CardRef> cardRefs;
    std::transform(nodes.cbegin(), nodes.cend(), std::back_inserter(cardRefs),
                   [](auto node) { return node->toCardRef(); });

    return cardRefs;
}

QList<QString> DeckListModel::getZones() const
{
    auto zoneNodes = deckList->getZoneNodes();

    QList<QString> zones;
    std::transform(zoneNodes.cbegin(), zoneNodes.cend(), std::back_inserter(zones),
                   [](auto zoneNode) { return zoneNode->getName(); });

    return zones;
}

static int maxAllowedForLegality(const FormatRules &format, const QString &legality)
{
    for (const AllowedCount &c : format.allowedCounts) {
        if (c.label == legality) {
            return c.max;
        }
    }
    return -1; // unknown legality → treat as illegal
}

static bool isCardQuantityLegalForFormat(const QString &format, const CardInfo &cardInfo, int quantity)
{
    if (format.isEmpty()) {
        return true;
    }

    auto formatRules = CardDatabaseManager::query()->getFormat(format);

    // if format has no custom rules, then just do the default check
    if (!formatRules) {
        return cardInfo.isLegalInFormat(format);
    }

    // Exceptions always win
    if (cardHasAnyException(cardInfo, *formatRules)) {
        return true;
    }

    const QString legalityProp = "format-" + format;
    if (!cardInfo.getProperties().contains(legalityProp)) {
        return false;
    }

    const QString legality = cardInfo.getProperty(legalityProp);

    int maxAllowed = maxAllowedForLegality(*formatRules, legality);

    if (maxAllowed == -1) {
        return false;
    }

    if (maxAllowed < 0) { // unlimited
        return true;
    }

    return quantity <= maxAllowed;
}

void DeckListModel::refreshCardFormatLegalities()
{
    InnerDecklistNode *listRoot = deckList->getTree()->getRoot();

    for (int i = 0; i < listRoot->size(); i++) {
        auto *currentZone = static_cast<InnerDecklistNode *>(listRoot->at(i));
        for (int j = 0; j < currentZone->size(); j++) {
            auto *currentCard = static_cast<DecklistCardNode *>(currentZone->at(j));

            // TODO: better sanity checking
            if (currentCard == nullptr) {
                continue;
            }

            ExactCard exactCard = CardDatabaseManager::query()->getCard(currentCard->toCardRef());
            if (!exactCard) {
                continue;
            }

            QString format = deckList->getGameFormat();
            bool legal = isCardQuantityLegalForFormat(format, exactCard.getInfo(), currentCard->getNumber());

            currentCard->setFormatLegality(legal);
        }
    }
}
