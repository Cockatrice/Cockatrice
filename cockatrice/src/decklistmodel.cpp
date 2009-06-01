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
	connect(deckList, SIGNAL(deckLoaded()), this, SLOT(rebuildTree()));
	root = new InnerDecklistNode;
}

DeckListModel::~DeckListModel()
{
}

void DeckListModel::debugIndexInfo(const QString &func, const QModelIndex &index) const
{
	quint32 id = index.internalId();	// 32 bit int, from MSB to LSB:
	int zone = id >> 30;			// 2 bits - zone
	int cardtype = (id >> 25) & 0x1f;	// 5 bits - card type
	int card = (id >> 3) & 0x3fffff;	// 22 bits - card
	int column = id & 0x7;			// 3 bits - column

	if (index.isValid())
		qDebug(QString("index: function = %1, zone = %2, cardtype = %3, card = %4, column = %5").arg(func).arg(zone).arg(cardtype).arg(card).arg(column).toLatin1());
	else
		qDebug(QString("index: function = %1, invalid").arg(func).toLatin1());
}

void DeckListModel::debugShowTree(InnerDecklistNode *node, int depth) const
{
	for (int i = 0; i < node->size(); i++) {
		DecklistModelCardNode *foo = dynamic_cast<DecklistModelCardNode *>(node->at(i));
		if (!foo) {
			InnerDecklistNode *bar = dynamic_cast<InnerDecklistNode *>(node->at(i));
			qDebug(QString("%1%2").arg(QString(depth * 4, ' ')).arg(bar->getName()).toLatin1());
			debugShowTree(bar, depth + 1);
		} else
			qDebug(QString("%1%2 %3").arg(QString(depth * 4, ' ')).arg(foo->getNumber()).arg(foo->getName()).toLatin1());
	}
}

void DeckListModel::rebuildTree()
{
	root->clearTree();
	InnerDecklistNode *listRoot = deckList->getRoot();
	for (int i = 0; i < listRoot->size(); i++) {
		InnerDecklistNode *currentZone = dynamic_cast<InnerDecklistNode *>(listRoot->at(i));
		InnerDecklistNode *node = new InnerDecklistNode(currentZone->getName(), root);
		for (int j = 0; j < currentZone->size(); j++) {
			DecklistCardNode *currentCard = dynamic_cast<DecklistCardNode *>(currentZone->at(j));

			QString cardType = db->getCard(currentCard->getName())->getMainCardType();
			InnerDecklistNode *cardTypeNode = dynamic_cast<InnerDecklistNode *>(findNode(cardType, node));
			if (!cardTypeNode)
				cardTypeNode = new InnerDecklistNode(cardType, node);
			
			DecklistModelCardNode *newCard = new DecklistModelCardNode(currentCard, cardTypeNode);
		}
	}
	debugShowTree(root, 0);
	
	reset();
}

AbstractDecklistNode *DeckListModel::findNode(const QString &name, InnerDecklistNode *root) const
{
	for (int i = 0; i < root->size(); i++)
		if (root->at(i)->getName() == name)
			return root->at(i);
	return 0;
}

AbstractDecklistNode *DeckListModel::findNode(const QModelIndex &index) const
{
//	debugIndexInfo("findNode", index);
	if (index.isValid()) {
		InnerDecklistNode *parentNode = dynamic_cast<InnerDecklistNode *>(findNode(index.parent()));
		return parentNode->at(index.row());
	} else
		return root;
}

int DeckListModel::rowCount(const QModelIndex &parent) const
{
//	debugIndexInfo("rowCount", parent);
	InnerDecklistNode *node = dynamic_cast<InnerDecklistNode *>(findNode(parent));
	if (node) {
//		qDebug(QString("  rowCount: return %1").arg(node->size()).toLatin1());
		return node->size();
	} else {
//		qDebug("  rowCount: return const 0");
		return 0;
	}
}

bool DeckListModel::hasChildren(const QModelIndex &parent) const
{
//	debugIndexInfo("hasChildren", parent);
	if (!parent.isValid() && root->size())
		return true;
	if (parent.column() != 0)
		return false;
/*	
	InnerDecklistNode *node = dynamic_cast<InnerDecklistNode *>(findNode(parent));
	if (!node)
		qDebug("line 109: return false");
		return false;
	}
	qDebug(QString("line 112: return %1").arg(node->size()).toLatin1());
	return node->size();

	if (!parent.isValid())
		return true;
	qDebug(QString("  hasChildren: return %1").arg((!parent.column() && (((parent.internalId() >> 3) & 0x3fffff) == 0x3fffff))).toLatin1());
*/	// An item (possibly) has children if its column is zero and its card is -1.
	return (!parent.column() && (((parent.internalId() >> 3) & 0x3fffff) == 0x3fffff));
}

