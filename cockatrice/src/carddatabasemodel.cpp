#include "carddatabasemodel.h"
#include "filtertree.h"

#define CARDDBMODEL_COLUMNS 6

CardDatabaseModel::CardDatabaseModel(CardDatabase *_db, QObject *parent)
    : QAbstractListModel(parent), db(_db)
{
    connect(db, SIGNAL(cardAdded(CardInfo *)), this, SLOT(cardAdded(CardInfo *)));
    connect(db, SIGNAL(cardRemoved(CardInfo *)), this, SLOT(cardRemoved(CardInfo *)));
    connect(db, SIGNAL(cardDatabaseEnabledSetsChanged()), this, SLOT(cardDatabaseEnabledSetsChanged()));

    cardDatabaseEnabledSetsChanged();
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
        case ColorColumn: return card->getColors().join("");
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
        case ColorColumn: return QString(tr("Color(s)"));
        default: return QVariant();
    }
}

void CardDatabaseModel::cardInfoChanged(CardInfo *card)
{
    const int row = cardList.indexOf(card);
    if (row == -1)
        return;
    
    emit dataChanged(index(row, 0), index(row, CARDDBMODEL_COLUMNS - 1));
}

bool CardDatabaseModel::checkCardHasAtLeastOneEnabledSet(CardInfo *card)
{
    foreach(CardSet * set, card->getSets())
    {
        if(set->getEnabled())
            return true;
    }

    return false;
}

void CardDatabaseModel::cardDatabaseEnabledSetsChanged()
{
    // remove all the cards no more present in at least one enabled set
    foreach(CardInfo * card, cardList)
    {
        if(!checkCardHasAtLeastOneEnabledSet(card))
            cardRemoved(card);
    }

    // re-check all the card currently not shown, maybe their part of a newly-enabled set
    foreach(CardInfo * card, db->getCardList())
    {
        if(!cardList.contains(card))
            cardAdded(card);
    }
}

void CardDatabaseModel::cardAdded(CardInfo *card)
{
    if(checkCardHasAtLeastOneEnabledSet(card))
    {
        // add the card if it's present in at least one enabled set
        beginInsertRows(QModelIndex(), cardList.size(), cardList.size());
        cardList.append(card);
        connect(card, SIGNAL(cardInfoChanged(CardInfo *)), this, SLOT(cardInfoChanged(CardInfo *)));
        endInsertRows();
    }
}

void CardDatabaseModel::cardRemoved(CardInfo *card)
{
    const int row = cardList.indexOf(card);
    if (row == -1)
        return;
    
    beginRemoveRows(QModelIndex(), row, row);
    disconnect(card, 0, this, 0);
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

    return rowMatchesCardName(info);
}

bool CardDatabaseDisplayModel::rowMatchesCardName(CardInfo const *info) const {
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

TokenDisplayModel::TokenDisplayModel(QObject *parent)
    : CardDatabaseDisplayModel(parent)
{

}

bool TokenDisplayModel::filterAcceptsRow(int sourceRow, const QModelIndex & /*sourceParent*/) const
{
    CardInfo const *info = static_cast<CardDatabaseModel *>(sourceModel())->getCard(sourceRow);
    return info->getIsToken() && rowMatchesCardName(info);
}

int TokenDisplayModel::rowCount(const QModelIndex &parent) const
{
    // always load all tokens at start
    return QSortFilterProxyModel::rowCount(parent);
}
