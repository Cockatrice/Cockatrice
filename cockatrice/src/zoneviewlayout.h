#ifndef ZONEVIEWLAYOUT_H
#define ZONEVIEWLAYOUT_H

#include <QGraphicsWidget>

class ZoneViewWidget;
class ZoneViewZone;
class Player;

class ZoneViewLayout : public QGraphicsWidget {
	Q_OBJECT
signals:
	void sizeChanged();
private:
	QList<ZoneViewWidget *> views;
public:
	ZoneViewLayout(QGraphicsItem *parent = 0);
	void retranslateUi();
public slots:
	void toggleZoneView(Player *player, const QString &zoneName, int numberCards = 0);
	void removeItem(ZoneViewWidget *item);
	void removeItem(ZoneViewZone *item);
	void closeMostRecentZoneView();
	void reorganize();
	void clear();
};

#endif
