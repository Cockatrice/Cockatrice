#include <QFile>
#include <QTextStream>
#include <QFont>
#include <QBrush>
#include "decklistmodel.h"
#include "carddatabase.h"

DeckListModel::DeckListModel(CardDatabase *_db, QObject *parent)
	: QAbstractItemModel(parent), db(_db)
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

int DeckListModel::rowCount(const QModelIndex &parent) const
{
	if (!parent.isValid()) // parent = root
		return deckList->zoneCount();
	else if (!parent.parent().isValid()) // parent = zone root
		return deckList->getZoneByIndex(parent.row())->size();
	else // parent = card
		return 0;
}

bool DeckListModel::hasChildren(const QModelIndex &parent) const
{
	return !parent.parent().isValid();
}

int DeckListModel::columnCount(const QModelIndex &/*parent*/) const
{
	return 2;
}

QVariant DeckListModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();
	if (index.column() >= 2)
		return QVariant();

	if (!index.parent().isValid()) {
		if (index.row() >= deckList->zoneCount())
			return QVariant();
		switch (role) {
			case Qt::FontRole: {
				QFont f;
				f.setBold(true);
				return f;
			}
			case Qt::DisplayRole:
			case Qt::EditRole: {
				DecklistZone *zone = deckList->getZoneByIndex(index.row());
				switch (index.column()) {
					case 0: return zone->getVisibleName();
					case 1: return QVariant();
					default: return QVariant();
				}
			}
			case Qt::BackgroundRole:
				return QBrush(QColor(200, 255, 200));
			default: return QVariant();
		}
	} else {
		DecklistZone *zone = deckList->getZoneByIndex(index.parent().row());
		if (index.row() >= zone->size())
			return QVariant();
		switch (role) {
			case Qt::DisplayRole:
			case Qt::EditRole: {
				DecklistRow *r = zone->at(index.row());
				switch (index.column()) {
					case 0: return r->getNumber();
					case 1: return r->getCard();
					default: return QVariant();
				}
			}
			case Qt::BackgroundRole: {
				int color = 255 - (index.row() % 2) * 30;
				return QBrush(QColor(color, color, color));
			}
			default: return QVariant();
		}
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

QModelIndex DeckListModel::index(int row, int column, const QModelIndex &parent) const
{
	int id;
	if (!parent.isValid())
		id = -((row + 1) * 1000 + column);
	else
		id = parent.row() * 1000000 + row * 1000 + column;
	return createIndex(row, column, id);
}

QModelIndex DeckListModel::parent(const QModelIndex &ind) const
{
	if ((int) ind.internalId() < 0)
		return QModelIndex();
	else
		return index(ind.internalId() / 1000000, 0);
}

Qt::ItemFlags DeckListModel::flags(const QModelIndex &index) const
{
	Qt::ItemFlags result = Qt::ItemIsEnabled;
	if (index.parent().isValid()) {
		result |= Qt::ItemIsSelectable;
		if (index.column() == 0)
			result |= Qt::ItemIsEditable;
	}
	return result;
}

bool DeckListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (!index.isValid() || !index.parent().isValid() || role != Qt::EditRole)
		return false;

	switch (index.column()) {
		case 0: deckList->getZoneByIndex(index.parent().row())->at(index.row())->setNumber(value.toInt()); break;
		case 1: deckList->getZoneByIndex(index.parent().row())->at(index.row())->setCard(value.toString()); break;
		default: return false;
	}
	emit dataChanged(index, index);
	return true;
}

bool DeckListModel::removeRows(int row, int count, const QModelIndex &parent)
{
	// Inserting zones is not supported.
	if (!parent.isValid())
		return false;

	beginRemoveRows(parent, row, row + count - 1);

	for (int i = 0; i < count; i++)
		deckList->getZoneByIndex(parent.row())->removeAt(row);

	endRemoveRows();
	return true;
}

bool DeckListModel::insertRows(int row, int count, const QModelIndex &parent)
{
	// Inserting zones is not supported.
	if (!parent.isValid())
		return false;

	beginInsertRows(parent, row, row + count - 1);

	for (int i = 0; i < count; i++)
		deckList->getZoneByIndex(parent.row())->insert(row, new DecklistRow);

	endInsertRows();
	return true;
}

void DeckListModel::cleanList()
{
	deckList->cleanList();
	deckList->initZones();
	reset();
}
