#ifndef PLAYER_H
#define PLAYER_H

#include "../../client/tearoff_menu.h"
#include "../../dialogs/dlg_create_token.h"
#include "../board/abstract_graphics_item.h"
#include "../cards/card_info.h"
#include "../filters/filter_string.h"
#include "pb/card_attributes.pb.h"
#include "pb/game_event.pb.h"

#include <QInputDialog>
#include <QLoggingCategory>
#include <QMap>
#include <QPoint>
#include <QTimer>

inline Q_LOGGING_CATEGORY(PlayerLog, "player");

namespace google
{
namespace protobuf
{
class Message;
}
} // namespace google
class AbstractCardItem;
class AbstractCounter;
class ArrowItem;
class ArrowTarget;
class CardDatabase;
class CardItem;
class CardZone;
class CommandContainer;
class Command_MoveCard;
class DeckLoader;
class Event_AttachCard;
class Event_ChangeZoneProperties;
class Event_CreateArrow;
class Event_CreateCounter;
class Event_CreateToken;
class Event_DelCounter;
class Event_DeleteArrow;
class Event_DestroyCard;
class Event_DrawCards;
class Event_DumpZone;
class Event_FlipCard;
class Event_GameSay;
class Event_MoveCard;
class Event_RevealCards;
class Event_RollDie;
class Event_SetCardAttr;
class Event_SetCardCounter;
class Event_SetCounter;
class Event_Shuffle;
class GameCommand;
class GameEvent;
class GameEventContext;
class HandZone;
class PendingCommand;
class PlayerTarget;
class QAction;
class QMenu;
class ServerInfo_Arrow;
class ServerInfo_Counter;
class ServerInfo_Player;
class ServerInfo_User;
class StackZone;
class TabGame;
class TableZone;
class ZoneViewZone;

const int MAX_TOKENS_PER_DIALOG = 99;

/**
 * The entire graphical area belonging to a single player.
 */
class PlayerArea : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
private:
    QRectF bRect;
    int playerZoneId;
private slots:
    void updateBg();

public:
    enum
    {
        Type = typeOther
    };
    int type() const override
    {
        return Type;
    }

    explicit PlayerArea(QGraphicsItem *parent = nullptr);
    QRectF boundingRect() const override
    {
        return bRect;
    }
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void setSize(qreal width, qreal height);

    void setPlayerZoneId(int _playerZoneId);
    int getPlayerZoneId() const
    {
        return playerZoneId;
    }
};

class Player : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
signals:
    void openDeckEditor(const DeckLoader *deck);
    void newCardAdded(AbstractCardItem *card);
    // Log events
    void logSay(Player *player, QString message);
    void logShuffle(Player *player, CardZone *zone, int start, int end);
    void logRollDie(Player *player, int sides, const QList<uint> &rolls);
    void logCreateArrow(Player *player,
                        Player *startPlayer,
                        QString startCard,
                        Player *targetPlayer,
                        QString targetCard,
                        bool _playerTarget);
    void logCreateToken(Player *player, QString cardName, QString pt, bool faceDown);
    void logDrawCards(Player *player, int number, bool deckIsEmpty);
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
    void logDumpZone(Player *player, CardZone *zone, int numberCards, bool isReversed = false);
    void logRevealCards(Player *player,
                        CardZone *zone,
                        int cardId,
                        QString cardName,
                        Player *otherPlayer,
                        bool faceDown,
                        int amount,
                        bool isLentToAnotherPlayer = false);
    void logAlwaysRevealTopCard(Player *player, CardZone *zone, bool reveal);
    void logAlwaysLookAtTopCard(Player *player, CardZone *zone, bool reveal);

    void sizeChanged();
    void playerCountChanged();
