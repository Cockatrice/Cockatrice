#ifndef DECKLISTMODEL_H
#define DECKLISTMODEL_H

#include <QAbstractItemModel>
#include <QList>
#include "decklist.h"

class CardDatabase;

class DecklistModelCardNode : public AbstractDecklistNode {
private:
	DecklistCardNode *dataNode;
public:
	DecklistModelCardNode(DecklistCardNode *_dataNode, InnerDecklistNode *_parent) : AbstractDecklistNode(_parent), dataNode(_dataNode) { }
	bool hasChildren() const { return false; }
	inline int getNumber() const { return dataNode->getNumber(); }
	inline void setNumber(int _number) { dataNode->setNumber(_number); }
	inline QString getName() const { return dataNode->getName(); }
	inline void setName(const QString &_name) { dataNode->setName(_name); }
};

class DeckListModel : public QAbstractItemModel {
	Q_OBJECT
private slots:
	void rebuildTree();
public:
	DeckListModel(CardDatabase *_db, QObject *parent = 0);
	~DeckListModel();
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	bool hasChildren(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
	QModelIndex parent(const QModelIndex &index) const;
	Qt::ItemFlags flags(const QModelIndex &index) const;
	bool setData(const QModelIndex &index, const QVariant &value, int role);
	bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
	bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
	void cleanList();
	DeckList *getDeckList() const { return deckList; }
private:
	CardDatabase *db;
	DeckList *deckList;
	InnerDecklistNode *root;
	AbstractDecklistNode *findNode(const QString &name, InnerDecklistNode *root) const;
	AbstractDecklistNode *findNode(const QModelIndex &index) const;
	void debugIndexInfo(const QString &func, const QModelIndex &index) const;
	void debugShowTree(InnerDecklistNode *node, int depth) const;
};

#endif
