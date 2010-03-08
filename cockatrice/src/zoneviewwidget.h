#ifndef ZONEVIEWWIDGET_H
#define ZONEVIEWWIDGET_H

#include <QDialog>
#include <QGraphicsScene>
#include <QGraphicsWidget>

class CardZone;
class ZoneViewZone;
class Player;
class CardDatabase;
class QScrollBar;
class QCheckBox;
class GameScene;

class ZoneViewWidget : public QGraphicsWidget {
	Q_OBJECT
private:
	ZoneViewZone *zone;
	
	QScrollBar *scrollBar;
	QCheckBox *sortByNameCheckBox, *sortByTypeCheckBox, *shuffleCheckBox;
	
	int extraHeight;
	Player *player;
signals:
	void closePressed(ZoneViewWidget *zv);
private slots:
	void resizeToZoneContents();
	void zoneDeleted();
public:
	ZoneViewWidget(Player *_player, CardZone *_origZone, int numberCards = 0);
	ZoneViewZone *getZone() const { return zone; }
	void retranslateUi();
protected:
	void closeEvent(QCloseEvent *event);
};

#endif
