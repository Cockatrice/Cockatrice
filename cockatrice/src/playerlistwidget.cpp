#include "playerlistwidget.h"
#include "protocol_datastructures.h"
#include "pingpixmapgenerator.h"

PlayerListWidget::PlayerListWidget(QWidget *parent)
	: QTreeWidget(parent)
{
	setColumnCount(2);
	setRootIsDecorated(false);
	retranslateUi();
}

void PlayerListWidget::retranslateUi()
{
	headerItem()->setText(0, tr("Player name"));
	headerItem()->setText(1, tr("Role"));
}

void PlayerListWidget::addPlayer(ServerInfo_Player *player)
{
	QTreeWidgetItem *newPlayer = new QTreeWidgetItem;
	newPlayer->setText(0, player->getName());
	newPlayer->setText(1, player->getSpectator() ? tr("Spectator") : tr("Player"));
	addTopLevelItem(newPlayer);
	players.insert(player->getPlayerId(), newPlayer);
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
		twi->setBackground(0, c);
		twi->setBackground(1, c);
	}
}

void PlayerListWidget::updatePing(int playerId, int pingTime)
{
	QTreeWidgetItem *twi = players.value(playerId, 0);
	if (!twi)
		return;
	twi->setIcon(0, QIcon(PingPixmapGenerator::generatePixmap(10, pingTime, 10)));
}
