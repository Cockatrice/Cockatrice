#include <QFile>
#include <QTextStream>
#include <QFont>
#include <QBrush>
#include <QTextCursor>
#include <QTextDocument>
#include <QPrinter>
#include <QTextTable>
#include <QProgressDialog>
#include "main.h"
#include "decklistmodel.h"
#include "carddatabase.h"

DeckListModel::DeckListModel(QObject *parent)
	: QAbstractItemModel(parent)
{
	deckList = new DeckList;
	connect(deckList, SIGNAL(deckLoaded()), this, SLOT(rebuildTree()));
	root = new InnerDecklistNode;
}

DeckListModel::~DeckListModel()
{
	delete root;
	delete deckList;
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
			// XXX better sanity checking
			if (!currentCard)
				continue;

			CardInfo *info = db->getCard(currentCard->getName());
			QString cardType;
			if (!info)
				cardType = "unknown";
			else
				cardType = info->getMainCardType();
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
		case 0: return tr("Number");
		case 1: return tr("Card");
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
	if (getNode<DecklistModelCardNode *>(index))
		result |= Qt::ItemIsSelectable;

	return result;
}

void DeckListModel::emitRecursiveUpdates(const QModelIndex &index)
{
	if (!index.isValid())
		return;
	emit dataChanged(index, index);
	emitRecursiveUpdates(index.parent());
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
	emitRecursiveUpdates(index);
	return true;
}

bool DeckListModel::removeRows(int row, int count, const QModelIndex &parent)
{
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
	else
		emitRecursiveUpdates(parent);

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
		QModelIndex parentIndex = nodeToIndex(cardTypeNode);
		beginInsertRows(parentIndex, cardTypeNode->size(), cardTypeNode->size());
		cardNode = new DecklistModelCardNode(decklistCard, cardTypeNode);
		endInsertRows();
		sort(1);
		emitRecursiveUpdates(parentIndex);
		return nodeToIndex(cardNode);
	} else {
		cardNode->setNumber(cardNode->getNumber() + 1);
		QModelIndex ind = nodeToIndex(cardNode);
		emitRecursiveUpdates(ind);
		return ind;
	}
}

QModelIndex DeckListModel::nodeToIndex(AbstractDecklistNode *node) const
{
	if (node == root)
		return QModelIndex();
	return createIndex(node->getParent()->indexOf(node), 0, node);
}

void DeckListModel::sortHelper(InnerDecklistNode *node, Qt::SortOrder order)
{
	// Sort children of node and save the information needed to
	// update the list of persistent indexes.
	QVector<QPair<int, int> > sortResult = node->sort(order);
	
	QModelIndexList from, to;
	for (int i = sortResult.size() - 1; i >= 0; --i) {
		const int fromRow = sortResult[i].first;
		const int toRow = sortResult[i].second;
		AbstractDecklistNode *temp = node->at(toRow);
		for (int j = columnCount(); j; --j) {
			from << createIndex(fromRow, 0, temp);
			to << createIndex(toRow, 0, temp);
		}
	}
	changePersistentIndexList(from, to);
	
	// Recursion
	for (int i = node->size() - 1; i >= 0; --i) {
		InnerDecklistNode *subNode = dynamic_cast<InnerDecklistNode *>(node->at(i));
		if (subNode)
			sortHelper(subNode, order);
	}
}

void DeckListModel::sort(int /*column*/, Qt::SortOrder order)
{
	emit layoutAboutToBeChanged();
	sortHelper(root, order);
	emit layoutChanged();
}

void DeckListModel::cleanList()
{
	root->clearTree();
	deckList->cleanList();
	reset();
}

void DeckListModel::setDeckList(DeckList *_deck)
{
	delete deckList;
	deckList = _deck;
	rebuildTree();
}

void DeckListModel::printDeckListNode(QTextCursor *cursor, InnerDecklistNode *node)
{
	static const int totalColumns = 3;

	if (node->height() == 1) {
		QTextBlockFormat blockFormat;
		QTextCharFormat charFormat;
		charFormat.setFontPointSize(11);
		charFormat.setFontWeight(QFont::Bold);
		cursor->insertBlock(blockFormat, charFormat);
		cursor->insertText(QString("%1: %2").arg(node->getVisibleName()).arg(node->recursiveCount(true)));

		QTextTableFormat tableFormat;
		tableFormat.setCellPadding(0);
		tableFormat.setCellSpacing(0);
		tableFormat.setBorder(0);
		QTextTable *table = cursor->insertTable(node->size() + 1, 2, tableFormat);
		for (int i = 0; i < node->size(); i++) {
			AbstractDecklistCardNode *card = dynamic_cast<AbstractDecklistCardNode *>(node->at(i));

			QTextCharFormat cellCharFormat;
			cellCharFormat.setFontPointSize(9);

			QTextTableCell cell = table->cellAt(i, 0);
			cell.setFormat(cellCharFormat);
			QTextCursor cellCursor = cell.firstCursorPosition();
			cellCursor.insertText(QString("%1 ").arg(card->getNumber()));

			cell = table->cellAt(i, 1);
			cell.setFormat(cellCharFormat);
			cellCursor = cell.firstCursorPosition();
			cellCursor.insertText(card->getName());
		}
	} else if (node->height() == 2) {
		QTextBlockFormat blockFormat;
		QTextCharFormat charFormat;
		charFormat.setFontPointSize(14);
		charFormat.setFontWeight(QFont::Bold);

		cursor->insertBlock(blockFormat, charFormat);
		cursor->insertText(QString("%1: %2").arg(node->getVisibleName()).arg(node->recursiveCount(true)));

		QTextTableFormat tableFormat;
		tableFormat.setCellPadding(10);
		tableFormat.setCellSpacing(0);
		tableFormat.setBorder(0);
		QVector<QTextLength> constraints;
		for (int i = 0; i < totalColumns; i++)
			constraints << QTextLength(QTextLength::PercentageLength, 100.0 / totalColumns);
		tableFormat.setColumnWidthConstraints(constraints);

		QTextTable *table = cursor->insertTable(1, totalColumns, tableFormat);
		for (int i = 0; i < node->size(); i++) {
			QTextCursor cellCursor = table->cellAt(0, (i * totalColumns) / node->size()).lastCursorPosition();
			printDeckListNode(&cellCursor, dynamic_cast<InnerDecklistNode *>(node->at(i)));
		}
	}
	cursor->movePosition(QTextCursor::End);
}

void DeckListModel::printDeckList(QPrinter *printer)
{
	QTextDocument doc;

	QFont font("Serif");
	font.setStyleHint(QFont::Serif);
	doc.setDefaultFont(font);

	QTextCursor cursor(&doc);

	QTextBlockFormat headerBlockFormat;
	QTextCharFormat headerCharFormat;
	headerCharFormat.setFontPointSize(16);
	headerCharFormat.setFontWeight(QFont::Bold);

	cursor.insertBlock(headerBlockFormat, headerCharFormat);
	cursor.insertText(deckList->getName());

	headerCharFormat.setFontPointSize(12);
	cursor.insertBlock(headerBlockFormat, headerCharFormat);
	cursor.insertText(deckList->getComments());
	cursor.insertBlock(headerBlockFormat, headerCharFormat);

	for (int i = 0; i < root->size(); i++) {
		cursor.insertHtml("<br><img src=:/resources/hr.jpg>");
		//cursor.insertHtml("<hr>");
		cursor.insertBlock(headerBlockFormat, headerCharFormat);

		printDeckListNode(&cursor, dynamic_cast<InnerDecklistNode *>(root->at(i)));
	}

	doc.print(printer);
}
