#include "playerlistwidget.h"
#include "protocol_datastructures.h"

PlayerListWidget::PlayerListWidget(QWidget *parent)
	: QTreeWidget(parent)
{
	setColumnCount(1);
	setRootIsDecorated(false);
	retranslateUi();
}

void PlayerListWidget::retranslateUi()
{
	headerItem()->setText(0, tr("Player name"));
}

void PlayerListWidget::addPlayer(ServerInfo_Player *player)
{
	QTreeWidgetItem *newPlayer = new QTreeWidgetItem;
	newPlayer->setText(0, player->getName());
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
		twi->setBackground(0, i.key() == playerId ? QColor(150, 255, 150) : Qt::white);
	}
}
