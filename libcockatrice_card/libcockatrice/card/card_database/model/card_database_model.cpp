#include "card_database_model.h"

#include "../card_database.h"

#include <QMap>

#define CARDDBMODEL_COLUMNS 6

CardDatabaseModel::CardDatabaseModel(CardDatabase *_db, bool _showOnlyCardsFromEnabledSets, QObject *parent)
    : QAbstractListModel(parent), db(_db), showOnlyCardsFromEnabledSets(_showOnlyCardsFromEnabledSets)
{
    connect(db, &CardDatabase::cardAdded, this, &CardDatabaseModel::cardAdded);
    connect(db, &CardDatabase::cardRemoved, this, &CardDatabaseModel::cardRemoved);
    connect(db, &CardDatabase::cardDatabaseEnabledSetsChanged, this,
            &CardDatabaseModel::cardDatabaseEnabledSetsChanged);

    cardDatabaseEnabledSetsChanged();
}

CardDatabaseModel::~CardDatabaseModel() = default;

int CardDatabaseModel::rowCount(const QModelIndex & /*parent*/) const
{
    return cardList.size();
}

int CardDatabaseModel::columnCount(const QModelIndex & /*parent*/) const
{
    return CARDDBMODEL_COLUMNS;
}

QVariant CardDatabaseModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= cardList.size() || index.column() >= CARDDBMODEL_COLUMNS ||
        (role != Qt::DisplayRole && role != SortRole))
        return QVariant();

    CardInfoPtr card = cardList.at(index.row());
    switch (index.column()) {
        case NameColumn:
            return card->getName();
        case SetListColumn:
            return card->getSetsNames();
        case ManaCostColumn:
            return role == SortRole ? QString("%1%2").arg(card->getCmc(), 4, QChar('0')).arg(card->getManaCost())
                                    : card->getManaCost();
        case CardTypeColumn:
            return card->getCardType();
        case PTColumn:
            return card->getPowTough();
        case ColorColumn:
            return card->getColors();
        default:
            return QVariant();
    }
}

QVariant CardDatabaseModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();
    if (orientation != Qt::Horizontal)
        return QVariant();
    switch (section) {
        case NameColumn:
            return QString(tr("Name"));
        case SetListColumn:
            return QString(tr("Sets"));
        case ManaCostColumn:
            return QString(tr("Mana cost"));
        case CardTypeColumn:
            return QString(tr("Card type"));
        case PTColumn:
            return QString(tr("P/T"));
        case ColorColumn:
            return QString(tr("Color(s)"));
        default:
            return QVariant();
    }
}

void CardDatabaseModel::cardInfoChanged(CardInfoPtr card)
{
    const int row = cardList.indexOf(card);
    if (row == -1)
        return;

    emit dataChanged(index(row, 0), index(row, CARDDBMODEL_COLUMNS - 1));
}

bool CardDatabaseModel::checkCardHasAtLeastOneEnabledSet(CardInfoPtr card)
{
    if (!showOnlyCardsFromEnabledSets)
        return true;

    for (const auto &printings : card->getSets()) {
        for (const auto &printing : printings) {
            if (printing.getSet()->getEnabled())
                return true;
        }
    }

    return false;
}

void CardDatabaseModel::cardDatabaseEnabledSetsChanged()
{
    // remove all the cards no more present in at least one enabled set
    for (const CardInfoPtr &card : cardList) {
        if (!checkCardHasAtLeastOneEnabledSet(card)) {
            cardRemoved(card);
        }
    }

    // re-check all the card currently not shown, maybe their part of a newly-enabled set
    for (const CardInfoPtr &card : db->getCardList()) {
        if (!cardListSet.contains(card)) {
            cardAdded(card);
        }
    }
}

void CardDatabaseModel::cardAdded(CardInfoPtr card)
{
    if (checkCardHasAtLeastOneEnabledSet(card)) {
        // add the card if it's present in at least one enabled set
        beginInsertRows(QModelIndex(), cardList.size(), cardList.size());
        cardList.append(card);
        cardListSet.insert(card);
        connect(card.data(), &CardInfo::cardInfoChanged, this, &CardDatabaseModel::cardInfoChanged);
        endInsertRows();
    }
}

void CardDatabaseModel::cardRemoved(CardInfoPtr card)
{
    const int row = cardList.indexOf(card);
    if (row == -1) {
        return;
    }

    beginRemoveRows(QModelIndex(), row, row);
    disconnect(card.data(), nullptr, this, nullptr);
    cardListSet.remove(card);
    card.clear();
    cardList.removeAt(row);
    endRemoveRows();
}
