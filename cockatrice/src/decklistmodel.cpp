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
	if (!index.isValid())
		qDebug(QString("debugIndexInfo: %1: index invalid").arg(func).toLatin1());
	else if (InnerDecklistNode *node = getNode<InnerDecklistNode *>(index))
		qDebug(QString("debugIndexInfo: %1: INNER index '%2', row=%3, col=%4").arg(func).arg(node->getName()).arg(index.row()).arg(index.column()).toLatin1());
	else if (DecklistModelCardNode *node = getNode<DecklistModelCardNode *>(index))
		qDebug(QString("debugIndexInfo: %1: CARD index '%2', row=%3, col=%4").arg(func).arg(node->getName()).arg(index.row()).arg(index.column()).toLatin1());
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
			InnerDecklistNode *cardTypeNode = dynamic_cast<InnerDecklistNode *>(node->findChild(cardType));
			if (!cardTypeNode)
				cardTypeNode = new InnerDecklistNode(cardType, node);
			
			new DecklistModelCardNode(currentCard, cardTypeNode);
		}
	}
	
	reset();
}

int DeckListModel::rowCount(const QModelIndex &parent) const
{
//	debugIndexInfo("rowCount", parent);
	InnerDecklistNode *node = getNode<InnerDecklistNode *>(parent);
	if (node)
		return node->size();
	else
		return 0;
}

QVariant DeckListModel::data(const QModelIndex &index, int role) const
{
//	debugIndexInfo("data", index);
	if (!index.isValid())
		return QVariant();
	if (index.column() >= 2)
		return QVariant();

	AbstractDecklistNode *temp = static_cast<AbstractDecklistNode *>(index.internalPointer());
	DecklistModelCardNode *card = dynamic_cast<DecklistModelCardNode *>(temp);
	if (!card) {
		InnerDecklistNode *node = dynamic_cast<InnerDecklistNode *>(temp);
		switch (role) {
			case Qt::FontRole: {
				QFont f;
				f.setBold(true);
				return f;
			}
			case Qt::DisplayRole:
			case Qt::EditRole:
				switch (index.column()) {
					case 0: return node->recursiveCount(true);
					case 1: return node->getVisibleName();
					default: return QVariant();
				}
			case Qt::BackgroundRole: {
				int color = 90 + 60 * node->depth();
				return QBrush(QColor(color, 255, color));
			}
			default: return QVariant();
		}
	} else {
		switch (role) {
			case Qt::DisplayRole:
			case Qt::EditRole: {
				switch (index.column()) {
					case 0: return card->getNumber();
					case 1: return card->getName();
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
	if ((role != Qt::DisplayRole) || (orientation != Qt::Horizontal))
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
	if (!hasIndex(row, column, parent))
		return QModelIndex();
	
	InnerDecklistNode *parentNode = getNode<InnerDecklistNode *>(parent);
	if (row >= parentNode->size())
		return QModelIndex();
		
	return createIndex(row, column, parentNode->at(row));
}

QModelIndex DeckListModel::parent(const QModelIndex &ind) const
{
	if (!ind.isValid())
		return QModelIndex();
	
	return nodeToIndex(static_cast<AbstractDecklistNode *>(ind.internalPointer())->getParent());
}

Qt::ItemFlags DeckListModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return 0;

	Qt::ItemFlags result = Qt::ItemIsEnabled;
	if (getNode<DecklistModelCardNode *>(index)) {
		result |= Qt::ItemIsSelectable;
		if (index.column() == 0)
			result |= Qt::ItemIsEditable;
	}
	return result;
}

bool DeckListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	DecklistModelCardNode *node = getNode<DecklistModelCardNode *>(index);
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
	debugIndexInfo("removeRows", parent);
	InnerDecklistNode *node = getNode<InnerDecklistNode *>(parent);
	if (!node)
		return false;
	if (row + count > node->size())
		return false;

	beginRemoveRows(parent, row, row + count - 1);
	for (int i = 0; i < count; i++) {
		AbstractDecklistNode *toDelete = node->takeAt(row);
		if (DecklistModelCardNode *temp = dynamic_cast<DecklistModelCardNode *>(toDelete))
			deckList->deleteNode(temp->getDataNode());
		delete toDelete;
	}
	endRemoveRows();
	
	if (!node->size() && (node != root))
		removeRows(parent.row(), 1, parent.parent());

	return true;
}

InnerDecklistNode *DeckListModel::createNodeIfNeeded(const QString &name, InnerDecklistNode *parent)
{
	InnerDecklistNode *newNode = dynamic_cast<InnerDecklistNode *>(parent->findChild(name));
	if (!newNode) {
		beginInsertRows(nodeToIndex(parent), parent->size(), parent->size());
		newNode = new InnerDecklistNode(name, parent);
		endInsertRows();
	}
	return newNode;
}

QModelIndex DeckListModel::addCard(const QString &cardName, const QString &zoneName)
{
	InnerDecklistNode *zoneNode = createNodeIfNeeded(zoneName, root);
	
	CardInfo *info = db->getCard(cardName);
	QString cardType = info->getMainCardType();
	InnerDecklistNode *cardTypeNode = createNodeIfNeeded(cardType, zoneNode);
	
	DecklistModelCardNode *cardNode = dynamic_cast<DecklistModelCardNode *>(cardTypeNode->findChild(cardName));
	if (!cardNode) {
		DecklistCardNode *decklistCard = deckList->addCard(cardName, zoneName);
		beginInsertRows(nodeToIndex(cardTypeNode), cardTypeNode->size(), cardTypeNode->size());
		cardNode = new DecklistModelCardNode(decklistCard, cardTypeNode);
		endInsertRows();
		sort(1);
		return nodeToIndex(cardNode);
	} else {
		cardNode->setNumber(cardNode->getNumber() + 1);
		QModelIndex ind = nodeToIndex(cardNode);
		emit dataChanged(ind, ind);
		return ind;
	}
}

QModelIndex DeckListModel::nodeToIndex(AbstractDecklistNode *node) const
{
	if (node == root)
		return QModelIndex();
	return createIndex(node->getParent()->indexOf(node), 0, node);
}


void DeckListModel::sort(int /*column*/, Qt::SortOrder order)
{
	emit layoutAboutToBeChanged();
	root->sort(order);
	emit layoutChanged();
}

void DeckListModel::cleanList()
{
	deckList->cleanList();
	reset();
}