public slots:
    void actUntapAll();
    void actRollDie();
    void actCreateToken();
    void actCreateAnotherToken();
    void actShuffle();
    void actShuffleTop();
    void actShuffleBottom();
    void actDrawCard();
    void actDrawCards();
    void actUndoDraw();
    void actMulligan();

    void actPlay();
    void actPlayFacedown();
    void actHide();

    void actMoveTopCardToPlay();
    void actMoveTopCardToPlayFaceDown();
    void actMoveTopCardToGrave();
    void actMoveTopCardToExile();
    void actMoveTopCardsToGrave();
    void actMoveTopCardsToExile();
    void actMoveTopCardsUntil();
    void actMoveTopCardToBottom();
    void actDrawBottomCard();
    void actDrawBottomCards();
    void actMoveBottomCardToPlay();
    void actMoveBottomCardToPlayFaceDown();
    void actMoveBottomCardToGrave();
    void actMoveBottomCardToExile();
    void actMoveBottomCardsToGrave();
    void actMoveBottomCardsToExile();
    void actMoveBottomCardToTop();

    void actSelectAll();
    void actSelectRow();
    void actSelectColumn();

    void actViewLibrary();
    void actViewHand();
    void actViewTopCards();
    void actViewBottomCards();
    void actAlwaysRevealTopCard();
    void actAlwaysLookAtTopCard();
    void actViewGraveyard();
    void actRevealRandomGraveyardCard();
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
    void actCreateAllRelatedCards();
    void cardMenuAction();
    void actMoveCardXCardsFromTop();
    void actCardCounterTrigger();
    void actAttach();
    void actUnattach();
    void actDrawArrow();
    void actIncPT(int deltaP, int deltaT);
    void actResetPT();
    void actSetPT();
    void actIncP();
    void actDecP();
    void actIncT();
    void actDecT();
    void actIncPT();
    void actDecPT();
    void actFlowP();
    void actFlowT();
    void actSetAnnotation();
    void actReveal(QAction *action);
    void refreshShortcuts();
    void actSortHand();
    void initSayMenu();

public:
    enum EventProcessingOption
    {
        SKIP_REVEAL_WINDOW = 0x0001,
        SKIP_TAP_ANIMATION = 0x0002
    };
    Q_DECLARE_FLAGS(EventProcessingOptions, EventProcessingOption)

