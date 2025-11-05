#include "deck_list_model.h"

#include "deck_loader.h"

#include <QBrush>
#include <QFont>
#include <QPrinter>
#include <QProgressDialog>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextStream>
#include <QTextTable>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/settings/cache_settings.h>

DeckListModel::DeckListModel(QObject *parent)
    : QAbstractItemModel(parent), lastKnownColumn(1), lastKnownOrder(Qt::AscendingOrder)
{
    deckList = new DeckLoader;
    deckList->setParent(this);
    connect(deckList, &DeckLoader::deckLoaded, this, &DeckListModel::rebuildTree);
    connect(deckList, &DeckLoader::deckHashChanged, this, &DeckListModel::deckHashChanged);
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

    auto *temp = static_cast<AbstractDecklistNode *>(index.internalPointer());
    auto *card = dynamic_cast<DecklistModelCardNode *>(temp);
    if (card == nullptr) {
        const auto *node = dynamic_cast<InnerDecklistNode *>(temp);
        switch (role) {
            case Qt::FontRole: {
                QFont f;
                f.setBold(true);
                return f;
            }
            case Qt::DisplayRole:
            case Qt::EditRole: {
                switch (index.column()) {
                    case 0:
                        return node->recursiveCount(true);
                    case 1: {
                        if (role == Qt::DisplayRole)
                            return node->getVisibleName();
                        return node->getName();
                    }
                    case 2: {
                        return node->getCardSetShortName();
                    }
                    case 3: {
                        return node->getCardCollectorNumber();
                    }
                    case 4: {
                        return node->getCardProviderId();
                    }
                    default:
                        return {};
                }
            }
            case Qt::UserRole + 1:
                return false;
            case Qt::BackgroundRole: {
                int color = 90 + 60 * node->depth();
                return QBrush(QColor(color, 255, color));
            }
            case Qt::ForegroundRole: {
                return QBrush(QColor(0, 0, 0));
            }
            default:
                return {};
        }
    } else {
        switch (role) {
            case Qt::DisplayRole:
            case Qt::EditRole: {
                switch (index.column()) {
                    case 0:
                        return card->getNumber();
                    case 1:
                        return card->getName();
                    case 2:
                        return card->getCardSetShortName();
                    case 3:
                        return card->getCardCollectorNumber();
                    case 4:
                        return card->getCardProviderId();
                    default:
                        return {};
                }
            }
            case Qt::UserRole + 1:
                return true;
            case Qt::BackgroundRole: {
                int color = 255 - (index.row() % 2) * 30;
                return QBrush(QColor(color, color, color));
            }
            case Qt::ForegroundRole: {
                return QBrush(QColor(0, 0, 0));
            }
            default:
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
        case 0:
            return tr("Count");
        case 1:
            return tr("Card");
        case 2:
            return tr("Set");
        case 3:
            return tr("Number");
        case 4:
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
        case 0:
            node->setNumber(value.toInt());
            break;
        case 1:
            node->setName(value.toString());
            break;
        case 2:
            node->setCardSetShortName(value.toString());
            break;
        case 3:
            node->setCardCollectorNumber(value.toString());
            break;
        case 4:
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

void DeckListModel::setActiveGroupCriteria(DeckListModelGroupCriteria newCriteria)
{
    activeGroupCriteria = newCriteria;
    rebuildTree();
}

void DeckListModel::cleanList()
{
    setDeckList(new DeckLoader);
}

/**
 * @param _deck The deck. Takes ownership of the object
 */
void DeckListModel::setDeckList(DeckLoader *_deck)
{
    deckList->deleteLater();
    deckList = _deck;
    deckList->setParent(this);
    connect(deckList, &DeckLoader::deckLoaded, this, &DeckListModel::rebuildTree);
    connect(deckList, &DeckLoader::deckHashChanged, this, &DeckListModel::deckHashChanged);
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

void DeckListModel::printDeckListNode(QTextCursor *cursor, InnerDecklistNode *node)
{
    const int totalColumns = 2;

    if (node->height() == 1) {
        QTextBlockFormat blockFormat;
        QTextCharFormat charFormat;
        charFormat.setFontPointSize(11);
        charFormat.setFontWeight(QFont::Bold);
        cursor->insertBlock(blockFormat, charFormat);

        QTextTableFormat tableFormat;
        tableFormat.setCellPadding(0);
        tableFormat.setCellSpacing(0);
        tableFormat.setBorder(0);
        QTextTable *table = cursor->insertTable(node->size() + 1, totalColumns, tableFormat);
        for (int i = 0; i < node->size(); i++) {
            auto *card = dynamic_cast<AbstractDecklistCardNode *>(node->at(i));

            QTextCharFormat cellCharFormat;
            cellCharFormat.setFontPointSize(9);

            QTextTableCell cell = table->cellAt(i, 0);
            cell.setFormat(cellCharFormat);
            QTextCursor cellCursor = cell.firstCursorPosition();
            cellCursor.insertText(QString("%1 ").arg(card->getNumber()));

            cell = table->cellAt(i, 1);
            cell.setFormat(cellCharFormat);
            cellCursor = cell.firstCursorPosition();
            cellCursor.insertText(card->getName());
        }
    } else if (node->height() == 2) {
        QTextBlockFormat blockFormat;
        QTextCharFormat charFormat;
        charFormat.setFontPointSize(14);
        charFormat.setFontWeight(QFont::Bold);

        cursor->insertBlock(blockFormat, charFormat);

        QTextTableFormat tableFormat;
        tableFormat.setCellPadding(10);
        tableFormat.setCellSpacing(0);
        tableFormat.setBorder(0);
        QVector<QTextLength> constraints;
        for (int i = 0; i < totalColumns; i++) {
            constraints << QTextLength(QTextLength::PercentageLength, 100.0 / totalColumns);
        }
        tableFormat.setColumnWidthConstraints(constraints);

        QTextTable *table = cursor->insertTable(1, totalColumns, tableFormat);
        for (int i = 0; i < node->size(); i++) {
            QTextCursor cellCursor = table->cellAt(0, (i * totalColumns) / node->size()).lastCursorPosition();
            printDeckListNode(&cellCursor, dynamic_cast<InnerDecklistNode *>(node->at(i)));
        }
    }

    cursor->movePosition(QTextCursor::End);
}

void DeckListModel::printDeckList(QPrinter *printer)
{
    QTextDocument doc;

    QFont font("Serif");
    font.setStyleHint(QFont::Serif);
    doc.setDefaultFont(font);

    QTextCursor cursor(&doc);

    QTextBlockFormat headerBlockFormat;
    QTextCharFormat headerCharFormat;
    headerCharFormat.setFontPointSize(16);
    headerCharFormat.setFontWeight(QFont::Bold);

    cursor.insertBlock(headerBlockFormat, headerCharFormat);
    cursor.insertText(deckList->getName());

    headerCharFormat.setFontPointSize(12);
    cursor.insertBlock(headerBlockFormat, headerCharFormat);
    cursor.insertText(deckList->getComments());
    cursor.insertBlock(headerBlockFormat, headerCharFormat);

    for (int i = 0; i < root->size(); i++) {
        cursor.insertHtml("<br><img src=theme:hr.jpg>");
        // cursor.insertHtml("<hr>");
        cursor.insertBlock(headerBlockFormat, headerCharFormat);

        printDeckListNode(&cursor, dynamic_cast<InnerDecklistNode *>(root->at(i)));
    }

    doc.print(printer);
}
