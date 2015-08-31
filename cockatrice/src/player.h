#ifndef PLAYER_H
#define PLAYER_H

#include <QInputDialog>
#include <QPoint>
#include <QMap>
#include "abstractgraphicsitem.h"
#include "pb/game_event.pb.h"
#include "pb/card_attributes.pb.h"

namespace google { namespace protobuf { class Message; } }
class CardDatabase;
class DeckLoader;
class QMenu;
class QAction;
class ZoneViewZone;
class TabGame;
class AbstractCounter;
class AbstractCardItem;
class CardItem;
class ArrowTarget;
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
class Event_ConnectionStateChanged;
class Event_GameSay;
class Event_Shuffle;
class Event_RollDie;
class Event_CreateArrow;
class Event_DeleteArrow;
class Event_CreateToken;
class Event_SetCardAttr;
class Event_SetCardCounter;
class Event_CreateCounter;
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
class Event_ChangeZoneProperties;
class PendingCommand;

class PlayerArea : public QObject, public QGraphicsItem {
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
private:
    QRectF bRect;
private slots:
    void updateBg();
public:
    enum { Type = typeOther };
    int type() const { return Type; }
    
    PlayerArea(QGraphicsItem *parent = 0);
    QRectF boundingRect() const { return bRect; }
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    
    void setSize(qreal width, qreal height);
};

class Player : public QObject, public QGraphicsItem {
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
signals:
    void openDeckEditor(const DeckLoader *deck);
    void newCardAdded(AbstractCardItem *card);
    // Log events
    void logSay(Player *player, QString message);
    void logShuffle(Player *player, CardZone *zone);
    void logRollDie(Player *player, int sides, int roll);
    void logCreateArrow(Player *player, Player *startPlayer, QString startCard, Player *targetPlayer, QString targetCard, bool _playerTarget);
    void logCreateToken(Player *player, QString cardName, QString pt);
    void logDrawCards(Player *player, int number);
    void logUndoDraw(Player *player, QString cardName);
    void logMoveCard(Player *player, CardItem *card, CardZone *startZone, int oldX, CardZone *targetZone, int newX);
    void logFlipCard(Player *player, QString cardName, bool faceDown);
    void logDestroyCard(Player *player, QString cardName);
    void logAttachCard(Player *player, QString cardName, Player *targetPlayer, QString targetCardName);
    void logUnattachCard(Player *player, QString cardName);
    void logSetCardCounter(Player *player, QString cardName, int counterId, int value, int oldValue);
    void logSetTapped(Player *player, CardItem *card, bool tapped);
    void logSetCounter(Player *player, QString counterName, int value, int oldValue);
    void logSetDoesntUntap(Player *player, CardItem *card, bool doesntUntap);
    void logSetPT(Player *player, CardItem *card, QString newPT);
    void logSetAnnotation(Player *player, CardItem *card, QString newAnnotation);
    void logDumpZone(Player *player, CardZone *zone, int numberCards);
    void logStopDumpZone(Player *player, CardZone *zone);
    void logRevealCards(Player *player, CardZone *zone, int cardId, QString cardName, Player *otherPlayer, bool faceDown);
    void logAlwaysRevealTopCard(Player *player, CardZone *zone, bool reveal);
    
    void sizeChanged();
    void gameConceded();
public slots:
    void actUntapAll();
    void actRollDie();
    void actCreateToken();
    void actCreateAnotherToken();
    void actShuffle();
    void actDrawCard();
    void actDrawCards();
    void actUndoDraw();
    void actMulligan();
    void actMoveTopCardToPlayFaceDown();
    void actMoveTopCardsToGrave();
    void actMoveTopCardsToExile();
    void actMoveTopCardToBottom();
    void actMoveBottomCardToGrave();

    void actViewLibrary();
    void actViewTopCards();
    void actAlwaysRevealTopCard();
    void actViewGraveyard();
    void actViewRfg();
    void actViewSideboard();
    
    void actSayMessage();
private slots:
    void addPlayer(Player *player);
    void removePlayer(Player *player);
    void playerListActionTriggered();
    
