/**
 * @file player_actions.h
 *  @ingroup GameLogicActions
 * @ingroup GameLogicPlayers
 */
//! \todo Document this file.

#ifndef COCKATRICE_PLAYER_ACTIONS_H
#define COCKATRICE_PLAYER_ACTIONS_H
#include "../dialogs/dlg_create_token.h"
#include "../dialogs/dlg_move_top_cards_until.h"
#include "card_menu_action_type.h"
#include "event_processing_options.h"
#include "player_logic.h"

#include <QMenu>
#include <QObject>
#include <libcockatrice/card/relation/card_relation_type.h>
#include <libcockatrice/filters/filter_string.h>

namespace google
{
namespace protobuf
{
class Message;
}
} // namespace google

class CardItem;
class Command_MoveCard;
class GameEventContext;
class PendingCommand;
class PlayerLogic;
class PlayerActions : public QObject
{
    Q_OBJECT

public:
    enum CardsToReveal
    {
        RANDOM_CARD_FROM_ZONE = -2
    };

    explicit PlayerActions(PlayerLogic *player);

    void sendGameCommand(PendingCommand *pend);
    void sendGameCommand(const google::protobuf::Message &command);

    PendingCommand *prepareGameCommand(const ::google::protobuf::Message &cmd);
    PendingCommand *prepareGameCommand(const QList<const ::google::protobuf::Message *> &cmdList);

    void moveOneCardUntil(CardItem *card);
    void stopMoveTopCardsUntil();

    [[nodiscard]] bool isMovingCardsUntil() const
    {
        return movingCardsUntil;
    }

signals:
    void requestZoneViewToggle(const QString &zoneName, int numberCards, bool isReversed = false);
    void requestSortHand(const QList<CardList::SortOption> &options);
    void requestEnableAndSetCreateAnotherTokenAction(const QString &lastTokenName);
    void requestSetLastToken(CardInfoPtr lastToken);

public slots:
    void setLastToken(CardInfoPtr cardInfo);
    void setLastTokenInfo(CardInfoPtr cardInfo);
    void playCard(CardItem *c, bool faceDown);
    void playCardToTable(const CardItem *c, bool faceDown);

    void actUntapAll();
    void actRollDie();
    void actFlipCoin();
    void actCreateToken(const QStringList &predefinedTokens);
    void actCreateAnotherToken();
    void actShuffle();
    void actShuffleTop();
    void actShuffleBottom();
    void actDrawCard();
    void actDrawCards();
    void actUndoDraw();
    void actMulligan();
    void actMulliganSameSize();
    void actMulliganMinusOne();
    void doMulligan(int number);

    void actPlay(QList<CardItem *> selectedCards);
    void actPlayFacedown(QList<CardItem *> selectedCards);
    void actHide(QList<CardItem *> selectedCards);

    void actMoveTopCardToPlay();
    void actMoveTopCardToPlayFaceDown();
    void actMoveTopCardToGrave();
    void actMoveTopCardToExile();
    void actMoveTopCardsToGrave();
    void actMoveTopCardsToGraveFaceDown();
    void actMoveTopCardsToExile();
    void actMoveTopCardsToExileFaceDown();
    void actMoveTopCardsUntil();
    void actMoveTopCardToBottom();
    void actDrawBottomCard();
    void actDrawBottomCards();
    void actMoveBottomCardToPlay();
    void actMoveBottomCardToPlayFaceDown();
    void actMoveBottomCardToGrave();
    void actMoveBottomCardToExile();
    void actMoveBottomCardsToGrave();
    void actMoveBottomCardsToGraveFaceDown();
    void actMoveBottomCardsToExile();
    void actMoveBottomCardsToExileFaceDown();
    void actMoveBottomCardToTop();

    void actSelectAll();
    void actSelectRow();
    void actSelectColumn();

    void actViewLibrary();
    void actViewHand();
    void actViewTopCards();
    void actViewBottomCards();
    void actAlwaysRevealTopCard(bool alwaysRevealTopCard);
    void actAlwaysLookAtTopCard(bool alwaysRevealTopCard);
    void actViewGraveyard();
    void actLendLibrary(int lendToPlayerId);
    void actRevealTopCards(int revealToPlayerId, int amount);
    void actRevealRandomGraveyardCard(int revealToPlayerId);
    void actViewRfg();
    void actViewSideboard();

    void actSayMessage();

    void actOpenDeckInDeckEditor();
    void actCreatePredefinedToken();
    void actCreateRelatedCard();
    void actCreateAllRelatedCards();

    void actMoveCardXCardsFromTop(QList<CardItem *> selectedCards);
    void actRemoveCardCounter(QList<CardItem *> selectedCards, int counterId);
    void actAddCardCounter(QList<CardItem *> selectedCards, int counterId);
    void actSetCardCounter(QList<CardItem *> selectedCards, int counterId);
    void actIncrementAllCardCounters(QList<CardItem *> cardsToUpdate);
    void actAttach();
    void actUnattach(QList<CardItem *> selectedCards);
    void actDrawArrow();
    void actIncPT(QList<CardItem *> selectedCards, int deltaP, int deltaT);
    void actResetPT(QList<CardItem *> selectedCards);
    void actSetPT(QList<CardItem *> selectedCards);
    void actIncP(QList<CardItem *> selectedCards);
    void actDecP(QList<CardItem *> selectedCards);
    void actIncT(QList<CardItem *> selectedCards);
    void actDecT(QList<CardItem *> selectedCards);
    void actIncPT(QList<CardItem *> selectedCards);
    void actDecPT(QList<CardItem *> selectedCards);
    void actFlowP(QList<CardItem *> selectedCards);
    void actFlowT(QList<CardItem *> selectedCards);

    void actReduceLifeByPower(QList<CardItem *> selectedCards);

    void actSetAnnotation(QList<CardItem *> selectedCards);
    void actReveal(QList<CardItem *> selectedCards, QAction *action);
    void actRevealHand(int revealToPlayerId);
    void actRevealRandomHandCard(int revealToPlayerId);
    void actRevealLibrary(int revealToPlayerId);

    void actSortHand();

    void cardMenuAction(QList<CardItem *> selectedCards, CardMenuActionType type);

private:
    PlayerLogic *player;

    int defaultNumberTopCards = 1;
    int defaultNumberTopCardsToPlaceBelow = 1;
    int defaultNumberBottomCards = 1;
    int defaultNumberDieRoll = 20;

    TokenInfo lastTokenInfo;
    int lastTokenTableRow;

    bool movingCardsUntil;
    QTimer *moveTopCardTimer;
    FilterString movingCardsUntilFilter;
    int movingCardsUntilCounter = 0;
    MoveTopCardsUntilOptions movingCardsUntilOptions;

    void moveTopCardsTo(const QString &targetZone, const QString &zoneDisplayName, bool faceDown);
    void moveBottomCardsTo(const QString &targetZone, const QString &zoneDisplayName, bool faceDown);

    void createCard(const CardItem *sourceCard,
                    const QString &dbCardName,
                    CardRelationType attach = CardRelationType::DoesNotAttach,
                    bool persistent = false);
    bool createRelatedFromRelation(const CardItem *sourceCard, const CardRelation *cardRelation);

    void playSelectedCards(QList<CardItem *> selectedCards, bool faceDown = false);

    void cmdSetTopCard(Command_MoveCard &cmd);
    void cmdSetBottomCard(Command_MoveCard &cmd);

    void offsetCardCounter(QList<CardItem *> selectedCards, int counterId, int offset);
};

#endif // COCKATRICE_PLAYER_ACTIONS_H
