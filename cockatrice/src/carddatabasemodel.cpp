#include "carddatabasemodel.h"
#include "filtertree.h"

#define CARDDBMODEL_COLUMNS 5

CardDatabaseModel::CardDatabaseModel(CardDatabase *_db, QObject *parent)
    : QAbstractListModel(parent), db(_db)
{
    connect(db, SIGNAL(cardListChanged()), this, SLOT(updateCardList()));
    connect(db, SIGNAL(cardAdded(CardInfo *)), this, SLOT(cardAdded(CardInfo *)));
    connect(db, SIGNAL(cardRemoved(CardInfo *)), this, SLOT(cardRemoved(CardInfo *)));
    updateCardList();
}

CardDatabaseModel::~CardDatabaseModel()
{
}

int CardDatabaseModel::rowCount(const QModelIndex &/*parent*/) const
{
    return cardList.size();
}

int CardDatabaseModel::columnCount(const QModelIndex &/*parent*/) const
{
    return CARDDBMODEL_COLUMNS;
}

QVariant CardDatabaseModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() ||
        index.row() >= cardList.size() || 
        index.column() >= CARDDBMODEL_COLUMNS || 
        (role != Qt::DisplayRole && role != SortRole))
        return QVariant();

    CardInfo *card = cardList.at(index.row());
    switch (index.column()){
        case NameColumn: return card->getName();
        case SetListColumn: return card->getSetsNames();
        case ManaCostColumn: return role == SortRole ?
            QString("%1%2").arg(card->getCmc(), 4, QChar('0')).arg(card->getManaCost()) :
            card->getManaCost();
        case CardTypeColumn: return card->getCardType();
        case PTColumn: return card->getPowTough();
        default: return QVariant();
    }
}

QVariant CardDatabaseModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();
    if (orientation != Qt::Horizontal)
        return QVariant();
    switch (section) {
        case NameColumn: return QString(tr("Name"));
        case SetListColumn: return QString(tr("Sets"));
        case ManaCostColumn: return QString(tr("Mana cost"));
        case CardTypeColumn: return QString(tr("Card type"));
        case PTColumn: return QString(tr("P/T"));
        default: return QVariant();
    }
}

void CardDatabaseModel::updateCardList()
{
    beginResetModel();

    for (int i = 0; i < cardList.size(); ++i)
        disconnect(cardList[i], 0, this, 0);
    
    cardList.clear();

    foreach(CardInfo * card, db->getCardList())
    {
        bool hasSet = false;
        foreach(CardSet * set, card->getSets())
        {
            if(set->getEnabled())
            {
                hasSet = true;
                break;
            }
        }

        if(hasSet)
        {
            cardList.append(card);
            connect(card, SIGNAL(cardInfoChanged(CardInfo *)), this, SLOT(cardInfoChanged(CardInfo *)));
        }
    }
    
    endResetModel();
}

void CardDatabaseModel::cardInfoChanged(CardInfo *card)
{
    const int row = cardList.indexOf(card);
    if (row == -1)
        return;
    
    emit dataChanged(index(row, 0), index(row, CARDDBMODEL_COLUMNS - 1));
}

void CardDatabaseModel::cardAdded(CardInfo *card)
{
    beginInsertRows(QModelIndex(), cardList.size(), cardList.size());
    cardList.append(card);
    connect(card, SIGNAL(cardInfoChanged(CardInfo *)), this, SLOT(cardInfoChanged(CardInfo *)));
    endInsertRows();
}

void CardDatabaseModel::cardRemoved(CardInfo *card)
{
    const int row = cardList.indexOf(card);
    if (row == -1)
        return;
    
    beginRemoveRows(QModelIndex(), row, row);
    cardList.removeAt(row);
    endRemoveRows();
}

CardDatabaseDisplayModel::CardDatabaseDisplayModel(QObject *parent)
    : QSortFilterProxyModel(parent),
      isToken(ShowAll)
{
    filterTree = NULL;
    setFilterCaseSensitivity(Qt::CaseInsensitive);
    setSortCaseSensitivity(Qt::CaseInsensitive);

    loadedRowCount = 0;
}

bool CardDatabaseDisplayModel::canFetchMore(const QModelIndex & index) const
{
    return loadedRowCount < sourceModel()->rowCount(index);
}

void CardDatabaseDisplayModel::fetchMore(const QModelIndex & index)
{
    int remainder = sourceModel()->rowCount(index) - loadedRowCount;
    int itemsToFetch = qMin(100, remainder);

    beginInsertRows(QModelIndex(), loadedRowCount, loadedRowCount+itemsToFetch-1);

    loadedRowCount += itemsToFetch;
    endInsertRows();
}

int CardDatabaseDisplayModel::rowCount(const QModelIndex &parent) const
{
    return qMin(QSortFilterProxyModel::rowCount(parent), loadedRowCount);
}

bool CardDatabaseDisplayModel::lessThan(const QModelIndex &left, const QModelIndex &right) const {

    QString leftString = sourceModel()->data(left, CardDatabaseModel::SortRole).toString();
    QString rightString = sourceModel()->data(right, CardDatabaseModel::SortRole).toString();

    if (!cardName.isEmpty() && left.column() == CardDatabaseModel::NameColumn)
    {
        bool isLeftType = leftString.startsWith(cardName, Qt::CaseInsensitive);
        bool isRightType = rightString.startsWith(cardName, Qt::CaseInsensitive);

        // test for an exact match: isLeftType && leftString.size() == cardName.size()
        // or an exclusive start match: isLeftType && !isRightType
        if (isLeftType && (!isRightType || leftString.size() == cardName.size()))
            return true;

        // same checks for the right string
        if (isRightType && (!isLeftType || rightString.size() == cardName.size()))
            return false;
    }
    return QString::localeAwareCompare(leftString, rightString) < 0;
}

bool CardDatabaseDisplayModel::filterAcceptsRow(int sourceRow, const QModelIndex & /*sourceParent*/) const
{
    CardInfo const *info = static_cast<CardDatabaseModel *>(sourceModel())->getCard(sourceRow);
    
    if (((isToken == ShowTrue) && !info->getIsToken()) || ((isToken == ShowFalse) && info->getIsToken()))
        return false;

    if (!cardName.isEmpty() && !info->getName().contains(cardName, Qt::CaseInsensitive))
        return false;

    if (!cardNameSet.isEmpty() && !cardNameSet.contains(info->getName()))
        return false;

    if (filterTree != NULL)
        return filterTree->acceptsCard(info);

    return true;
}

void CardDatabaseDisplayModel::clearFilterAll()
{
    cardName.clear();
    cardText.clear();
    cardTypes.clear();
    cardColors.clear();
    if (filterTree != NULL)
        filterTree->clear();
    invalidateFilter();
}

void CardDatabaseDisplayModel::setFilterTree(FilterTree *filterTree)
{
    if (this->filterTree != NULL)
        disconnect(this->filterTree, 0, this, 0);

    this->filterTree = filterTree;
    connect(this->filterTree, SIGNAL(changed()), this, SLOT(filterTreeChanged()));
    invalidate();
}

void CardDatabaseDisplayModel::filterTreeChanged()
{
    invalidate();
}
