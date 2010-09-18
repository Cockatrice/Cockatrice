#include <QHeaderView>
#include "playerlistwidget.h"
#include "protocol_datastructures.h"
#include "pixmapgenerator.h"
#include <QDebug>

PlayerListWidget::PlayerListWidget(QWidget *parent)
	: QTreeWidget(parent), gameStarted(false)
{
	readyIcon = QIcon(":/resources/icon_ready_start.svg");
	notReadyIcon = QIcon(":/resources/icon_not_ready_start.svg");
	concededIcon = QIcon(":/resources/icon_conceded.svg");
	playerIcon = QIcon(":/resources/icon_player.svg");
	spectatorIcon = QIcon(":/resources/icon_spectator.svg");

	setColumnCount(5);
	setRootIsDecorated(false);
	setSelectionMode(NoSelection);
	header()->setResizeMode(QHeaderView::ResizeToContents);
	retranslateUi();
}

void PlayerListWidget::retranslateUi()
{
	headerItem()->setText(0, QString());
	headerItem()->setText(1, QString());
	headerItem()->setText(2, QString());
	headerItem()->setText(3, tr("Player name"));
	headerItem()->setText(4, tr("Deck"));
}

void PlayerListWidget::addPlayer(ServerInfo_PlayerProperties *player)
{
	QTreeWidgetItem *newPlayer = new QTreeWidgetItem;
	players.insert(player->getPlayerId(), newPlayer);
	updatePlayerProperties(player);
	addTopLevelItem(newPlayer);
}

void PlayerListWidget::updatePlayerProperties(ServerInfo_PlayerProperties *prop)
{
	QTreeWidgetItem *player = players.value(prop->getPlayerId(), 0);
	if (!player)
		return;

	player->setIcon(1, prop->getSpectator() ? spectatorIcon : playerIcon);
	player->setIcon(2, gameStarted ? (prop->getConceded() ? concededIcon : QIcon()) : (prop->getReadyStart() ? readyIcon : notReadyIcon));
	player->setText(3, prop->getUserInfo()->getName());
	if (!prop->getUserInfo()->getCountry().isEmpty())
		player->setIcon(3, QIcon(CountryPixmapGenerator::generatePixmap(10, prop->getUserInfo()->getCountry())));

	QString deckText;
	if (!prop->getSpectator())
		switch (prop->getDeckId()) {
			case -2: deckText = tr("---"); break;
			case -1: deckText = tr("local"); break;
			default: deckText = tr("#%1").arg(prop->getDeckId());
		}
	player->setText(4, deckText);
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
		twi->setBackground(3, c);
	}
}

void PlayerListWidget::updatePing(int playerId, int pingTime)
{
	QTreeWidgetItem *twi = players.value(playerId, 0);
	if (!twi)
		return;
	twi->setIcon(0, QIcon(PingPixmapGenerator::generatePixmap(10, pingTime, 10)));
}

void PlayerListWidget::setGameStarted(bool _gameStarted)
{
	gameStarted = _gameStarted;
	QMapIterator<int, QTreeWidgetItem *> i(players);
	while (i.hasNext()) {
		QTreeWidgetItem *twi = i.next().value();
		twi->setIcon(2, gameStarted ? QIcon() : notReadyIcon);
	}
}
