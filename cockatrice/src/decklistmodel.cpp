#include <QFile>
#include <QTextStream>
#include "decklistmodel.h"
#include "carddatabase.h"

DeckListModel::DeckListModel(CardDatabase *_db, QObject *parent)
	: QAbstractListModel(parent), db(_db)
{
	deckList = new DeckList(db, this);
	connect(deckList, SIGNAL(deckLoaded()), this, SLOT(resetModel()));
}

DeckListModel::~DeckListModel()
{
}

void DeckListModel::resetModel()
{
	reset();
}

int DeckListModel::rowCount(const QModelIndex &/*parent*/) const
{
//	qDebug(QString("rowCount = %1").arg(deckList->size()).toLatin1());
	return deckList->size();
}

int DeckListModel::columnCount(const QModelIndex &/*parent*/) const
{
	return 2;
}

QVariant DeckListModel::data(const QModelIndex &index, int role) const
{
//	qDebug(QString("data() called: index.row = %1, column = %2, role = %3").arg(index.row()).arg(index.column()).arg(role).toLatin1());
	if (!index.isValid())
		return QVariant();
	if ((index.row() >= deckList->size()) || (index.column() >= 2))
		return QVariant();

	if ((role != Qt::DisplayRole) && (role != Qt::EditRole))
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
//	qDebug(QString("headerData() called: section = %1, orientation = %2, role = %3").arg(section).arg(orientation).arg(role).toLatin1());
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

Qt::ItemFlags DeckListModel::flags(const QModelIndex &index) const
{
	if (index.column() == 0)
		return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
	else
		return QAbstractItemModel::flags(index);
}

bool DeckListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (!index.isValid() || role != Qt::EditRole)
		return false;

	switch (index.column()) {
		case 0: deckList->at(index.row())->setNumber(value.toInt()); break;
		case 1: deckList->at(index.row())->setCard(value.toString()); break;
		default: return false;
	}
	emit dataChanged(index, index);
	return true;
}

bool DeckListModel::removeRows(int row, int count, const QModelIndex &parent)
{
	beginRemoveRows(parent, row, row + count - 1);

	for (int i = 0; i < count; i++)
		deckList->removeAt(row);

	endRemoveRows();
	return true;
}

bool DeckListModel::insertRows(int row, int count, const QModelIndex &parent)
{
	beginInsertRows(parent, row, row + count - 1);

	for (int i = 0; i < count; i++)
		deckList->insert(row, new DecklistRow);

	endInsertRows();
	return true;
}

void DeckListModel::cleanList()
{
	deckList->cleanList();
	reset();
}
