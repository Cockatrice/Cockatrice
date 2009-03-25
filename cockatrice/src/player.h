#ifndef PLAYER_H
#define PLAYER_H

#include <QInputDialog>
#include <QPoint>
#include "zonelist.h"
#include "counterlist.h"
#include "servereventdata.h"

class Client;
class CardDatabase;
class QMenu;
class QAction;

class Player : public QObject {
	Q_OBJECT
signals:
	void moveCard(int cardId, QString startZone, QString targetZone, int x, int y);
	void hoverCard(QString name);
	void addZoneView(Player *player, QString zoneName, int number);
	void sigShowCardMenu(QPoint p);
	// Log events
	void logMoveCard(QString playerName, QString cardName, QString startZone, QString targetZone);
	void logCreateToken(QString playerName, QString cardName);
	void logSetCardCounters(QString playerName, QString cardName, int value, int oldValue);
	void logSetTapped(QString playerName, QString cardName, bool tapped);
	void logSetCounter(QString playerName, QString counterName, int value, int oldValue);
private slots:
	void actMoveHandToTopLibrary();
	void actMoveHandToBottomLibrary();

	void actViewLibrary();
	void actViewTopCards();

	void actViewGraveyard();

	void actViewRfg();

	void actViewSideboard();
private:
	QMenu *playerMenu;
	QAction *aMoveHandToTopLibrary, *aMoveHandToBottomLibrary,
		*aViewLibrary, *aViewTopCards, *aViewGraveyard, *aViewRfg, *aViewSideboard;

	int defaultNumberTopCards;
	QString name;
	int id;
	QPointF base;
	bool local;
	ZoneList zones;
	CounterList counters;
	CardDatabase *db;
public:
	Client *client;
	void addZone(CardZone *z);
	void addCounter(Counter *c);
	Player(const QString &_name, int _id, QPointF _base, bool _local, CardDatabase *_db, Client *_client);
	~Player();
	QMenu *getPlayerMenu() const { return playerMenu; }
	int getId() const { return id; }
	QString getName() const { return name; }
	bool getLocal() const { return local; }
	const ZoneList *const getZones() const { return &zones; }
	const CounterList *const getCounters() const { return &counters; }
	void gameEvent(ServerEventData *event);
	void hoverCardEvent(CardItem *card);
	CardDatabase *getDb() const { return db; }
	void showCardMenu(const QPoint &p);
};

#endif
