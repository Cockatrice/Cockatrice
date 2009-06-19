#include "carddatabasemodel.h"

CardDatabaseModel::CardDatabaseModel(CardDatabase *_db, QObject *parent)
	: QAbstractListModel(parent), db(_db)
{
	cardList = db->getCardList();
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
			QList<CardSet *> sets = card->getSets();
			for (int i = 0; i < sets.size(); i++)
				setList << sets[i]->getShortName();
			return setList.join(", ");
		}
		case 2: return card->getManacost();
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

class CardInfoCompare {
private:
	int column;
	Qt::SortOrder order;
public:
	CardInfoCompare(int _column, Qt::SortOrder _order) : column(_column), order(_order) { }
	inline bool operator()(CardInfo *a, CardInfo *b) const
	{
		bool result;
		switch (column) {
			case 0: result = (a->getName() < b->getName()); break;
			case 1: result = (a->getSets().at(0)->getShortName() < b->getSets().at(0)->getShortName()); break;
			case 2: result = (a->getManacost() < b->getManacost()); break;
			case 3: result = (a->getCardType() < b->getCardType()); break;
			case 4: result = (a->getPowTough() < b->getPowTough()); break;
			default: result = false;
		}
		return (order == Qt::AscendingOrder) ^ result;
	}
};

void CardDatabaseModel::sort(int column, Qt::SortOrder order)
{
	CardInfoCompare cmp(column, order);
	qSort(cardList.begin(), cardList.end(), cmp);
}
