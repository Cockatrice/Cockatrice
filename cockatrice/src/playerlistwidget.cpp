#include <QHeaderView>
#include "playerlistwidget.h"
#include "protocol_datastructures.h"
#include "pixmapgenerator.h"
#include "abstractclient.h"
#include "tab_game.h"
#include "tab_supervisor.h"
#include "tab_userlists.h"
#include "protocol_items.h"
#include "userlist.h"
#include "userinfobox.h"
#include <QDebug>
#include <QMouseEvent>
#include <QAction>
#include <QMenu>

PlayerListItemDelegate::PlayerListItemDelegate(QObject *const parent)
	: QStyledItemDelegate(parent)
{
}

bool PlayerListItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
	if ((event->type() == QEvent::MouseButtonPress) && index.isValid()) {
		QMouseEvent *const mouseEvent = static_cast<QMouseEvent *>(event);
		if (mouseEvent->button() == Qt::RightButton) {
			static_cast<PlayerListWidget *>(parent())->showContextMenu(mouseEvent->globalPos(), index);
			return true;
		}
	}
	return QStyledItemDelegate::editorEvent(event, model, option, index);
}

PlayerListTWI::PlayerListTWI()
	: QTreeWidgetItem(Type)
{
}

bool PlayerListTWI::operator<(const QTreeWidgetItem &other) const
{
	// Sort by spectator/player
	if (data(1, Qt::UserRole) != other.data(1, Qt::UserRole))
		return data(1, Qt::UserRole).toBool();
	
	// Sort by player ID
	return data(4, Qt::UserRole + 1).toInt() < other.data(4, Qt::UserRole + 1).toInt();
}

PlayerListWidget::PlayerListWidget(TabSupervisor *_tabSupervisor, AbstractClient *_client, TabGame *_game, bool _gameCreator, QWidget *parent)
	: QTreeWidget(parent), tabSupervisor(_tabSupervisor), client(_client), game(_game), gameCreator(_gameCreator), gameStarted(false)
{
	readyIcon = QIcon(":/resources/icon_ready_start.svg");
	notReadyIcon = QIcon(":/resources/icon_not_ready_start.svg");
	concededIcon = QIcon(":/resources/icon_conceded.svg");
	playerIcon = QIcon(":/resources/icon_player.svg");
	spectatorIcon = QIcon(":/resources/icon_spectator.svg");

	itemDelegate = new PlayerListItemDelegate(this);
	setItemDelegate(itemDelegate);
	
	setMinimumHeight(60);
	setIconSize(QSize(20, 15));
	setColumnCount(6);
	setHeaderHidden(true);
	setRootIsDecorated(false);
	header()->setResizeMode(QHeaderView::ResizeToContents);
	retranslateUi();
}

void PlayerListWidget::retranslateUi()
{
}

void PlayerListWidget::addPlayer(ServerInfo_PlayerProperties *player)
{
	QTreeWidgetItem *newPlayer = new PlayerListTWI;
	players.insert(player->getPlayerId(), newPlayer);
	updatePlayerProperties(player);
	addTopLevelItem(newPlayer);
	sortItems(1, Qt::AscendingOrder);
}

void PlayerListWidget::updatePlayerProperties(ServerInfo_PlayerProperties *prop)
{
	QTreeWidgetItem *player = players.value(prop->getPlayerId(), 0);
	if (!player)
		return;

	player->setIcon(1, prop->getSpectator() ? spectatorIcon : playerIcon);
	player->setData(1, Qt::UserRole, !prop->getSpectator());
	player->setData(2, Qt::UserRole, prop->getConceded());
	player->setData(2, Qt::UserRole + 1, prop->getReadyStart());
	player->setIcon(2, gameStarted ? (prop->getConceded() ? concededIcon : QIcon()) : (prop->getReadyStart() ? readyIcon : notReadyIcon));
	player->setData(3, Qt::UserRole, prop->getUserInfo()->getUserLevel());
	player->setIcon(3, QIcon(UserLevelPixmapGenerator::generatePixmap(12, prop->getUserInfo()->getUserLevel())));
	player->setText(4, prop->getUserInfo()->getName());
	if (!prop->getUserInfo()->getCountry().isEmpty())
		player->setIcon(4, QIcon(CountryPixmapGenerator::generatePixmap(12, prop->getUserInfo()->getCountry())));
	player->setData(4, Qt::UserRole, prop->getUserInfo()->getName());
	player->setData(4, Qt::UserRole + 1, prop->getPlayerId());
	player->setText(5, prop->getDeckHash());
}

void PlayerListWidget::removePlayer(int playerId)
{
	QTreeWidgetItem *player = players.value(playerId, 0);
	if (!player)
		return;
	players.remove(playerId);
	delete takeTopLevelItem(indexOfTopLevelItem(player));
}