private:
    TabGame *game;
    QMenu *sbMenu, *countersMenu, *sayMenu, *createPredefinedTokenMenu, *mRevealLibrary, *mLendLibrary, *mRevealTopCard,
        *mRevealHand, *mRevealRandomHandCard, *mRevealRandomGraveyardCard, *mCustomZones, *mCardCounters;
    TearOffMenu *moveGraveMenu, *moveRfgMenu, *graveMenu, *moveHandMenu, *handMenu, *libraryMenu, *topLibraryMenu,
        *bottomLibraryMenu, *rfgMenu, *playerMenu;
    QList<QMenu *> playerLists;
    QList<QMenu *> singlePlayerLists;
    QList<QAction *> allPlayersActions;
    QList<QPair<QString, int>> playersInfo;
    QAction *aMoveHandToTopLibrary, *aMoveHandToBottomLibrary, *aMoveHandToGrave, *aMoveHandToRfg,
        *aMoveGraveToTopLibrary, *aMoveGraveToBottomLibrary, *aMoveGraveToHand, *aMoveGraveToRfg, *aMoveRfgToTopLibrary,
        *aMoveRfgToBottomLibrary, *aMoveRfgToHand, *aMoveRfgToGrave, *aViewHand, *aViewLibrary, *aViewTopCards,
        *aViewBottomCards, *aAlwaysRevealTopCard, *aAlwaysLookAtTopCard, *aOpenDeckInDeckEditor,
        *aMoveTopCardToGraveyard, *aMoveTopCardToExile, *aMoveTopCardsToGraveyard, *aMoveTopCardsToExile,
        *aMoveTopCardsUntil, *aMoveTopCardToBottom, *aViewGraveyard, *aViewRfg, *aViewSideboard, *aDrawCard,
        *aDrawCards, *aUndoDraw, *aMulligan, *aShuffle, *aShuffleTopCards, *aShuffleBottomCards, *aMoveTopToPlay,
        *aMoveTopToPlayFaceDown, *aUntapAll, *aRollDie, *aCreateToken, *aCreateAnotherToken, *aMoveBottomToPlay,
        *aMoveBottomToPlayFaceDown, *aMoveBottomCardToTop, *aMoveBottomCardToGraveyard, *aMoveBottomCardToExile,
        *aMoveBottomCardsToGraveyard, *aMoveBottomCardsToExile, *aDrawBottomCard, *aDrawBottomCards;

    QAction *aCardMenu;
    QList<QAction *> aAddCounter, aSetCounter, aRemoveCounter;
    QAction *aPlay, *aPlayFacedown, *aHide, *aTap, *aDoesntUntap, *aAttach, *aUnattach, *aDrawArrow, *aSetPT, *aResetPT,
        *aIncP, *aDecP, *aIncT, *aDecT, *aIncPT, *aDecPT, *aFlowP, *aFlowT, *aSetAnnotation, *aFlip, *aPeek, *aClone,
        *aMoveToTopLibrary, *aMoveToBottomLibrary, *aMoveToHand, *aMoveToGraveyard, *aMoveToExile,
        *aMoveToXfromTopOfLibrary, *aSelectAll, *aSelectRow, *aSelectColumn, *aSortHand, *aIncrementAllCardCounters;

    bool movingCardsUntil;
    QTimer *moveTopCardTimer;
    QStringList movingCardsUntilExprs = {};
    int movingCardsUntilNumberOfHits = 1;
    bool movingCardsUntilAutoPlay = false;
    FilterString movingCardsUntilFilter;
    int movingCardsUntilCounter = 0;
    void stopMoveTopCardsUntil();

    bool shortcutsActive;
    int defaultNumberTopCards = 1;
    int defaultNumberTopCardsToPlaceBelow = 1;
    int defaultNumberBottomCards = 1;
    int defaultNumberDieRoll = 20;

    TokenInfo lastTokenInfo;
    int lastTokenTableRow;

    ServerInfo_User *userInfo;
    int id;
    bool active;
    bool local;
    bool judge;
    bool mirrored;
    bool handVisible;
    bool conceded;
    int zoneId;

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

    void setCardAttrHelper(const GameEventContext &context,
                           CardItem *card,
                           CardAttribute attribute,
                           const QString &avalue,
                           bool allCards,
                           EventProcessingOptions options);
    void addRelatedCardActions(const CardItem *card, QMenu *cardMenu);
    void addRelatedCardView(const CardItem *card, QMenu *cardMenu);
    void createCard(const CardItem *sourceCard,
                    const QString &dbCardName,
                    CardRelation::AttachType attach = CardRelation::DoesNotAttach,
                    bool persistent = false);
    bool createRelatedFromRelation(const CardItem *sourceCard, const CardRelation *cardRelation);
    void moveOneCardUntil(CardItem *card);
    void addPlayerToList(QMenu *playerList, Player *player);
    static void removePlayerFromList(QMenu *playerList, Player *player);

    void playSelectedCards(bool faceDown = false);

    QRectF bRect;

    QMap<int, AbstractCounter *> counters;
    QMap<int, ArrowItem *> arrows;
    void rearrangeCounters();

    void initContextualPlayersMenu(QMenu *menu);

    // void eventConnectionStateChanged(const Event_ConnectionStateChanged &event);
    void eventGameSay(const Event_GameSay &event);
    void eventShuffle(const Event_Shuffle &event);
    void eventRollDie(const Event_RollDie &event);
    void eventCreateArrow(const Event_CreateArrow &event);
    void eventDeleteArrow(const Event_DeleteArrow &event);
    void eventCreateToken(const Event_CreateToken &event);
    void
    eventSetCardAttr(const Event_SetCardAttr &event, const GameEventContext &context, EventProcessingOptions options);
    void eventSetCardCounter(const Event_SetCardCounter &event);
    void eventCreateCounter(const Event_CreateCounter &event);
    void eventSetCounter(const Event_SetCounter &event);
    void eventDelCounter(const Event_DelCounter &event);
    void eventDumpZone(const Event_DumpZone &event);
    void eventMoveCard(const Event_MoveCard &event, const GameEventContext &context);
    void eventFlipCard(const Event_FlipCard &event);
    void eventDestroyCard(const Event_DestroyCard &event);
    void eventAttachCard(const Event_AttachCard &event);
    void eventDrawCards(const Event_DrawCards &event);
    void eventRevealCards(const Event_RevealCards &event, EventProcessingOptions options);
    void eventChangeZoneProperties(const Event_ChangeZoneProperties &event);
    void cmdSetTopCard(Command_MoveCard &cmd);
    void cmdSetBottomCard(Command_MoveCard &cmd);

    QVariantList parsePT(const QString &pt);

