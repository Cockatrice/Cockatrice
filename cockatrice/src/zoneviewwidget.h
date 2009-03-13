#ifndef ZONEVIEWWIDGET_H
#define ZONEVIEWWIDGET_H

#include <QDialog>
#include <QGraphicsScene>
#include <QGraphicsWidget>
#include "serverzonecard.h"

class PlayerZone;
class ZoneViewZone;
class Player;
class CardDatabase;

class ZoneViewWidget : public QGraphicsWidget {
	Q_OBJECT
private:
	ZoneViewZone *zone;
	int cmdId;
	
	CardDatabase *db;
	Player *player;
signals:
	void closePressed(ZoneViewWidget *zv);
private slots:
	void zoneDumpReceived(int commandId, QList<ServerZoneCard *> cards);
	void slotClosePressed();
public:
	ZoneViewWidget(CardDatabase *_db, Player *_player, PlayerZone *_origZone, int numberCards = 0, QGraphicsItem *parent = 0);
};

#endif
