#ifndef ZONEVIEWWIDGET_H
#define ZONEVIEWWIDGET_H

#include <QDialog>
#include <QGraphicsScene>
#include <QGraphicsWidget>
#include "serverzonecard.h"

class CardZone;
class ZoneViewZone;
class Player;
class CardDatabase;
class QScrollBar;
class QCheckBox;

class ZoneViewWidget : public QGraphicsWidget {
	Q_OBJECT
private:
	ZoneViewZone *zone;
	
	QScrollBar *scrollBar;
	QCheckBox *sortCheckBox, *shuffleCheckBox;
	
	CardDatabase *db;
	Player *player;
signals:
	void closePressed(ZoneViewWidget *zv);
	void sizeChanged();
private slots:
	void resizeToZoneContents();
public:
	ZoneViewWidget(CardDatabase *_db, Player *_player, CardZone *_origZone, int numberCards = 0, QGraphicsItem *parent = 0);
	ZoneViewZone *getZone() const { return zone; }
	void retranslateUi();
protected:
	void closeEvent(QCloseEvent *event);
};

#endif