public:
    static const int counterAreaWidth = 55;
    enum CardMenuActionType
    {
        cmTap,
        cmUntap,
        cmDoesntUntap,
        cmFlip,
        cmPeek,
        cmClone,
        cmMoveToTopLibrary,
        cmMoveToBottomLibrary,
        cmMoveToHand,
        cmMoveToGraveyard,
        cmMoveToExile
    };
    enum CardsToReveal
    {
        RANDOM_CARD_FROM_ZONE = -2
    };

    enum
    {
        Type = typeOther
    };
    int type() const override
    {
        return Type;
    }
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void playCard(CardItem *c, bool faceDown);
    void playCardToTable(const CardItem *c, bool faceDown);
    void addCard(CardItem *c);
    void deleteCard(CardItem *c);

    template <typename T> T *addZone(T *zone)
    {
        zones.insert(zone->getName(), zone);
        return zone;
    }

    AbstractCounter *addCounter(const ServerInfo_Counter &counter);
    AbstractCounter *addCounter(int counterId, const QString &name, QColor color, int radius, int value);
    void delCounter(int counterId);
    void clearCounters();
    void incrementAllCardCounters();

    ArrowItem *addArrow(const ServerInfo_Arrow &arrow);
    ArrowItem *addArrow(int arrowId, CardItem *startCard, ArrowTarget *targetItem, const QColor &color);
    void delArrow(int arrowId);
    void removeArrow(ArrowItem *arrow);
    void clearArrows();
    PlayerTarget *getPlayerTarget() const
    {
        return playerTarget;
    }

    Player(const ServerInfo_User &info, int _id, bool _local, bool _judge, TabGame *_parent);
    ~Player() override;

    void retranslateUi();
    void clear();
    TabGame *getGame() const
    {
        return game;
    }
    void setDeck(const DeckLoader &_deck);
    QMenu *getPlayerMenu() const
    {
        return playerMenu;
    }
    int getId() const
    {
        return id;
    }
    QString getName() const;
    ServerInfo_User *getUserInfo() const
    {
        return userInfo;
    }
    bool getLocal() const
    {
        return local;
    }
    bool getLocalOrJudge() const
    {
        return local || judge;
    }
    bool getJudge() const
    {
        return judge;
    }
    bool getMirrored() const
    {
        return mirrored;
    }
    int getZoneId() const
    {
        return zoneId;
    }
    void setZoneId(int _zoneId);
    const QMap<QString, CardZone *> &getZones() const
    {
        return zones;
    }
    const QMap<int, ArrowItem *> &getArrows() const
    {
        return arrows;
    }
    void setCardMenu(QMenu *menu);
    QMenu *getCardMenu() const;
    void updateCardMenu(const CardItem *card);
    bool getActive() const
    {
        return active;
    }
    void setActive(bool _active);
    void setShortcutsActive();
    void setShortcutsInactive();
    void updateZones();

    void setConceded(bool _conceded);
    bool getConceded() const
    {
        return conceded;
    }

    void setGameStarted();

    qreal getMinimumWidth() const;
    void setMirrored(bool _mirrored);
    void processSceneSizeChange(int newPlayerWidth);

    void processPlayerInfo(const ServerInfo_Player &info);
    void processCardAttachment(const ServerInfo_Player &info);

    void processGameEvent(GameEvent::GameEventType type,
                          const GameEvent &event,
                          const GameEventContext &context,
                          EventProcessingOptions options);

    PendingCommand *prepareGameCommand(const ::google::protobuf::Message &cmd);
    PendingCommand *prepareGameCommand(const QList<const ::google::protobuf::Message *> &cmdList);
    void sendGameCommand(PendingCommand *pend);
    void sendGameCommand(const google::protobuf::Message &command);

    void setLastToken(CardInfoPtr cardInfo);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Player::EventProcessingOptions)

class AnnotationDialog : public QInputDialog
{
    Q_OBJECT
    void keyPressEvent(QKeyEvent *e) override;

public:
    explicit AnnotationDialog(QWidget *parent) : QInputDialog(parent)
    {
    }
};

#endif
