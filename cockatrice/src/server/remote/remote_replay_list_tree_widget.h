#ifndef REMOTEREPLAYLIST_TREEWIDGET_H
#define REMOTEREPLAYLIST_TREEWIDGET_H

#include "pb/serverinfo_replay.pb.h"
#include "pb/serverinfo_replay_match.pb.h"

#include <QAbstractItemModel>
#include <QDateTime>
#include <QTreeView>

class Response;
class AbstractClient;
class QSortFilterProxyModel;

class RemoteReplayList_TreeModel : public QAbstractItemModel
{
    Q_OBJECT
private:
    class MatchNode;
    class ReplayNode;
    class Node
    {
    protected:
        QString name;

    public:
        Node(const QString &_name) : name(_name)
        {
        }
        virtual ~Node() {};
        QString getName() const
        {
            return name;
        }
    };
    class MatchNode : public Node, public QList<ReplayNode *>
    {
    private:
        ServerInfo_ReplayMatch matchInfo;

    public:
        MatchNode(const ServerInfo_ReplayMatch &_matchInfo);
        ~MatchNode();
        void clearTree();
        const ServerInfo_ReplayMatch &getMatchInfo()
        {
            return matchInfo;
        }
        void updateMatchInfo(const ServerInfo_ReplayMatch &_matchInfo);
    };
    class ReplayNode : public Node
    {
    private:
        MatchNode *parent;
        ServerInfo_Replay replayInfo;

    public:
        ReplayNode(const ServerInfo_Replay &_replayInfo, MatchNode *_parent)
            : Node(QString::fromStdString(_replayInfo.replay_name())), parent(_parent), replayInfo(_replayInfo)
        {
        }
        MatchNode *getParent() const
        {
            return parent;
        }
        const ServerInfo_Replay &getReplayInfo()
        {
            return replayInfo;
        }
    };

    AbstractClient *client;
    QList<MatchNode *> replayMatches;

    QIcon dirIcon, fileIcon, lockIcon;
    void clearTree();

    static const int numberOfColumns;
signals:
    void treeRefreshed();
private slots:
    void replayListFinished(const Response &r);

public:
    RemoteReplayList_TreeModel(AbstractClient *_client, QObject *parent = nullptr);
    ~RemoteReplayList_TreeModel();
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex & /*parent*/ = QModelIndex()) const
    {
        return numberOfColumns;
    }
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    void refreshTree();
    ServerInfo_Replay const *getReplay(const QModelIndex &index) const;
    ServerInfo_ReplayMatch const *getReplayMatch(const QModelIndex &index) const;
    ServerInfo_ReplayMatch const *getEnclosingReplayMatch(const QModelIndex &index) const;
    void addMatchInfo(const ServerInfo_ReplayMatch &matchInfo);
    void updateMatchInfo(int gameId, const ServerInfo_ReplayMatch &matchInfo);
    void removeMatchInfo(int gameId);
};

class RemoteReplayList_TreeWidget : public QTreeView
{
private:
    RemoteReplayList_TreeModel *treeModel;
    QSortFilterProxyModel *proxyModel;

public:
    RemoteReplayList_TreeWidget(AbstractClient *_client, QWidget *parent = nullptr);
    ServerInfo_Replay const *getReplay(const QModelIndex &ind) const;
    ServerInfo_ReplayMatch const *getReplayMatch(const QModelIndex &ind) const;
    QList<ServerInfo_Replay const *> getSelectedReplays() const;
    QSet<ServerInfo_ReplayMatch const *> getSelectedReplayMatches() const;
    void refreshTree() {
        treeModel->refreshTree();
    }
    void addMatchInfo(const ServerInfo_ReplayMatch &matchInfo)
    {
        treeModel->addMatchInfo(matchInfo);
    }
    void updateMatchInfo(int gameId, const ServerInfo_ReplayMatch &matchInfo)
    {
        treeModel->updateMatchInfo(gameId, matchInfo);
    }
    void removeMatchInfo(int gameId)
    {
        treeModel->removeMatchInfo(gameId);
    }
};

#endif
