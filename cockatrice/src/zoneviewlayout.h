#ifndef ZONEVIEWLAYOUT_H
#define ZONEVIEWLAYOUT_H

#include <QGraphicsWidget>

class CardDatabase;
class ZoneViewWidget;
class Player;

class ZoneViewLayout : public QGraphicsWidget {
	Q_OBJECT
signals:
	void sizeChanged();
private:
	QList<ZoneViewWidget *> views;
	CardDatabase *db;
public:
	ZoneViewLayout(CardDatabase *_db, QGraphicsItem *parent = 0);
	void reorganize();
public slots:
	void addItem(Player *player, const QString &zoneName, int numberCards = 0);
	void removeItem(ZoneViewWidget *item);
	void closeMostRecentZoneView();
	void clear();
};

#endif
