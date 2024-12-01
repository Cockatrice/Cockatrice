#include "deck_list_model.h"

#include "../game/cards/card_database.h"
#include "../game/cards/card_database_manager.h"
#include "../main.h"
#include "../settings/cache_settings.h"
#include "deck_loader.h"

#include <QBrush>
#include <QFile>
#include <QFont>
#include <QPrinter>
#include <QProgressDialog>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextStream>
#include <QTextTable>

DeckListModel::DeckListModel(QObject *parent)
    : QAbstractItemModel(parent), lastKnownColumn(1), lastKnownOrder(Qt::AscendingOrder)
{
    deckList = new DeckLoader;
    connect(deckList, SIGNAL(deckLoaded()), this, SLOT(rebuildTree()));
    connect(deckList, SIGNAL(deckHashChanged()), this, SIGNAL(deckHashChanged()));
    root = new InnerDecklistNode;
}

DeckListModel::~DeckListModel()
{
    delete root;
    delete deckList;
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

            CardInfoPtr info = CardDatabaseManager::getInstance()->getCard(currentCard->getName());
            QString cardType = info ? info->getMainCardType() : "unknown";

            auto *cardTypeNode = dynamic_cast<InnerDecklistNode *>(node->findChild(cardType));

            if (!cardTypeNode) {
                cardTypeNode = new InnerDecklistNode(cardType, node);
            }

            new DecklistModelCardNode(currentCard, cardTypeNode);
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
    deckList->updateDeckHash();
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

DecklistModelCardNode *
DeckListModel::findCardNode(const QString &cardName, const QString &zoneName, const QString &providerId) const
{
    InnerDecklistNode *zoneNode, *typeNode;
    CardInfoPtr info;
    QString cardType;

    zoneNode = dynamic_cast<InnerDecklistNode *>(root->findChild(zoneName));
    if (!zoneNode) {
        return nullptr;
    }

    info = CardDatabaseManager::getInstance()->getCard(cardName);
    if (!info) {
        return nullptr;
    }

    cardType = info->getMainCardType();
    typeNode = dynamic_cast<InnerDecklistNode *>(zoneNode->findChild(cardType));
    if (!typeNode) {
        return nullptr;
    }

    if (providerId.isEmpty()) {
        return dynamic_cast<DecklistModelCardNode *>(typeNode->findChild(cardName));
    }
    return dynamic_cast<DecklistModelCardNode *>(typeNode->findCardChildByNameAndProviderId(cardName, providerId));
}

QModelIndex DeckListModel::findCard(const QString &cardName, const QString &zoneName, const QString &providerId) const
{
    DecklistModelCardNode *cardNode;

    cardNode = findCardNode(cardName, zoneName, providerId);
    if (!cardNode) {
        return {};
    }

    return nodeToIndex(cardNode);
}

QModelIndex DeckListModel::addPreferredPrintingCard(const QString &cardName, const QString &zoneName, bool abAddAnyway)
{
    return addCard(cardName, CardDatabaseManager::getInstance()->getPreferredSetForCard(cardName), zoneName,
                   abAddAnyway);
}

QModelIndex DeckListModel::addCard(const QString &cardName,
                                   const CardInfoPerSet cardInfoSet,
                                   const QString &zoneName,
                                   bool abAddAnyway)
{
    CardInfoPtr cardInfo =
        CardDatabaseManager::getInstance()->getCardByNameAndProviderId(cardName, cardInfoSet.getProperty("uuid"));

    if (cardInfo == nullptr) {
        if (abAddAnyway) {
            // We need to keep this card added no matter what
            // This is usually called from tab_deck_editor
            // So we'll create a new CardInfo with the name
            // and default values for all fields
            cardInfo = CardInfo::newInstance(cardName);
        } else {
            return {};
        }
    }

    InnerDecklistNode *zoneNode = createNodeIfNeeded(zoneName, root);

    const QString cardType = cardInfo->getMainCardType();
    InnerDecklistNode *cardTypeNode = createNodeIfNeeded(cardType, zoneNode);

    const QModelIndex parentIndex = nodeToIndex(cardTypeNode);
    auto *cardNode = dynamic_cast<DecklistModelCardNode *>(
        cardTypeNode->findCardChildByNameAndProviderId(cardName, cardInfoSet.getProperty("uuid")));
    if (!cardNode) {
        auto *decklistCard =
            deckList->addCard(cardInfo->getName(), zoneName, cardInfoSet.getPtr()->getCorrectedShortName(),
                              cardInfoSet.getProperty("num"), cardInfoSet.getProperty("uuid"));
        beginInsertRows(parentIndex, static_cast<int>(cardTypeNode->size()), static_cast<int>(cardTypeNode->size()));
        cardNode = new DecklistModelCardNode(decklistCard, cardTypeNode);
        endInsertRows();
    } else {
        cardNode->setNumber(cardNode->getNumber() + 1);
        cardNode->setCardSetShortName(cardInfoSet.getPtr()->getCorrectedShortName());
        cardNode->setCardCollectorNumber(cardInfoSet.getProperty("num"));
        cardNode->setCardProviderId(cardInfoSet.getProperty("uuid"));
        deckList->updateDeckHash();
    }
    sort(lastKnownColumn, lastKnownOrder);
    emitRecursiveUpdates(parentIndex);
    return nodeToIndex(cardNode);
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

void DeckListModel::cleanList()
{
    setDeckList(new DeckLoader);
}

void DeckListModel::setDeckList(DeckLoader *_deck)
{
    delete deckList;
    deckList = _deck;
    connect(deckList, SIGNAL(deckLoaded()), this, SLOT(rebuildTree()));
    connect(deckList, SIGNAL(deckHashChanged()), this, SIGNAL(deckHashChanged()));
    rebuildTree();
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
