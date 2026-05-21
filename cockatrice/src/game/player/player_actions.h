/**
 * @file player_actions.h
 *  @ingroup GameLogicActions
 * @ingroup GameLogicPlayers
 */
//! \todo Document this file.

#ifndef COCKATRICE_PLAYER_ACTIONS_H
#define COCKATRICE_PLAYER_ACTIONS_H

#include "../../game_graphics/board/card_item.h"
#include "../../game_graphics/dialogs/dlg_create_token.h"
#include "../../game_graphics/dialogs/dlg_move_top_cards_until.h"
#include "../../game_graphics/player/card_menu_action_type.h"
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

    void moveOneCardUntil(CardState *card);
    void stopMoveTopCardsUntil();

    [[nodiscard]] bool isMovingCardsUntil() const
    {
        return movingCardsUntil;
    }

signals:
    void requestViewTopCardsDialog(int defaultNumberTopCards, int deckSize);
    void requestViewBottomCardsDialog(int defaultNumberBottomCards, int deckSize);
    void requestShuffleTopDialog(int defaultNumberTopCards, int maxCards);
    void requestShuffleBottomDialog(int defaultNumberBottomCards, int maxCards);
    void requestMulliganDialog(int startSize, int handSize, int deckSize);
    void requestDrawCardsDialog(int defaultNumberTopCards, int deckSize);
    void requestMoveTopCardsToDialog(int defaultNumberTopCards,
                                     int maxCards,
                                     const QString &targetZone,
                                     const QString &zoneDisplayName,
                                     bool faceDown);
    void requestMoveTopCardsUntilDialog(MoveTopCardsUntilOptions options);
    void requestMoveBottomCardsToDialog(int defaultNumberBottomCards,
                                        int maxCards,
                                        const QString &targetZone,
                                        const QString &zoneDisplayName,
                                        bool faceDown);
    void requestDrawBottomCardsDialog(int defaultNumberBottomCards, int maxCards);
    void requestRollDieDialog();
    void requestCreateTokenDialog(const QStringList &predefinedTokens);
    void requestCreateRelatedFromRelationDialog(const CardItem *sourceCard, const CardRelation *cardRelation);
    void requestMoveCardXCardsFromTopDialog(int defaultNumberTopCardsToPlaceBelow, int deckSize);
    void requestSetPTDialog(const QString &oldPT);
    void requestSetAnnotationDialog(const QString &oldAnnotation);
    void requestSetCardCounterDialog(int counterId, const QString &oldValueForDlg);
    void requestZoneViewToggle(const QString &zoneName, int numberCards, bool isReversed = false);
    void requestSortHand(const QList<CardList::SortOption> &options);
    void requestEnableAndSetCreateAnotherTokenAction(const QString &lastTokenName);
    void requestSetLastToken(CardInfoPtr lastToken);