    void updateBoundingRect();
    void rearrangeZones();
    
    void actOpenDeckInDeckEditor();
    void actCreatePredefinedToken();
    void actCreateRelatedCard();
    void cardMenuAction();
    void actCardCounterTrigger();
    void actAttach();
    void actUnattach();
    void actDrawArrow();
    void actIncPT(int deltaP, int deltaT);
    void actSetPT();
    void actIncP();
    void actDecP();
    void actIncT();
    void actDecT();
    void actIncPT();
    void actDecPT();
    void actSetAnnotation();
    void actPlay();
    void actHide();
    void actPlayFacedown();
    void refreshShortcuts();

private:
    TabGame *game;
    QMenu *playerMenu, *handMenu, *moveHandMenu, *graveMenu, *moveGraveMenu, *rfgMenu, *moveRfgMenu, *libraryMenu, *sbMenu, *countersMenu, *sayMenu, *createPredefinedTokenMenu,
        *mRevealLibrary, *mRevealTopCard, *mRevealHand, *mRevealRandomHandCard;
    QList<QMenu *> playerLists;
    QList<QAction *> allPlayersActions;
    QAction *aMoveHandToTopLibrary, *aMoveHandToBottomLibrary, *aMoveHandToGrave, *aMoveHandToRfg,
        *aMoveGraveToTopLibrary, *aMoveGraveToBottomLibrary, *aMoveGraveToHand, *aMoveGraveToRfg,
        *aMoveRfgToTopLibrary, *aMoveRfgToBottomLibrary, *aMoveRfgToHand, *aMoveRfgToGrave,
        *aViewLibrary, *aViewTopCards, *aAlwaysRevealTopCard, *aOpenDeckInDeckEditor, *aMoveTopCardsToGrave, *aMoveTopCardsToExile, *aMoveTopCardToBottom,
        *aViewGraveyard, *aViewRfg, *aViewSideboard,
                *aDrawCard, *aDrawCards, *aUndoDraw, *aMulligan, *aShuffle, *aMoveTopToPlayFaceDown,
        *aUntapAll, *aRollDie, *aCreateToken, *aCreateAnotherToken,
        *aCardMenu, *aMoveBottomCardToGrave;
    
    QList<QAction *> aAddCounter, aSetCounter, aRemoveCounter;
    QAction *aPlay, *aPlayFacedown,
        *aHide,
        *aTap, *aUntap, *aDoesntUntap, *aAttach, *aUnattach, *aDrawArrow, *aSetPT, *aIncP, *aDecP, *aIncT, *aDecT, *aIncPT, *aDecPT, *aSetAnnotation, *aFlip, *aPeek, *aClone,
        *aMoveToTopLibrary, *aMoveToBottomLibrary, *aMoveToHand, *aMoveToGraveyard, *aMoveToExile;

    bool shortcutsActive;
    int defaultNumberTopCards;
    QString lastTokenName, lastTokenColor, lastTokenPT, lastTokenAnnotation;
    bool lastTokenDestroy;
    int lastTokenTableRow;
    ServerInfo_User *userInfo;
    int id;
    bool active;
    bool local;
    bool mirrored;
    bool handVisible;
    bool conceded;
    
    bool dialogSemaphore;
    bool clearCardsToDelete();
    QList<CardItem *> cardsToDelete;
    
    DeckLoader *deck;
    QStringList predefinedTokens;
    
    PlayerArea *playerArea;
    QMap<QString, CardZone *> zones;
    StackZone *stack;
    TableZone *table;
    HandZone *hand;
    PlayerTarget *playerTarget;
    
    void setCardAttrHelper(const GameEventContext &context, CardItem *card, CardAttribute attribute, const QString &avalue, bool allCards);

    QRectF bRect;

    QMap<int, AbstractCounter *> counters;
    QMap<int, ArrowItem *> arrows;
    void rearrangeCounters();
    
    void initSayMenu();
    
