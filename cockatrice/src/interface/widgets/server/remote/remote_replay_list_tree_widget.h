/**
 * @file remote_replay_list_tree_widget.h
 * @ingroup DeckStorageWidgets
 * @ingroup Replays
 * @ingroup NetworkingWidgets
 * @brief TODO: Document this.
 */

#ifndef REMOTEREPLAYLIST_TREEWIDGET_H
#define REMOTEREPLAYLIST_TREEWIDGET_H

#include <QDateTime>
#include <QTreeView>
#include <libcockatrice/protocol/pb/serverinfo_replay.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_replay_match.pb.h>

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
        explicit Node(const QString &_name) : name(_name)
        {
        }
        virtual ~Node() = default;
        [[nodiscard]] QString getName() const
        {
            return name;
        }
    };
    class MatchNode : public Node, public QList<ReplayNode *>
    {
    private:
        ServerInfo_ReplayMatch matchInfo;

    public:
        explicit MatchNode(const ServerInfo_ReplayMatch &_matchInfo);
        ~MatchNode() override;
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
    void clearAll();

    static const int numberOfColumns;
signals:
    void treeRefreshed();
private slots:
    void replayListFinished(const Response &r);

public:
    explicit RemoteReplayList_TreeModel(AbstractClient *_client, QObject *parent = nullptr);
    ~RemoteReplayList_TreeModel() override;
    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]] int columnCount(const QModelIndex & /*parent*/ = QModelIndex()) const override
    {
        return numberOfColumns;
    }
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
    [[nodiscard]] QVariant
    headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    [[nodiscard]] QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]] QModelIndex parent(const QModelIndex &index) const override;
    [[nodiscard]] Qt::ItemFlags flags(const QModelIndex &index) const override;
    void clearTree();
    void refreshTree();
    [[nodiscard]] ServerInfo_Replay const *getReplay(const QModelIndex &index) const;
    [[nodiscard]] ServerInfo_ReplayMatch const *getReplayMatch(const QModelIndex &index) const;
    [[nodiscard]] ServerInfo_ReplayMatch const *getEnclosingReplayMatch(const QModelIndex &index) const;
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
    explicit RemoteReplayList_TreeWidget(AbstractClient *_client, QWidget *parent = nullptr);
    [[nodiscard]] ServerInfo_Replay const *getReplay(const QModelIndex &ind) const;
    [[nodiscard]] ServerInfo_ReplayMatch const *getReplayMatch(const QModelIndex &ind) const;
    [[nodiscard]] QList<ServerInfo_Replay const *> getSelectedReplays() const;
    [[nodiscard]] QSet<ServerInfo_ReplayMatch const *> getSelectedReplayMatches() const;
    void clearTree()
    {
        treeModel->clearTree();
    }
    void refreshTree()
    {
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