public slots:
    void setLastToken(CardInfoPtr cardInfo);
    void setLastTokenInfo(CardInfoPtr cardInfo);
    void playCard(CardState *card, bool faceDown);
    void playCardToTable(const CardItem *c, bool faceDown);

    void actUntapAll();
    void actRequestRollDieDialog();
    void actRollDie(int sides, int count);
    void actFlipCoin();
    void actRequestCreateTokenDialog(const QStringList &predefinedTokens);
    void actCreateToken(TokenInfo tokenToCreate);
    void actCreateAnotherToken();
    void actRequestCreateRelatedFromRelationDialog(const CardItem *sourceCard, const CardRelation *cardRelation);
    bool createRelatedFromRelation(const CardItem *sourceCard, const CardRelation *cardRelation, int variableCount);
    void onRelatedCardCreated(const CardItem *sourceCard, const CardRelation *cardRelation);
    void setLastRelatedCreationSucceeded(bool succeeded)
    {
        lastRelatedCreationSucceeded = succeeded;
    }
    void actShuffle();
    void actRequestShuffleTopDialog();
    void actShuffleTop(int number);
    void actRequestShuffleBottomDialog();
    void actShuffleBottom(int number);
    void actDrawCard();
    void actRequestDrawCardsDialog();
    void actDrawCards(int number);
    void actUndoDraw();
    void actRequestMulliganDialog();
    void actMulligan(int number);
    void actMulliganSameSize();
    void actMulliganMinusOne();
    void doMulligan(int number);

    void actPlay(QList<CardState *> selectedCards);
    void actPlayFacedown(QList<CardState *> selectedCards);
    void actHide(QList<CardState *> selectedCards);

    void actMoveTopCardToPlay();
    void actMoveTopCardToPlayFaceDown();
    void actMoveTopCardToGrave();
    void actMoveTopCardToExile();
    void actMoveTopCardsToGrave();
    void actMoveTopCardsToGraveFaceDown();
    void actMoveTopCardsToExile();
    void actMoveTopCardsToExileFaceDown();
    void actRequestMoveTopCardsUntilDialog();
    void moveTopCardsUntil(const QString &expr, MoveTopCardsUntilOptions options);
    void actMoveTopCardToBottom();
    void actRequestMoveTopCardsToDialog(const QString &targetZone, const QString &zoneDisplayName, bool faceDown);
    void moveTopCardsTo(int number, const QString &targetZone, bool faceDown);
    void actDrawBottomCard();
    void actRequestDrawBottomCardsDialog();
    void actDrawBottomCards(int number);
    void actMoveBottomCardToPlay();
    void actMoveBottomCardToPlayFaceDown();
    void actMoveBottomCardToGrave();
    void actMoveBottomCardToExile();
    void actMoveBottomCardsToGrave();
    void actMoveBottomCardsToGraveFaceDown();
    void actMoveBottomCardsToExile();
    void actMoveBottomCardsToExileFaceDown();
    void actMoveBottomCardToTop();
    void actRequestMoveBottomCardsToDialog(const QString &targetZone, const QString &zoneDisplayName, bool faceDown);
    void moveBottomCardsTo(int number, const QString &targetZone, bool faceDown);

    void actSelectAll();
    void actSelectRow();
    void actSelectColumn();

    void actViewLibrary();
    void actViewHand();
    void actRequestViewTopCardsDialog();
    void actViewTopCards(int number);
    void actRequestViewBottomCardsDialog();
    void actViewBottomCards(int number);
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

    void actRequestMoveCardXCardsFromTopDialog();
    void actMoveCardXCardsFromTop(QList<CardState *> selectedCards, int number);
    void actRemoveCardCounter(QList<CardState *> selectedCards, int counterId);
    void actAddCardCounter(QList<CardState *> selectedCards, int counterId);
    void actRequestSetCardCounterDialog(QList<CardState *> selectedCards, int counterId);
    void actSetCardCounter(QList<CardState *> selectedCards, int counterId, const QString &counterValue);
    void actIncrementAllCardCounters(QList<CardState *> cardsToUpdate);
    void actAttach();
    void actUnattach(QList<CardState *> selectedCards);
    void actDrawArrow();
    void actIncPT(QList<CardState *> selectedCards, int deltaP, int deltaT);
    void actResetPT(QList<CardState *> selectedCards);
    void actRequestSetPTDialog(QList<CardState *> selectedCards);
    void actSetPT(QList<CardState *> selectedCards, const QString &pt);
    void actIncP(QList<CardState *> selectedCards);
    void actDecP(QList<CardState *> selectedCards);
    void actIncT(QList<CardState *> selectedCards);
    void actDecT(QList<CardState *> selectedCards);
    void actIncPT(QList<CardState *> selectedCards);
    void actDecPT(QList<CardState *> selectedCards);
    void actFlowP(QList<CardState *> selectedCards);
    void actFlowT(QList<CardState *> selectedCards);

    void actReduceLifeByPower(QList<CardState *> selectedCards);

    void actRequestSetAnnotationDialog(QList<CardState *> selectedCards);
    void actSetAnnotation(QList<CardState *> selectedCards, const QString &annotation);
    void actReveal(QList<CardState *> selectedCards, QAction *action);
    void actRevealHand(int revealToPlayerId);
    void actRevealRandomHandCard(int revealToPlayerId);
    void actRevealLibrary(int revealToPlayerId);

    void actSortHand();

    void cardMenuAction(QList<CardState *> selectedCards, CardMenuActionType type);

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

    bool lastRelatedCreationSucceeded = false;

    void createCard(const CardItem *sourceCard,
                    const QString &dbCardName,
                    CardRelationType attach = CardRelationType::DoesNotAttach,
                    bool persistent = false);

    void playSelectedCards(QList<CardState *> selectedCards, bool faceDown = false);

    void cmdSetTopCard(Command_MoveCard &cmd);
    void cmdSetBottomCard(Command_MoveCard &cmd);

    void offsetCardCounter(QList<CardState *> selectedCards, int counterId, int offset);
};

#endif // COCKATRICE_PLAYER_ACTIONS_H
