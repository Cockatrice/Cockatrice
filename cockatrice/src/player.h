#ifndef PLAYER_H
#define PLAYER_H

#include <QInputDialog>
#include <QPoint>
#include <QMap>
#include "client.h"
#include "carditem.h"

class Client;
class CardDatabase;
class QMenu;
class QAction;
class ZoneViewZone;
class Game;
class Counter;
class CardZone;
class TableZone;
class HandZone;

class Player : public QObject, public QGraphicsItem {
	Q_OBJECT
signals:
	void moveCard(int cardId, QString startZone, QString targetZone, int x, int y);
	void hoverCard(QString name);
	void closeZoneView(ZoneViewZone *zone);
	void toggleZoneView(Player *player, QString zoneName, int number);
	void sigShowCardMenu(QPoint p);
	// Log events
	void logMoveCard(Player *player, QString cardName, CardZone *startZone, int oldX, CardZone *targetZone, int newX);
	void logCreateToken(Player *player, QString cardName);
	void logSetCardCounters(Player *player, QString cardName, int value, int oldValue);
	void logSetTapped(Player *player, QString cardName, bool tapped);
	void logSetCounter(Player *player, QString counterName, int value, int oldValue);
	void logSetDoesntUntap(Player *player, QString cardName, bool doesntUntap);
	
	void sizeChanged();
public slots:
	void actUntapAll();
	void actRollDie();
	void actCreateToken();
	void actShuffle();
	void actDrawCard();
	void actDrawCards();
        void actMulligan();

	void actViewLibrary();
	void actViewTopCards();
	void actViewGraveyard();
	void actViewRfg();
	void actViewSideboard();
	
	void actSayMessage();
private slots:
	void updateBoundingRect();
private:
	QMenu *playerMenu, *handMenu, *graveMenu, *rfgMenu, *libraryMenu, *sbMenu, *countersMenu, *sayMenu;
	QAction *aMoveHandToTopLibrary, *aMoveHandToBottomLibrary, *aMoveHandToGrave, *aMoveHandToRfg,
		*aMoveGraveToTopLibrary, *aMoveGraveToBottomLibrary, *aMoveGraveToHand, *aMoveGraveToRfg,
		*aMoveRfgToTopLibrary, *aMoveRfgToBottomLibrary, *aMoveRfgToHand, *aMoveRfgToGrave,
		*aViewLibrary, *aViewTopCards, *aViewGraveyard, *aViewRfg, *aViewSideboard,
                *aDrawCard, *aDrawCards, *aMulligan, *aShuffle,
		*aUntapAll, *aRollDie, *aCreateToken;

	int defaultNumberTopCards;
	QString name;
	int id;
	bool active;
	bool local;

        int cardsInHand;
	
	QMap<QString, CardZone *> zones;
	TableZone *table;
	HandZone *hand;
	
	CardDatabase *db;
	void setCardAttrHelper(CardItem *card, const QString &aname, const QString &avalue, bool allCards);

	QPixmap bgPixmap;
	QRectF bRect;

	QMap<int, Counter *> counters;
	void rearrangeCounters();
	
	void initSayMenu();
public:
	static const int counterAreaWidth = 65;
	
	enum { Type = typeOther };
	int type() const { return Type; }
	QRectF boundingRect() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	
	void addCounter(int counterId, const QString &name, QColor color, int radius, int value);
	void delCounter(int counterId);
	void clearCounters();

	Client *client;
	void addZone(CardZone *z);
	Player(const QString &_name, int _id, bool _local, CardDatabase *_db, Client *_client, Game *_parent);
	~Player();
	void retranslateUi();
	QMenu *getPlayerMenu() const { return playerMenu; }
	int getId() const { return id; }
	QString getName() const { return name; }
	bool getLocal() const { return local; }
	const QMap<QString, CardZone *> &getZones() const { return zones; }
	TableZone *getTable() const { return table; }
	void gameEvent(const ServerEventData &event);
	CardDatabase *getDb() const { return db; }
	void showCardMenu(const QPoint &p);
	bool getActive() const { return active; }
	void setActive(bool _active);
};

#endif
