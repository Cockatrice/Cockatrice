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
    if (!index.isValid())
        return QVariant();
    if ((index.row() >= cardList.size()) || (index.column() >= CARDDBMODEL_COLUMNS))
        return QVariant();
    if (role != Qt::DisplayRole)
        return QVariant();

    CardInfo *card = cardList.at(index.row());
    switch (index.column()){
        case NameColumn: return card->getName();
        case SetListColumn: {
            QStringList setList;
            const QList<CardSet *> &sets = card->getSets();
            for (int i = 0; i < sets.size(); i++)
                setList << sets[i]->getShortName();
            return setList.join(", ");
        }
        case ManaCostColumn: return card->getManaCost();
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
    
    cardList = db->getCardList();
    for (int i = 0; i < cardList.size(); ++i)
        connect(cardList[i], SIGNAL(cardInfoChanged(CardInfo *)), this, SLOT(cardInfoChanged(CardInfo *)));
    
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
}


bool CardDatabaseDisplayModel::lessThan(const QModelIndex &left, const QModelIndex &right) const {

    QString leftString = sourceModel()->data(left).toString();
    QString rightString = sourceModel()->data(right).toString();

    if (!cardName.isEmpty() && left.column() == CardDatabaseModel::NameColumn)
    {
        if (leftString.compare(cardName, Qt::CaseInsensitive) == 0) {// exact match should be at top
            return true;
        }

        if (rightString.compare(cardName, Qt::CaseInsensitive) == 0) {// exact match should be at top
            return false;
        }

        bool isLeftType2 = leftString.startsWith(cardName, Qt::CaseInsensitive);
        bool isRightType2 = rightString.startsWith(cardName, Qt::CaseInsensitive);
        if (isLeftType2 && !isRightType2)
            return true;
        if (isRightType2 && !isLeftType2)
            return false;
    }
    return QString::localeAwareCompare(leftString, rightString) < 0;

}

bool CardDatabaseDisplayModel::filterAcceptsRow(int sourceRow, const QModelIndex & /*sourceParent*/) const
{
    CardInfo const *info = static_cast<CardDatabaseModel *>(sourceModel())->getCard(sourceRow);
    
    if (((isToken == ShowTrue) && !info->getIsToken()) || ((isToken == ShowFalse) && info->getIsToken()))
        return false;

    if (!cardName.isEmpty())
        if (!info->getName().contains(cardName, Qt::CaseInsensitive)) 
            if (!CardInfo::simplifyName(info->getName()).contains(cardName, Qt::CaseInsensitive))
                return false;

    if (!cardNameSet.isEmpty())
            if (!cardNameSet.contains(info->getName()))
                return false;

    if (filterTree != NULL)
        return filterTree->acceptsCard(info);

    return true;
}

void CardDatabaseDisplayModel::clearSearch()
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
