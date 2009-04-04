#ifndef CARDDATABASEMODEL_H
#define CARDDATABASEMODEL_H

#include <QAbstractListModel>
#include <QList>
#include "carddatabase.h"

class CardDatabaseModel : public QAbstractListModel {
	Q_OBJECT
public:
	CardDatabaseModel(CardDatabase *_db, QObject *parent = 0);
	~CardDatabaseModel();
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);
private:
	QList<CardInfo *> cardList;
	CardDatabase *db;
};

#endif