void PlayerListWidget::setActivePlayer(int playerId)
{
	QMapIterator<int, QTreeWidgetItem *> i(players);
	while (i.hasNext()) {
		i.next();
		QTreeWidgetItem *twi = i.value();
		QColor c = i.key() == playerId ? QColor(150, 255, 150) : Qt::white;
		twi->setBackground(4, c);
	}
}

void PlayerListWidget::updatePing(int playerId, int pingTime)
{
	QTreeWidgetItem *twi = players.value(playerId, 0);
	if (!twi)
		return;
	twi->setIcon(0, QIcon(PingPixmapGenerator::generatePixmap(12, pingTime, 10)));
}

void PlayerListWidget::setGameStarted(bool _gameStarted, bool resuming)
{
	gameStarted = _gameStarted;
	QMapIterator<int, QTreeWidgetItem *> i(players);
	while (i.hasNext()) {
		QTreeWidgetItem *twi = i.next().value();
		if (gameStarted) {
			if (resuming)
				twi->setIcon(2, twi->data(2, Qt::UserRole).toBool() ? concededIcon : QIcon());
			else {
				twi->setData(2, Qt::UserRole, false);
				twi->setIcon(2, QIcon());
			}
		} else
			twi->setIcon(2, notReadyIcon);
	}
}

void PlayerListWidget::showContextMenu(const QPoint &pos, const QModelIndex &index)
{
	const QString &userName = index.sibling(index.row(), 4).data(Qt::UserRole).toString();
	int playerId = index.sibling(index.row(), 4).data(Qt::UserRole + 1).toInt();
	ServerInfo_User::UserLevelFlags userLevel = static_cast<ServerInfo_User::UserLevelFlags>(index.sibling(index.row(), 3).data(Qt::UserRole).toInt());
	
	QAction *aUserName = new QAction(userName, this);
	aUserName->setEnabled(false);
	QAction *aDetails = new QAction(tr("User &details"), this);
	QAction *aChat = new QAction(tr("Direct &chat"), this);
	QAction *aAddToBuddyList = new QAction(tr("Add to &buddy list"), this);
	QAction *aRemoveFromBuddyList = new QAction(tr("Remove from &buddy list"), this);
	QAction *aAddToIgnoreList = new QAction(tr("Add to &ignore list"), this);
	QAction *aRemoveFromIgnoreList = new QAction(tr("Remove from &ignore list"), this);
	QAction *aKick = new QAction(tr("Kick from &game"), this);
	
	QMenu *menu = new QMenu(this);
	menu->addAction(aUserName);
	menu->addSeparator();
	menu->addAction(aDetails);
	menu->addAction(aChat);
	if ((userLevel & ServerInfo_User::IsRegistered) && (tabSupervisor->getUserLevel() & ServerInfo_User::IsRegistered)) {
		menu->addSeparator();
		if (tabSupervisor->getUserListsTab()->getBuddyList()->userInList(userName))
			menu->addAction(aRemoveFromBuddyList);
		else
			menu->addAction(aAddToBuddyList);
		if (tabSupervisor->getUserListsTab()->getIgnoreList()->userInList(userName))
			menu->addAction(aRemoveFromIgnoreList);
		else
			menu->addAction(aAddToIgnoreList);
	}
	if (gameCreator) {
		menu->addSeparator();
		menu->addAction(aKick);
	}
	
	QAction *actionClicked = menu->exec(pos);
	if (actionClicked == aDetails) {
		UserInfoBox *infoWidget = new UserInfoBox(client, true, this, Qt::Dialog | Qt::WindowTitleHint | Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);
		infoWidget->setAttribute(Qt::WA_DeleteOnClose);
		infoWidget->updateInfo(userName);
	} else if (actionClicked == aChat)
		emit openMessageDialog(userName, true);
	else if (actionClicked == aAddToBuddyList)
		client->sendCommand(new Command_AddToList("buddy", userName));
	else if (actionClicked == aRemoveFromBuddyList)
		client->sendCommand(new Command_RemoveFromList("buddy", userName));
	else if (actionClicked == aAddToIgnoreList)
		client->sendCommand(new Command_AddToList("ignore", userName));
	else if (actionClicked == aRemoveFromIgnoreList)
		client->sendCommand(new Command_RemoveFromList("ignore", userName));
	else if (actionClicked == aKick)
		game->sendGameCommand(new Command_KickFromGame(-1, playerId));
	
	delete menu;
	delete aUserName;
	delete aDetails;
	delete aChat;
	delete aAddToBuddyList;
	delete aRemoveFromBuddyList;
	delete aAddToIgnoreList;
	delete aRemoveFromIgnoreList;
	delete aKick;
}