    void eventConnectionStateChanged(const Event_ConnectionStateChanged &event);
    void eventGameSay(const Event_GameSay &event);
    void eventShuffle(const Event_Shuffle &event);
    void eventRollDie(const Event_RollDie &event);
    void eventCreateArrow(const Event_CreateArrow &event);
    void eventDeleteArrow(const Event_DeleteArrow &event);
    void eventCreateToken(const Event_CreateToken &event);
    void eventSetCardAttr(const Event_SetCardAttr &event, const GameEventContext &context);
    void eventSetCardCounter(const Event_SetCardCounter &event);
    void eventCreateCounter(const Event_CreateCounter &event);
    void eventSetCounter(const Event_SetCounter &event);
    void eventDelCounter(const Event_DelCounter &event);
    void eventDumpZone(const Event_DumpZone &event);
    void eventStopDumpZone(const Event_StopDumpZone &event);
    void eventMoveCard(const Event_MoveCard &event, const GameEventContext &context);
    void eventFlipCard(const Event_FlipCard &event);
    void eventDestroyCard(const Event_DestroyCard &event);
    void eventAttachCard(const Event_AttachCard &event);
    void eventDrawCards(const Event_DrawCards &event);
    void eventRevealCards(const Event_RevealCards &event);
    void eventChangeZoneProperties(const Event_ChangeZoneProperties &event);
public:
    static const int counterAreaWidth = 55;
    enum CardMenuActionType { cmTap, cmUntap, cmDoesntUntap, cmFlip, cmPeek, cmClone, cmMoveToTopLibrary, cmMoveToBottomLibrary, cmMoveToHand, cmMoveToGraveyard, cmMoveToExile };
    
    enum { Type = typeOther };
    int type() const { return Type; }
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    
    void playCard(CardItem *c, bool faceDown, bool tapped);
    void addCard(CardItem *c);
    void deleteCard(CardItem *c);
    void addZone(CardZone *z);

    AbstractCounter *addCounter(const ServerInfo_Counter &counter);
    AbstractCounter *addCounter(int counterId, const QString &name, QColor color, int radius, int value);
    void delCounter(int counterId);
    void clearCounters();
    
    ArrowItem *addArrow(const ServerInfo_Arrow &arrow);
    ArrowItem *addArrow(int arrowId, CardItem *startCard, ArrowTarget *targetItem, const QColor &color);
    void delArrow(int arrowId);
    void removeArrow(ArrowItem *arrow);
    void clearArrows();
    PlayerTarget *getPlayerTarget() const { return playerTarget; }

    Player(const ServerInfo_User &info, int _id, bool _local, TabGame *_parent);
    ~Player();
    void retranslateUi();
    void clear();
    TabGame *getGame() const { return game; }
    void setDeck(const DeckLoader &_deck);
    QMenu *getPlayerMenu() const { return playerMenu; }
    int getId() const { return id; }
    QString getName() const;
    ServerInfo_User *getUserInfo() const { return userInfo; }
    bool getLocal() const { return local; }
    bool getMirrored() const { return mirrored; }
    const QMap<QString, CardZone *> &getZones() const { return zones; }
    const QMap<int, ArrowItem *> &getArrows() const { return arrows; }
    void setCardMenu(QMenu *menu);
    QMenu *getCardMenu() const;
    void updateCardMenu(CardItem *card);
    bool getActive() const { return active; }
    void setActive(bool _active);
    void setShortcutsActive();
    void setShortcutsInactive();
    void updateZones();
    
    void setConceded(bool _conceded);
    bool getConceded() const { return conceded; }

    void setGameStarted();
    
    qreal getMinimumWidth() const;
    void setMirrored(bool _mirrored);
    void processSceneSizeChange(int newPlayerWidth);
    
    void processPlayerInfo(const ServerInfo_Player &info);
    void processCardAttachment(const ServerInfo_Player &info);
    
    void processGameEvent(GameEvent::GameEventType type, const GameEvent &event, const GameEventContext &context);

    PendingCommand *prepareGameCommand(const ::google::protobuf::Message &cmd);
    PendingCommand *prepareGameCommand(const QList< const ::google::protobuf::Message * > &cmdList);
    void sendGameCommand(PendingCommand *pend);
    void sendGameCommand(const google::protobuf::Message &command);
};

#endif
