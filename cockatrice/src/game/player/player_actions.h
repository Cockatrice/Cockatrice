/**
 * @file player_actions.h
 *  @ingroup GameLogicActions
 * @ingroup GameLogicPlayers
 * @brief TODO: Document this.
 */

#ifndef COCKATRICE_PLAYER_ACTIONS_H
#define COCKATRICE_PLAYER_ACTIONS_H
#include "event_processing_options.h"
#include "player.h"

#include <QMenu>
#include <QObject>
#include <libcockatrice/card/relation/card_relation_type.h>
#include <libcockatrice/filters/filter_string.h>
#include <libcockatrice/protocol/pb/card_attributes.pb.h>

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
class Player;
class PlayerActions : public QObject
{
    Q_OBJECT

signals:
    void logSetTapped(Player *player, CardItem *card, bool tapped);
    void logSetAnnotation(Player *player, CardItem *card, QString newAnnotation);
    void logSetDoesntUntap(Player *player, CardItem *card, bool doesntUntap);
    void logSetPT(Player *player, CardItem *card, QString newPT);

public:
    enum CardsToReveal
    {
        RANDOM_CARD_FROM_ZONE = -2
    };

    explicit PlayerActions(Player *player);

    void sendGameCommand(PendingCommand *pend);
    void sendGameCommand(const google::protobuf::Message &command);

    PendingCommand *prepareGameCommand(const ::google::protobuf::Message &cmd);
    PendingCommand *prepareGameCommand(const QList<const ::google::protobuf::Message *> &cmdList);

    void setCardAttrHelper(const GameEventContext &context,
                           CardItem *card,
                           CardAttribute attribute,
                           const QString &avalue,
                           bool allCards,
                           EventProcessingOptions options);

    void moveOneCardUntil(CardItem *card);
    void stopMoveTopCardsUntil();

    [[nodiscard]] bool isMovingCardsUntil() const
    {
        return movingCardsUntil;
    }

public slots:
    void setLastToken(CardInfoPtr cardInfo);
    void playCard(CardItem *c, bool faceDown);
    void playCardToTable(const CardItem *c, bool faceDown);

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
    void actRevealHand(int revealToPlayerId);
    void actRevealRandomHandCard(int revealToPlayerId);
    void actRevealLibrary(int revealToPlayerId);

    void actSortHand();

    void cardMenuAction();

private:
    Player *player;

    int defaultNumberTopCards = 1;
    int defaultNumberTopCardsToPlaceBelow = 1;
    int defaultNumberBottomCards = 1;
    int defaultNumberDieRoll = 20;

    TokenInfo lastTokenInfo;
    int lastTokenTableRow;

    bool movingCardsUntil;
    QTimer *moveTopCardTimer;
    QStringList movingCardsUntilExprs = {};
    int movingCardsUntilNumberOfHits = 1;
    bool movingCardsUntilAutoPlay = false;
    FilterString movingCardsUntilFilter;
    int movingCardsUntilCounter = 0;

    void createCard(const CardItem *sourceCard,
                    const QString &dbCardName,
                    CardRelationType attach = CardRelationType::DoesNotAttach,
                    bool persistent = false);
    bool createRelatedFromRelation(const CardItem *sourceCard, const CardRelation *cardRelation);

    void playSelectedCards(bool faceDown = false);

    void cmdSetTopCard(Command_MoveCard &cmd);
    void cmdSetBottomCard(Command_MoveCard &cmd);

    QVariantList parsePT(const QString &pt);
};

#endif // COCKATRICE_PLAYER_ACTIONS_H
