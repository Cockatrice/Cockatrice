#include "remote_replay_list_tree_widget.h"

#include <QFileIconProvider>
#include <QHeaderView>
#include <QSortFilterProxyModel>
#include <libcockatrice/network/client/abstract/abstract_client.h>
#include <libcockatrice/protocol/pb/command_replay_list.pb.h>
#include <libcockatrice/protocol/pb/response_replay_list.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_replay.pb.h>
#include <libcockatrice/protocol/pending_command.h>

const int RemoteReplayList_TreeModel::numberOfColumns = 6;

RemoteReplayList_TreeModel::MatchNode::MatchNode(const ServerInfo_ReplayMatch &_matchInfo)
    : RemoteReplayList_TreeModel::Node(QString::fromStdString(_matchInfo.game_name())), matchInfo(_matchInfo)
{
    for (int i = 0; i < matchInfo.replay_list_size(); ++i)
        append(new ReplayNode(matchInfo.replay_list(i), this));
}

RemoteReplayList_TreeModel::MatchNode::~MatchNode()
{
    for (int i = 0; i < size(); ++i)
        delete at(i);
}

void RemoteReplayList_TreeModel::MatchNode::updateMatchInfo(const ServerInfo_ReplayMatch &_matchInfo)
{
    matchInfo.MergeFrom(_matchInfo);
}

RemoteReplayList_TreeModel::RemoteReplayList_TreeModel(AbstractClient *_client, QObject *parent)
    : QAbstractItemModel(parent), client(_client)
{
    QFileIconProvider fip;
    dirIcon = fip.icon(QFileIconProvider::Folder);
    fileIcon = fip.icon(QFileIconProvider::File);
    lockIcon = QPixmap("theme:icons/lock");
}

RemoteReplayList_TreeModel::~RemoteReplayList_TreeModel()
{
    clearAll();
}

int RemoteReplayList_TreeModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return replayMatches.size();

    auto *matchNode = dynamic_cast<MatchNode *>(static_cast<Node *>(parent.internalPointer()));
    if (matchNode)
        return matchNode->size();
    else
        return 0;
}

QVariant RemoteReplayList_TreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if (index.column() >= numberOfColumns)
        return QVariant();

    auto *replayNode = dynamic_cast<ReplayNode *>(static_cast<Node *>(index.internalPointer()));
    if (replayNode) {
        const ServerInfo_Replay &replayInfo = replayNode->getReplayInfo();
        switch (role) {
            case Qt::TextAlignmentRole:
                return index.column() == 0 ? Qt::AlignRight : Qt::AlignLeft;
            case Qt::DisplayRole: {
                switch (index.column()) {
                    case 0:
                        return replayInfo.replay_id();
                    case 1:
                        return QString::fromStdString(replayInfo.replay_name());
                    case 5:
                        return replayInfo.duration();
                    default:
                        return QVariant();
                }
            }
            case Qt::DecorationRole:
                return index.column() == 0 ? fileIcon : QVariant();
        }
    } else {
        auto *matchNode = dynamic_cast<MatchNode *>(static_cast<Node *>(index.internalPointer()));
        const ServerInfo_ReplayMatch &matchInfo = matchNode->getMatchInfo();
        switch (role) {
            case Qt::TextAlignmentRole:
                switch (index.column()) {
                    case 0:
                    case 5:
                        return Qt::AlignRight;
                    default:
                        return Qt::AlignLeft;
                }
            case Qt::DisplayRole: {
                switch (index.column()) {
                    case 0:
                        return matchInfo.game_id();
                    case 1:
                        return QString::fromStdString(matchInfo.game_name());
                    case 2: {
                        QStringList playerList;
                        for (int i = 0; i < matchInfo.player_names_size(); ++i)
                            playerList.append(QString::fromStdString(matchInfo.player_names(i)));
                        return playerList.join(", ");
                    }
                    case 4:
                        return QDateTime::fromSecsSinceEpoch(matchInfo.time_started());
                    case 5:
                        return matchInfo.length();
                    default:
                        return QVariant();
                }
            }
            case Qt::DecorationRole:
                switch (index.column()) {
                    case 0:
                        return dirIcon;
                    case 3:
                        return matchInfo.do_not_hide() ? lockIcon : QVariant();
                    default:
                        return QVariant();
                }
        }
    }
    return QVariant();
}

