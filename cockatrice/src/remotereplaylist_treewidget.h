#ifndef REMOTEREPLAYLIST_TREEWIDGET_H
#define REMOTEREPLAYLIST_TREEWIDGET_H

#include <QAbstractItemModel>
#include <QDateTime>
#include <QTreeView>
#include "pb/serverinfo_replay.pb.h"

class Response;
class AbstractClient;
class QSortFilterProxyModel;

class RemoteReplayList_TreeModel : public QAbstractItemModel {
	Q_OBJECT
private:
	AbstractClient *client;
	QList<ServerInfo_Replay> replays;
	
	QIcon fileIcon;
signals:
	void treeRefreshed();
private slots:
	void replayListFinished(const Response &r);
public:
	RemoteReplayList_TreeModel(AbstractClient *_client, QObject *parent = 0);
	~RemoteReplayList_TreeModel();
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &/*parent*/ = QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
	QModelIndex parent(const QModelIndex &index) const;
	Qt::ItemFlags flags(const QModelIndex &index) const;
	void refreshTree();
	ServerInfo_Replay const *getNode(const QModelIndex &index) const;
};

class RemoteReplayList_TreeWidget : public QTreeView {
private:
	RemoteReplayList_TreeModel *treeModel;
	QSortFilterProxyModel *proxyModel;
	ServerInfo_Replay const *getNode(const QModelIndex &ind) const;
public:
	RemoteReplayList_TreeWidget(AbstractClient *_client, QWidget *parent = 0);
	ServerInfo_Replay const *getCurrentItem() const;
	void refreshTree();
};

#endif
