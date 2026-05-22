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
#include <functional>
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

    void moveOneCardUntil(CardItem *card);
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
    void playCard(CardItem *c, bool faceDown);
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

    void actPlay(QList<CardItem *> selectedCards);
    void actPlayFacedown(QList<CardItem *> selectedCards);
    /** @brief Plays the selected card and increments the primary commander tax counter. */
    void actPlayAndIncreaseTax();
    /** @brief Plays the selected card and increments the partner commander tax counter. */
    void actPlayAndIncreasePartnerTax();
    /** @brief Modifies a tax counter by delta if it is active. */
    void actModifyTaxCounter(int counterId, int delta);
    /** @brief Toggles a tax counter's active state (only if inactive or value is 0). */
    void actToggleTaxCounter(int counterId);
    void actHide(QList<CardItem *> selectedCards);

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
    void actMoveCardXCardsFromTop(QList<CardItem *> selectedCards, int number);
    void actRemoveCardCounter(QList<CardItem *> selectedCards, int counterId);
    void actAddCardCounter(QList<CardItem *> selectedCards, int counterId);
    void actRequestSetCardCounterDialog(QList<CardItem *> selectedCards, int counterId);
    void actSetCardCounter(QList<CardItem *> selectedCards, int counterId, const QString &counterValue);
    void actIncrementAllCardCounters(QList<CardItem *> cardsToUpdate);
    void actAttach();
    void actUnattach(QList<CardItem *> selectedCards);
    void actDrawArrow();
    void actIncPT(QList<CardItem *> selectedCards, int deltaP, int deltaT);
    void actResetPT(QList<CardItem *> selectedCards);
    void actRequestSetPTDialog(QList<CardItem *> selectedCards);
    void actSetPT(QList<CardItem *> selectedCards, const QString &pt);
    void actIncP(QList<CardItem *> selectedCards);
    void actDecP(QList<CardItem *> selectedCards);
    void actIncT(QList<CardItem *> selectedCards);
    void actDecT(QList<CardItem *> selectedCards);
    void actIncPT(QList<CardItem *> selectedCards);
    void actDecPT(QList<CardItem *> selectedCards);
    void actFlowP(QList<CardItem *> selectedCards);
    void actFlowT(QList<CardItem *> selectedCards);

    void actReduceLifeByPower(QList<CardItem *> selectedCards);

    void actRequestSetAnnotationDialog(QList<CardItem *> selectedCards);
    void actSetAnnotation(QList<CardItem *> selectedCards, const QString &annotation);
    void actReveal(QList<CardItem *> selectedCards, QAction *action);
    void actRevealHand(int revealToPlayerId);
    void actRevealRandomHandCard(int revealToPlayerId);
    void actRevealLibrary(int revealToPlayerId);

    void actSortHand();

    void cardMenuAction(QList<CardItem *> selectedCards, CardMenuActionType type);

private:
    void sendIncCounter(int counterId, int delta);

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
                    bool persistent = false,
                    bool faceDown = false);

    void playSelectedCards(QList<CardItem *> selectedCards, bool faceDown = false);

    /**
     * @brief Shared implementation for playing selected cards with an optional post-play callback.
     * @param postPlayCallback Called after each card is played, receiving the card and its *original* zone name
     *        (captured before playCard, since playCard sends a move command that may change the card's zone).
     */
    void playSelectedCardsImpl(bool faceDown,
                               const std::function<void(CardItem *, const QString &)> &postPlayCallback = nullptr);

    void cmdSetTopCard(Command_MoveCard &cmd);
    void cmdSetBottomCard(Command_MoveCard &cmd);

    void offsetCardCounter(QList<CardItem *> selectedCards, int counterId, int offset);
};

#endif // COCKATRICE_PLAYER_ACTIONS_H
