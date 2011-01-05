#ifndef PLAYER_H
#define PLAYER_H

#include <QInputDialog>
#include <QPoint>
#include <QMap>
#include "carditem.h"

class CardDatabase;
class QMenu;
class QAction;
class ZoneViewZone;
class TabGame;
class AbstractCounter;
class ArrowItem;
class CardZone;
class StackZone;
class TableZone;
class HandZone;
class PlayerTarget;
class ServerInfo_User;
class ServerInfo_Player;
class ServerInfo_Arrow;
class ServerInfo_Counter;
class CommandContainer;
class GameCommand;
class GameEvent;
class GameEventContext;
class Event_Say;
class Event_Shuffle;
class Event_RollDie;
class Event_CreateArrows;
class Event_DeleteArrow;
class Event_CreateToken;
class Event_SetCardAttr;
class Event_SetCardCounter;
class Event_CreateCounters;
class Event_SetCounter;
class Event_DelCounter;
class Event_DumpZone;
class Event_StopDumpZone;
class Event_MoveCard;
class Event_FlipCard;
class Event_DestroyCard;
class Event_AttachCard;
class Event_DrawCards;
class Event_RevealCards;

class Player : public QObject, public QGraphicsItem {
	Q_OBJECT
signals:
	void newCardAdded(AbstractCardItem *card);
	// Log events
	void logSay(Player *player, QString message);
	void logShuffle(Player *player);
	void logRollDie(Player *player, int sides, int roll);
	void logCreateArrow(Player *player, Player *startPlayer, QString startCard, Player *targetPlayer, QString targetCard, bool _playerTarget);
	void logCreateToken(Player *player, QString cardName, QString pt);
	void logDrawCards(Player *player, int number);
	void logMoveCard(Player *player, QString cardName, CardZone *startZone, int oldX, CardZone *targetZone, int newX);
	void logFlipCard(Player *player, QString cardName, bool faceDown);
	void logDestroyCard(Player *player, QString cardName);
	void logAttachCard(Player *player, QString cardName, Player *targetPlayer, QString targetCardName);
	void logUnattachCard(Player *player, QString cardName);
	void logSetCardCounter(Player *player, QString cardName, int counterId, int value, int oldValue);
	void logSetTapped(Player *player, QString cardName, bool tapped);
	void logSetCounter(Player *player, QString counterName, int value, int oldValue);
	void logSetDoesntUntap(Player *player, QString cardName, bool doesntUntap);
	void logSetPT(Player *player, QString cardName, QString newPT);
	void logSetAnnotation(Player *player, QString cardName, QString newAnnotation);
	void logDumpZone(Player *player, CardZone *zone, int numberCards);
	void logStopDumpZone(Player *player, CardZone *zone);
	void logRevealCards(Player *player, CardZone *zone, int cardId, QString cardName, Player *otherPlayer);
	
	void sizeChanged();
public slots:
	void actUntapAll();
	void actRollDie();
	void actCreateToken();
	void actCreateAnotherToken();
	void actShuffle();
	void actDrawCard();
	void actDrawCards();
        void actMulligan();
	void actMoveTopCardsToGrave();
	void actMoveTopCardsToExile();
	void actMoveTopCardToBottom();

	void actViewLibrary();
	void actViewTopCards();
	void actViewGraveyard();
	void actViewRfg();
	void actViewSideboard();
	
	void actSayMessage();

	void actAttach(QAction *action);
	void actUnattach(QAction *action);
	void actSetPT(QAction *action);
	void actSetAnnotation(QAction *action);
	void cardMenuAction(QAction *action);
	void actCardCounterTrigger(QAction *action);

private slots:
	void addPlayer(Player *player);
	void removePlayer(Player *player);
	void playerListActionTriggered();
	
	void updateBgPixmap();
	void updateBoundingRect();
	void rearrangeZones();
private:
	QMenu *playerMenu, *handMenu, *graveMenu, *rfgMenu, *libraryMenu, *sbMenu, *countersMenu, *sayMenu,
		*mRevealLibrary, *mRevealTopCard, *mRevealHand, *mRevealRandomHandCard;
	QList<QMenu *> playerLists;
	QList<QAction *> allPlayersActions;
	QAction *aMoveHandToTopLibrary, *aMoveHandToBottomLibrary, *aMoveHandToGrave, *aMoveHandToRfg,
		*aMoveGraveToTopLibrary, *aMoveGraveToBottomLibrary, *aMoveGraveToHand, *aMoveGraveToRfg,
		*aMoveRfgToTopLibrary, *aMoveRfgToBottomLibrary, *aMoveRfgToHand, *aMoveRfgToGrave,
		*aViewLibrary, *aViewTopCards, *aMoveTopCardsToGrave, *aMoveTopCardsToExile, *aMoveTopCardToBottom,
		*aViewGraveyard, *aViewRfg, *aViewSideboard,
                *aDrawCard, *aDrawCards, *aMulligan, *aShuffle,
		*aUntapAll, *aRollDie, *aCreateToken, *aCreateAnotherToken,
		*aCardMenu;

