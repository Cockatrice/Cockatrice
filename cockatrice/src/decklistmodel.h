#ifndef DECKLISTMODEL_H
#define DECKLISTMODEL_H

#include <QAbstractItemModel>
#include <QList>
#include "decklist.h"

class CardDatabase;
class QProgressDialog;
class QPrinter;
class QTextCursor;

class DecklistModelCardNode : public AbstractDecklistCardNode {
private:
	DecklistCardNode *dataNode;
public:
	DecklistModelCardNode(DecklistCardNode *_dataNode, InnerDecklistNode *_parent) : AbstractDecklistCardNode(_parent), dataNode(_dataNode) { }
	int getNumber() const { return dataNode->getNumber(); }
	void setNumber(int _number) { dataNode->setNumber(_number); }
	QString getName() const { return dataNode->getName(); }
	void setName(const QString &_name) { dataNode->setName(_name); }
	DecklistCardNode *getDataNode() const { return dataNode; }
};

class DeckListModel : public QAbstractItemModel {
	Q_OBJECT
private slots:
	void rebuildTree();
public slots:
	void printDeckList(QPrinter *printer);
public:
	DeckListModel(QObject *parent = 0);
	~DeckListModel();
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &/*parent*/ = QModelIndex()) const { return 2; }
	QVariant data(const QModelIndex &index, int role) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
	QModelIndex parent(const QModelIndex &index) const;
	Qt::ItemFlags flags(const QModelIndex &index) const;
	bool setData(const QModelIndex &index, const QVariant &value, int role);
	bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
	QModelIndex addCard(const QString &cardName, const QString &zoneName);
	void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);
	void cleanList();
	DeckList *getDeckList() const { return deckList; }
	void setDeckList(DeckList *_deck);
private:
	DeckList *deckList;
	InnerDecklistNode *root;
	InnerDecklistNode *createNodeIfNeeded(const QString &name, InnerDecklistNode *parent);
	QModelIndex nodeToIndex(AbstractDecklistNode *node) const;
	void emitRecursiveUpdates(const QModelIndex &index);
	void sortHelper(InnerDecklistNode *node, Qt::SortOrder order);

	void printDeckListNode(QTextCursor *cursor, InnerDecklistNode *node);

	template<typename T> T getNode(const QModelIndex &index) const
	{
		if (!index.isValid())
			return dynamic_cast<T>(root);
		return dynamic_cast<T>(static_cast<AbstractDecklistNode *>(index.internalPointer()));
	}
};

#endif
