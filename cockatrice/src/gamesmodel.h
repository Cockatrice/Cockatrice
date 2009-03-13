#ifndef GAMESMODEL_H
#define GAMESMODEL_H

#include <QAbstractListModel>
#include <QList>
#include "servergame.h"

class GamesModel : public QAbstractListModel {
	Q_OBJECT
public:
	GamesModel(QObject *parent = 0)
		: QAbstractListModel(parent) { }
	~GamesModel();
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	
	void setGameList(const QList<ServerGame *> &_gameList);
	ServerGame *getGame(int row);
private:
	QList<ServerGame *> gameList;
	void cleanList();
};

#endif
