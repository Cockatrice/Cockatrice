#ifndef GAMESMODEL_H
#define GAMESMODEL_H

#include <QAbstractListModel>
#include <QList>
#include "servergame.h"

class GamesModel : public QAbstractTableModel {
	Q_OBJECT
public:
	GamesModel(QObject *parent = 0) : QAbstractTableModel(parent) { }
	~GamesModel();
	int rowCount(const QModelIndex &parent = QModelIndex()) const { return parent.isValid() ? 0 : gameList.size(); }
	int columnCount(const QModelIndex &/*parent*/ = QModelIndex()) const { return 5; }
	QVariant data(const QModelIndex &index, int role) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	
	ServerGame *getGame(int row);
	void cleanList();
public slots:
	void updateGameList(ServerGame *game);
private:
	QList<ServerGame *> gameList;
};

#endif
