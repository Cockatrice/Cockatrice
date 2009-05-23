#ifndef DECKLISTMODEL_H
#define DECKLISTMODEL_H

#include <QAbstractItemModel>
#include <QList>
#include "decklist.h"

class CardDatabase;

class DeckListModel : public QAbstractItemModel {
	Q_OBJECT
private slots:
	void resetModel();
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
	bool loadFromFile(const QString &fileName, DeckList::FileFormat fmt);
	bool saveToFile(const QString &fileName, DeckList::FileFormat fmt);
private:
	CardDatabase *db;
	DeckList *deckList;
};

#endif
