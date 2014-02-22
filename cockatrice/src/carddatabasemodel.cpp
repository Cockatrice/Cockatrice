#include "carddatabasemodel.h"

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
    return 5;
}

QVariant CardDatabaseModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if ((index.row() >= cardList.size()) || (index.column() >= 5))
        return QVariant();
    if (role != Qt::DisplayRole)
        return QVariant();

    CardInfo *card = cardList.at(index.row());
    switch (index.column()){
        case 0: return card->getName();
        case 1: {
            QStringList setList;
            const QList<CardSet *> &sets = card->getSets();
            for (int i = 0; i < sets.size(); i++)
                setList << sets[i]->getShortName();
            return setList.join(", ");
        }
        case 2: return card->getManaCost();
        case 3: return card->getCardType();
        case 4: return card->getPowTough();
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
        case 0: return QString(tr("Name"));
        case 1: return QString(tr("Sets"));
        case 2: return QString(tr("Mana cost"));
        case 3: return QString(tr("Card type"));
        case 4: return QString(tr("P/T"));
        default: return QVariant();
    }
}

void CardDatabaseModel::updateCardList()
{
    for (int i = 0; i < cardList.size(); ++i)
        disconnect(cardList[i], 0, this, 0);
    
    cardList = db->getCardList();
    for (int i = 0; i < cardList.size(); ++i)
        connect(cardList[i], SIGNAL(cardInfoChanged(CardInfo *)), this, SLOT(cardInfoChanged(CardInfo *)));
    
    reset();
}

void CardDatabaseModel::cardInfoChanged(CardInfo *card)
{
    const int row = cardList.indexOf(card);
    if (row == -1)
        return;
    
    emit dataChanged(index(row, 0), index(row, 4));
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
    setFilterCaseSensitivity(Qt::CaseInsensitive);
    setSortCaseSensitivity(Qt::CaseInsensitive);
}

bool CardDatabaseDisplayModel::filterAcceptsRow(int sourceRow, const QModelIndex & /*sourceParent*/) const
{
    CardInfo const *info = static_cast<CardDatabaseModel *>(sourceModel())->getCard(sourceRow);
    
    if (((isToken == ShowTrue) && !info->getIsToken()) || ((isToken == ShowFalse) && info->getIsToken()))
        return false;
    
    if (!cardNameBeginning.isEmpty())
        if (!info->getName().startsWith(cardNameBeginning, Qt::CaseInsensitive))
            return false;
    
    if (!cardName.isEmpty())
        if (!info->getName().contains(cardName, Qt::CaseInsensitive))
            return false;
    
    if (!cardNameSet.isEmpty())
        if (!cardNameSet.contains(info->getName()))
            return false;
    
    if (!cardText.isEmpty())
        if (!info->getText().contains(cardText, Qt::CaseInsensitive))
            return false;
    
    if (!cardColors.isEmpty())
        if (QSet<QString>::fromList(info->getColors()).intersect(cardColors).isEmpty() && !(info->getColors().isEmpty() && cardColors.contains("X")))
            return false;
    
    if (!cardTypes.isEmpty())
        if (!cardTypes.contains(info->getMainCardType()))
            return false;

    return true;
}

void CardDatabaseDisplayModel::clearSearch()
{
    cardName.clear();
    cardText.clear();
    cardTypes.clear();
    cardColors.clear();
    invalidateFilter();
}