	bool shortcutsActive;
	int defaultNumberTopCards;
	QString lastTokenName, lastTokenColor, lastTokenPT, lastTokenAnnotation;
	bool lastTokenDestroy;
	ServerInfo_User *userInfo;
	int id;
	bool active;
	bool local;
	bool mirrored;
	
	bool dialogSemaphore;
	bool clearCardsToDelete();
	QList<CardItem *> cardsToDelete;
	
	QMap<QString, CardZone *> zones;
	StackZone *stack;
	TableZone *table;
	HandZone *hand;
	PlayerTarget *playerTarget;
	
	void setCardAttrHelper(CardItem *card, const QString &aname, const QString &avalue, bool allCards);

	QPixmap bgPixmap;
	QRectF bRect;

	QMap<int, AbstractCounter *> counters;
	QMap<int, ArrowItem *> arrows;
	void rearrangeCounters();
	
	void initSayMenu();
	
	void eventSay(Event_Say *event);
	void eventShuffle(Event_Shuffle *event);
	void eventRollDie(Event_RollDie *event);
	void eventCreateArrows(Event_CreateArrows *event);
	void eventDeleteArrow(Event_DeleteArrow *event);
	void eventCreateToken(Event_CreateToken *event);
	void eventSetCardAttr(Event_SetCardAttr *event);
	void eventSetCardCounter(Event_SetCardCounter *event);
	void eventCreateCounters(Event_CreateCounters *event);
	void eventSetCounter(Event_SetCounter *event);
	void eventDelCounter(Event_DelCounter *event);
	void eventDumpZone(Event_DumpZone *event);
	void eventStopDumpZone(Event_StopDumpZone *event);
	void eventMoveCard(Event_MoveCard *event);
	void eventFlipCard(Event_FlipCard *event);
	void eventDestroyCard(Event_DestroyCard *event);
	void eventAttachCard(Event_AttachCard *event);
	void eventDrawCards(Event_DrawCards *event);
	void eventRevealCards(Event_RevealCards *event);
public:
	static const int counterAreaWidth = 55;
	
	enum { Type = typeOther };
	int type() const { return Type; }
	QRectF boundingRect() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	
	void playCard(CardItem *c, bool faceDown, bool tapped);
	void addCard(CardItem *c);
	void deleteCard(CardItem *c);
	void addZone(CardZone *z);

	AbstractCounter *addCounter(ServerInfo_Counter *counter);
	AbstractCounter *addCounter(int counterId, const QString &name, QColor color, int radius, int value);
	void delCounter(int counterId);
	void clearCounters();
	
	ArrowItem *addArrow(ServerInfo_Arrow *arrow);
	ArrowItem *addArrow(int arrowId, CardItem *startCard, ArrowTarget *targetItem, const QColor &color);
	void delArrow(int arrowId);
	void removeArrow(ArrowItem *arrow);
	void clearArrows();
	PlayerTarget *getPlayerTarget() const { return playerTarget; }

	Player(ServerInfo_User *info, int _id, bool _local, TabGame *_parent);
	~Player();
	void retranslateUi();
	QMenu *getPlayerMenu() const { return playerMenu; }
	int getId() const { return id; }
	QString getName() const;
	ServerInfo_User *getUserInfo() const { return userInfo; }
	bool getLocal() const { return local; }
	bool getMirrored() const { return mirrored; }
	const QMap<QString, CardZone *> &getZones() const { return zones; }
	const QMap<int, ArrowItem *> &getArrows() const { return arrows; }
	TableZone *getTable() const { return table; }
	void setCardMenu(QMenu *menu);
	QMenu *getCardMenu() const;
	bool getActive() const { return active; }
	void setActive(bool _active);
	void setShortcutsActive();
	void setShortcutsInactive();
	
	qreal getMinimumWidth() const;
	void setMirrored(bool _mirrored);
	void processSceneSizeChange(const QSizeF &newSize);
	
	void processPlayerInfo(ServerInfo_Player *info);
	void processCardAttachment(ServerInfo_Player *info);
	
	void processGameEvent(GameEvent *event, GameEventContext *context);
	void sendGameCommand(GameCommand *command);
	void sendCommandContainer(CommandContainer *cont);
};

#endif
