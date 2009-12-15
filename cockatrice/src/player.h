#ifndef PLAYER_H
#define PLAYER_H

#include <QInputDialog>
#include <QPoint>
#include <QMap>
#include "carditem.h"

class Client;
class CardDatabase;
class QMenu;
class QAction;
class ZoneViewZone;
class TabGame;
class Counter;
class ArrowItem;
class CardZone;
class TableZone;
class HandZone;
class ServerInfo_Player;
class ServerInfo_Arrow;
class ServerInfo_Counter;
class GameCommand;
class GameEvent;
class Event_DeckSelect;
class Event_Say;
class Event_ReadyStart;
class Event_Concede;
class Event_Shuffle;
class Event_RollDie;
class Event_CreateArrows;
class Event_DeleteArrow;
class Event_CreateToken;
class Event_SetCardAttr;
class Event_CreateCounters;
class Event_SetCounter;
class Event_DelCounter;
class Event_DumpZone;
class Event_StopDumpZone;
class Event_MoveCard;
class Event_DrawCards;

class Player : public QObject, public QGraphicsItem {
	Q_OBJECT
signals:
	void closeZoneView(ZoneViewZone *zone);
	void toggleZoneView(Player *player, QString zoneName, int number);
	void newCardAdded(CardItem *card);
	// Log events
	void logDeckSelect(Player *player, int deckId);
	void logSay(Player *player, QString message);
	void logReadyStart(Player *player);
	void logConcede(Player *player);
	void logShuffle(Player *player);
	void logRollDie(Player *player, int sides, int roll);
	void logCreateArrow(Player *player, Player *startPlayer, QString startCard, Player *targetPlayer, QString targetCard);
	void logCreateToken(Player *player, QString cardName);
	void logDrawCards(Player *player, int number);
	void logMoveCard(Player *player, QString cardName, CardZone *startZone, int oldX, CardZone *targetZone, int newX);
	void logSetCardCounters(Player *player, QString cardName, int value, int oldValue);
	void logSetTapped(Player *player, QString cardName, bool tapped);
	void logSetCounter(Player *player, QString counterName, int value, int oldValue);
	void logSetDoesntUntap(Player *player, QString cardName, bool doesntUntap);
	void logDumpZone(Player *player, CardZone *zone, int numberCards);
	void logStopDumpZone(Player *player, CardZone *zone);
	
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
	void cardMenuAction();
	void actSetCounters();
private:
	QMenu *playerMenu, *handMenu, *graveMenu, *rfgMenu, *libraryMenu, *sbMenu, *countersMenu, *sayMenu;
	QAction *aMoveHandToTopLibrary, *aMoveHandToBottomLibrary, *aMoveHandToGrave, *aMoveHandToRfg,
		*aMoveGraveToTopLibrary, *aMoveGraveToBottomLibrary, *aMoveGraveToHand, *aMoveGraveToRfg,
		*aMoveRfgToTopLibrary, *aMoveRfgToBottomLibrary, *aMoveRfgToHand, *aMoveRfgToGrave,
		*aViewLibrary, *aViewTopCards, *aViewGraveyard, *aViewRfg, *aViewSideboard,
                *aDrawCard, *aDrawCards, *aMulligan, *aShuffle,
		*aUntapAll, *aRollDie, *aCreateToken;

	typedef void (Player::*CardMenuHandler)(CardItem *card);
	QHash<QAction *, CardMenuHandler> cardMenuHandlers;
	
	QMenu *cardMenu, *moveMenu;
	QAction *aTap, *aUntap, *aDoesntUntap, *aFlip, *aAddCounter, *aRemoveCounter, *aSetCounters,
		*aMoveToTopLibrary, *aMoveToBottomLibrary, *aMoveToGraveyard, *aMoveToExile;


	void actTap(CardItem *card);
	void actUntap(CardItem *card);
	void actDoesntUntap(CardItem *card);
	void actFlip(CardItem *card);
	void actAddCounter(CardItem *card);
	void actRemoveCounter(CardItem *card);
	void actMoveToTopLibrary(CardItem *card);
	void actMoveToBottomLibrary(CardItem *card);
	void actMoveToGraveyard(CardItem *card);
	void actMoveToExile(CardItem *card);

	int defaultNumberTopCards;
	QString name;
	int id;
	bool active;
	bool local;
	
	QMap<QString, CardZone *> zones;
	TableZone *table;
	HandZone *hand;
	
	void setCardAttrHelper(CardItem *card, const QString &aname, const QString &avalue, bool allCards);

	QPixmap bgPixmap;
	QRectF bRect;

	QMap<int, Counter *> counters;
	QMap<int, ArrowItem *> arrows;
	void rearrangeCounters();
	
	void initSayMenu();
	
	void eventDeckSelect(Event_DeckSelect *event);
	void eventSay(Event_Say *event);
	void eventReadyStart(Event_ReadyStart *event);
	void eventConcede(Event_Concede *event);
	void eventShuffle(Event_Shuffle *event);
	void eventRollDie(Event_RollDie *event);
	void eventCreateArrows(Event_CreateArrows *event);
	void eventDeleteArrow(Event_DeleteArrow *event);
	void eventCreateToken(Event_CreateToken *event);
	void eventSetCardAttr(Event_SetCardAttr *event);
	void eventCreateCounters(Event_CreateCounters *event);
	void eventSetCounter(Event_SetCounter *event);
	void eventDelCounter(Event_DelCounter *event);
	void eventDumpZone(Event_DumpZone *event);
	void eventStopDumpZone(Event_StopDumpZone *event);
	void eventMoveCard(Event_MoveCard *event);
	void eventDrawCards(Event_DrawCards *event);
public:
	static const int counterAreaWidth = 65;
	
	enum { Type = typeOther };
	int type() const { return Type; }
	QRectF boundingRect() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	
	void addCard(CardItem *c);
	void addZone(CardZone *z);

	Counter *addCounter(ServerInfo_Counter *counter);
	Counter *addCounter(int counterId, const QString &name, QColor color, int radius, int value);
	void delCounter(int counterId);
	void clearCounters();
	
	ArrowItem *addArrow(ServerInfo_Arrow *arrow);
	ArrowItem *addArrow(int arrowId, CardItem *startCard, CardItem *targetCard, const QColor &color);
	void delArrow(int arrowId);
	void clearArrows();

	Client *client;
	Player(const QString &_name, int _id, bool _local, Client *_client, TabGame *_parent);
	~Player();
	void retranslateUi();
	QMenu *getPlayerMenu() const { return playerMenu; }
	int getId() const { return id; }
	QString getName() const { return name; }
	bool getLocal() const { return local; }
	const QMap<QString, CardZone *> &getZones() const { return zones; }
	const QMap<int, ArrowItem *> &getArrows() const { return arrows; }
	TableZone *getTable() const { return table; }
	void showCardMenu(const QPoint &p);
	bool getActive() const { return active; }
	void setActive(bool _active);
	
	void processPlayerInfo(ServerInfo_Player *info);
	void processGameEvent(GameEvent *event);
	void sendGameCommand(GameCommand *command);
};

#endif
