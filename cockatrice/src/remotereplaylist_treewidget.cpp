#include <QFileIconProvider>
#include <QHeaderView>
#include <QSortFilterProxyModel>
#include "remotereplaylist_treewidget.h"
#include "abstractclient.h"

#include "pending_command.h"
#include "pb/command_replay_list.pb.h"
#include "pb/response_replay_list.pb.h"
#include "pb/serverinfo_replay.pb.h"

RemoteReplayList_TreeModel::RemoteReplayList_TreeModel(AbstractClient *_client, QObject *parent)
	: QAbstractItemModel(parent), client(_client)
{
	QFileIconProvider fip;
	fileIcon = fip.icon(QFileIconProvider::File);

	refreshTree();
}

RemoteReplayList_TreeModel::~RemoteReplayList_TreeModel()
{
}

int RemoteReplayList_TreeModel::rowCount(const QModelIndex &parent) const
{
	return parent.isValid() ? 0 : replays.size();
}

int RemoteReplayList_TreeModel::columnCount(const QModelIndex &/*parent*/) const
{
	return 6;
}

QVariant RemoteReplayList_TreeModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();
	if (index.column() > 5)
		return QVariant();
	
	ServerInfo_Replay *replayInfo = static_cast<ServerInfo_Replay *>(index.internalPointer());
	switch (role) {
		case Qt::TextAlignmentRole:
			return index.column() == 0 ? Qt::AlignRight : Qt::AlignLeft;
		case Qt::DisplayRole: {
			switch (index.column()) {
				case 0: return replayInfo->game_id();
				case 1: return QString::fromStdString(replayInfo->game_name());
				case 2: return QString::fromStdString(replayInfo->replay_name());
				case 3: {
					QStringList playerList;
					for (int i = 0; i < replayInfo->player_names_size(); ++i)
						playerList.append(QString::fromStdString(replayInfo->player_names(i)));
					return playerList.join(", ");
				}
				case 4: return QDateTime::fromTime_t(replayInfo->time_started());
				case 5: return replayInfo->length();
				default: return QVariant();
			}
		}
		case Qt::DecorationRole:
			return index.column() == 0 ? fileIcon : QVariant();
	}
	return QVariant();
}

QVariant RemoteReplayList_TreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation != Qt::Horizontal)
		return QVariant();
	switch (role) {
		case Qt::TextAlignmentRole:
			return section == 0 ? Qt::AlignRight : Qt::AlignLeft;
		case Qt::DisplayRole: {
			switch (section) {
				case 0: return tr("Game ID");
				case 1: return tr("Game name");
				case 2: return tr("Replay name");
				case 3: return tr("Players");
				case 4: return tr("Time started");
				case 5: return tr("Duration (sec)");
				default: return QVariant();
			}
		}
		default: return QVariant();
	}
}

QModelIndex RemoteReplayList_TreeModel::index(int row, int column, const QModelIndex &parent) const
{
	if (!hasIndex(row, column, parent))
		return QModelIndex();

	return createIndex(row, column, (void *) &(replays[row]));
}

QModelIndex RemoteReplayList_TreeModel::parent(const QModelIndex &ind) const
{
	return QModelIndex();
}

Qt::ItemFlags RemoteReplayList_TreeModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return 0;

	return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

ServerInfo_Replay const* RemoteReplayList_TreeModel::getNode(const QModelIndex &index) const
{
	if (!index.isValid())
		return 0;
	
	return &(replays[index.row()]);
}

void RemoteReplayList_TreeModel::refreshTree()
{
	PendingCommand *pend = client->prepareSessionCommand(Command_ReplayList());
	connect(pend, SIGNAL(finished(const Response &)), this, SLOT(replayListFinished(const Response &)));
	
	client->sendCommand(pend);
}

void RemoteReplayList_TreeModel::replayListFinished(const Response &r)
{
	const Response_ReplayList &resp = r.GetExtension(Response_ReplayList::ext);
	
	beginResetModel();
	replays.clear();
	
	for (int i = 0; i < resp.replay_list_size(); ++i)
		replays.append(resp.replay_list(i));
	
	endResetModel();
	emit treeRefreshed();
}

RemoteReplayList_TreeWidget::RemoteReplayList_TreeWidget(AbstractClient *_client, QWidget *parent)
	: QTreeView(parent)
{
	treeModel = new RemoteReplayList_TreeModel(_client, this);
	proxyModel = new QSortFilterProxyModel(this);
	proxyModel->setSourceModel(treeModel);
	proxyModel->setDynamicSortFilter(true);
	proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
	setModel(proxyModel);
	connect(treeModel, SIGNAL(treeRefreshed()), this, SLOT(expandAll()));

	header()->setResizeMode(QHeaderView::ResizeToContents);
	setUniformRowHeights(true);
	setSortingEnabled(true);
	proxyModel->sort(0, Qt::AscendingOrder);
	header()->setSortIndicator(0, Qt::AscendingOrder);
}

ServerInfo_Replay const *RemoteReplayList_TreeWidget::getNode(const QModelIndex &ind) const
{
	return treeModel->getNode(proxyModel->mapToSource(ind));
}

ServerInfo_Replay const *RemoteReplayList_TreeWidget::getCurrentItem() const
{
	return getNode(selectionModel()->currentIndex());
}