QVariant RemoteReplayList_TreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal)
        return QVariant();
    switch (role) {
        case Qt::TextAlignmentRole:
            switch (section) {
                case 0:
                case 5:
                    return Qt::AlignRight;
                default:
                    return Qt::AlignLeft;
            }
        case Qt::DisplayRole: {
            switch (section) {
                case 0:
                    return tr("ID");
                case 1:
                    return tr("Name");
                case 2:
                    return tr("Players");
                case 3:
                    return tr("Keep");
                case 4:
                    return tr("Time started");
                case 5:
                    return tr("Duration (sec)");
                default:
                    return QVariant();
            }
        }
        default:
            return QVariant();
    }
}

QModelIndex RemoteReplayList_TreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    auto *matchNode = dynamic_cast<MatchNode *>(static_cast<Node *>(parent.internalPointer()));
    if (matchNode) {
        if (row >= matchNode->size())
            return QModelIndex();
        return createIndex(row, column, (void *)matchNode->at(row));
    } else {
        if (row >= replayMatches.size())
            return QModelIndex();
        return createIndex(row, column, (void *)replayMatches[row]);
    }
}

QModelIndex RemoteReplayList_TreeModel::parent(const QModelIndex &ind) const
{
    MatchNode const *matchNode = dynamic_cast<MatchNode *>(static_cast<Node *>(ind.internalPointer()));
    if (matchNode)
        return QModelIndex();
    else {
        auto *replayNode = dynamic_cast<ReplayNode *>(static_cast<Node *>(ind.internalPointer()));
        return createIndex(replayNode->getParent()->indexOf(replayNode), 0, replayNode->getParent());
    }
}

Qt::ItemFlags RemoteReplayList_TreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

ServerInfo_Replay const *RemoteReplayList_TreeModel::getReplay(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    auto *node = dynamic_cast<ReplayNode *>(static_cast<Node *>(index.internalPointer()));
    if (!node)
        return 0;
    return &node->getReplayInfo();
}

ServerInfo_ReplayMatch const *RemoteReplayList_TreeModel::getReplayMatch(const QModelIndex &index) const
{
    if (!index.isValid())
        return nullptr;

    auto *node = dynamic_cast<MatchNode *>(static_cast<Node *>(index.internalPointer()));
    if (!node)
        return nullptr;

    return &node->getMatchInfo();
}

ServerInfo_ReplayMatch const *RemoteReplayList_TreeModel::getEnclosingReplayMatch(const QModelIndex &index) const
{
    if (!index.isValid())
        return nullptr;

    auto *node = dynamic_cast<MatchNode *>(static_cast<Node *>(index.internalPointer()));
    if (!node) {
        auto *_node = dynamic_cast<ReplayNode *>(static_cast<Node *>(index.internalPointer()));
        if (!_node)
            return nullptr;
        return &_node->getParent()->getMatchInfo();
    }
    return &node->getMatchInfo();
}

/**
 * Deletes all items in the model
 */
void RemoteReplayList_TreeModel::clearAll()
{
    for (int i = 0; i < replayMatches.size(); ++i)
        delete replayMatches[i];
    replayMatches.clear();
}

void RemoteReplayList_TreeModel::refreshTree()
{
    PendingCommand *pend = client->prepareSessionCommand(Command_ReplayList());
    connect(pend, &PendingCommand::finished, this, &RemoteReplayList_TreeModel::replayListFinished);

    client->sendCommand(pend);
}

void RemoteReplayList_TreeModel::clearTree()
{
    beginResetModel();
    clearAll();
    endResetModel();
}

void RemoteReplayList_TreeModel::addMatchInfo(const ServerInfo_ReplayMatch &matchInfo)
{
    beginInsertRows(QModelIndex(), replayMatches.size(), replayMatches.size());
    replayMatches.append(new MatchNode(matchInfo));
    endInsertRows();

    emit treeRefreshed();
}

