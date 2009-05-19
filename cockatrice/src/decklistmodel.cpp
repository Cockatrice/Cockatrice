#include <QFile>
#include <QTextStream>
#include "decklistmodel.h"
#include "carddatabase.h"

DeckListModel::DeckListModel(CardDatabase *_db, QObject *parent)
	: QAbstractListModel(parent), db(_db)
{
	deckList = new DeckList(db);
}

DeckListModel::~DeckListModel()
{
	delete deckList;
}

int DeckListModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return deckList->size();
}

int DeckListModel::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return 2;
}

QVariant DeckListModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();
	if ((index.row() >= deckList->size()) || (index.column() >= 2))
		return QVariant();

	if (role != Qt::DisplayRole)
		return QVariant();

	DecklistRow *r = deckList->at(index.row());
	switch (index.column()) {
		case 0: return r->getNumber();
		case 1: return r->getCard();
		default: return QVariant();
	}
}

QVariant DeckListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole)
		return QVariant();
	if (orientation != Qt::Horizontal)
		return QVariant();
	switch (section) {
		case 0: return QString(tr("Number"));
		case 1: return QString(tr("Card"));
		default: return QVariant();
	}
}

void DeckListModel::cleanList()
{
	deckList->cleanList();
	reset();
}

DecklistRow *DeckListModel::getRow(int row) const
{
	if (row >= deckList->size())
		return 0;
	return deckList->at(row);
}