int DeckListModel::columnCount(const QModelIndex &/*parent*/) const
{
	return 2;
}

QVariant DeckListModel::data(const QModelIndex &index, int role) const
{
//	debugIndexInfo("data", index);
	if (!index.isValid())
		return QVariant();
	if (index.column() >= 2)
		return QVariant();

	AbstractDecklistNode *tempNode = findNode(index);
	if (tempNode == root)
		return QVariant();
		
	if (tempNode->hasChildren()) {
		switch (role) {
			case Qt::FontRole: {
				QFont f;
				f.setBold(true);
				return f;
			}
			case Qt::DisplayRole:
			case Qt::EditRole: {
				switch (index.column()) {
					case 0: {
						InnerDecklistNode *node = dynamic_cast<InnerDecklistNode *>(tempNode);
						return node->getVisibleName();
					}
					default: return QVariant();
				}
			}
			case Qt::BackgroundRole:
				return QBrush(QColor(200, 255, 200));
			default: return QVariant();
		}
	} else {
		switch (role) {
			case Qt::DisplayRole:
			case Qt::EditRole: {
				DecklistModelCardNode *node = dynamic_cast<DecklistModelCardNode *>(tempNode);
				switch (index.column()) {
					case 0: return node->getNumber();
					case 1: return node->getName();
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
//	debugIndexInfo("index", parent);
	// for explanation of the bit shifting, look at parent()
	int indexZone, indexCardType, indexCard;
	if (!parent.isValid()) {
		indexZone = row;
		indexCardType = 0x1f;
		indexCard = 0x3fffff;
	} else {
		quint32 pid = parent.internalId();
		indexZone = pid >> 30;
		int pcardtype = (pid >> 25) & 0x1f;
		
		if (pcardtype == 0x1f) {
			indexCardType = row;
			indexCard = 0x3fffff;
		} else {
			indexCardType = pcardtype;
			indexCard = row;
		}
	}

//	qDebug(QString("index(): zone = %1, cardtype = %2, card = %3, column = %4").arg(indexZone).arg(indexCardType).arg(indexCard).arg(column).toLatin1());
	return createIndex(row, column, (indexZone << 30) + (indexCardType << 25) + (indexCard << 3) + column);
}

QModelIndex DeckListModel::parent(const QModelIndex &ind) const
{
//	debugIndexInfo("parent", ind);
		
	quint32 id = ind.internalId();		// 32 bit int, from MSB to LSB:
	int zone = id >> 30;			// 2 bits - zone
	int cardtype = (id >> 25) & 0x1f;	// 5 bits - card type
	int card = (id >> 3) & 0x3fffff;	// 22 bits - card
//	int column = id & 0x7;			// 3 bits - column
	
	if (cardtype == 0x1f)
		return QModelIndex();
	else if (card == 0x3fffff)
		return index(zone, 0);
	else
		return index(cardtype, 0, index(zone, 0));
}

Qt::ItemFlags DeckListModel::flags(const QModelIndex &index) const
{
	Qt::ItemFlags result = Qt::ItemIsEnabled;
	if (((index.internalId() >> 3) & 0x3fffff) != 0x3fffff) {
		result |= Qt::ItemIsSelectable;
		if (index.column() == 0)
			result |= Qt::ItemIsEditable;
	}
	return result;
}

bool DeckListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	DecklistModelCardNode *node = dynamic_cast<DecklistModelCardNode *>(findNode(index));
	if (!node || (role != Qt::EditRole))
		return false;

	switch (index.column()) {
		case 0: node->setNumber(value.toInt()); break;
		case 1: node->setName(value.toString()); break;
		default: return false;
	}
	emit dataChanged(index, index);
	return true;
}

bool DeckListModel::removeRows(int row, int count, const QModelIndex &parent)
{
/*	DecklistNode *node = findNode(parent);
	if (row + count > node->size())
		return false;

	beginRemoveRows(parent, row, row + count - 1);

	for (int i = 0; i < count; i++)
		delete node->takeAt(row);

	endRemoveRows();
	return true;
*/}
/*
void DeckListModel::insertCard(...)
{
}

void DeckListModel::removeCard(...)
{
}
*/
bool DeckListModel::insertRows(int row, int count, const QModelIndex &parent)
{
/*	
	// Inserting zones is not supported.
	if (!parent.isValid())
		return false;

	beginInsertRows(parent, row, row + count - 1);

	for (int i = 0; i < count; i++)
		deckList->getZoneByIndex(parent.row())->insert(row, new DecklistRow);

	endInsertRows();
	return true;
*/}

void DeckListModel::cleanList()
{
	deckList->cleanList();
	deckList->initZones();
	reset();
}