void RemoteReplayList_TreeModel::updateMatchInfo(int gameId, const ServerInfo_ReplayMatch &matchInfo)
{
    for (int i = 0; i < replayMatches.size(); ++i)
        if (replayMatches[i]->getMatchInfo().game_id() == gameId) {
            replayMatches[i]->updateMatchInfo(matchInfo);
            emit dataChanged(createIndex(i, 0, (void *)replayMatches[i]),
                             createIndex(i, numberOfColumns - 1, (void *)replayMatches[i]));
            break;
        }
}

void RemoteReplayList_TreeModel::removeMatchInfo(int gameId)
{
    for (int i = 0; i < replayMatches.size(); ++i)
        if (replayMatches[i]->getMatchInfo().game_id() == gameId) {
            beginRemoveRows(QModelIndex(), i, i);
            replayMatches.removeAt(i);
            endRemoveRows();
            break;
        }
}

void RemoteReplayList_TreeModel::replayListFinished(const Response &r)
{
    const Response_ReplayList &resp = r.GetExtension(Response_ReplayList::ext);

    beginResetModel();
    clearAll();

    for (int i = 0; i < resp.match_list_size(); ++i)
        replayMatches.append(new MatchNode(resp.match_list(i)));

    endResetModel();
    emit treeRefreshed();
}

RemoteReplayList_TreeWidget::RemoteReplayList_TreeWidget(AbstractClient *_client, QWidget *parent) : QTreeView(parent)
{
    treeModel = new RemoteReplayList_TreeModel(_client, this);
    proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(treeModel);
    proxyModel->setDynamicSortFilter(true);
    proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    setModel(proxyModel);

    header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    header()->setStretchLastSection(false);
    setUniformRowHeights(true);
    setSortingEnabled(true);
    proxyModel->sort(0, Qt::AscendingOrder);
    header()->setSortIndicator(0, Qt::AscendingOrder);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
}

/**
 * Gets the replay at the given index
 * @return The replay. Returns nullptr if there is no replay at the index.
 */
ServerInfo_Replay const *RemoteReplayList_TreeWidget::getReplay(const QModelIndex &ind) const
{
    return treeModel->getReplay(proxyModel->mapToSource(ind));
}

/**
 * Gets the replay match at the given index
 * @return The replay match. Returns nullptr if there is no replay match at the index.
 */
ServerInfo_ReplayMatch const *RemoteReplayList_TreeWidget::getReplayMatch(const QModelIndex &ind) const
{
    return treeModel->getReplayMatch(proxyModel->mapToSource(ind));
}

/**
 * Gets all currently selected replays.
 * Any selection that isn't a replay file (e.g. a folder) will appear as a nullptr in the list.
 * Make sure to check the list for nullptr before using it.
 *
 * @return A List of pointers to the selected replays, as well as nullptr for any selection that isn't a replay.
 */
QList<ServerInfo_Replay const *> RemoteReplayList_TreeWidget::getSelectedReplays() const
{
    const auto selection = selectionModel()->selectedRows();
    auto replays = QList<ServerInfo_Replay const *>();
    for (const auto &row : selection) {
        replays << treeModel->getReplay(proxyModel->mapToSource(row));
    }

    return replays;
}

/**
 * Gets all currently selected replayMatches.
 * If a non-folder node is selected, it will return the parent folder of that node.
 *
 * @return A Set of pointers to the selected replayMatches.
 */
QSet<ServerInfo_ReplayMatch const *> RemoteReplayList_TreeWidget::getSelectedReplayMatches() const
{
    const auto selection = selectionModel()->selectedRows();
    auto replayMatches = QSet<ServerInfo_ReplayMatch const *>();
    for (const auto &row : selection) {
        if (const auto replayMatch = treeModel->getEnclosingReplayMatch(proxyModel->mapToSource(row))) {
            replayMatches << replayMatch;
        }
    }

    return replayMatches;
}
