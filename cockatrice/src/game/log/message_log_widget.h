/**
 * @file message_log_widget.h
 * @ingroup GameWidgets
 * @brief TODO: Document this.
 */

#ifndef MESSAGELOGWIDGET_H
#define MESSAGELOGWIDGET_H

#include "../../interface/widgets/server/chat_view/chat_view.h"
#include "../zones/logic/card_zone_logic.h"

class AbstractGame;
class CardItem;
class GameEventContext;
class Player;
class PlayerEventHandler;

class MessageLogWidget : public ChatView
{
    Q_OBJECT
private:
    enum MessageContext
    {
        MessageContext_None,
        MessageContext_MoveCard,
        MessageContext_Mulligan
    };

    MessageContext currentContext;
    QString messagePrefix, messageSuffix;

    static QPair<QString, QString> getFromStr(CardZoneLogic *zone, QString cardName, int position, bool ownerChange);

public:
    void connectToPlayerEventHandler(PlayerEventHandler *player);
    MessageLogWidget(TabSupervisor *_tabSupervisor, AbstractGame *_game, QWidget *parent = nullptr);

public slots:
    void containerProcessingDone();
    void containerProcessingStarted(const GameEventContext &context);
    void logAlwaysRevealTopCard(Player *player, CardZoneLogic *zone, bool reveal);
    void logAlwaysLookAtTopCard(Player *player, CardZoneLogic *zone, bool reveal);
    void logAttachCard(Player *player, QString cardName, Player *targetPlayer, QString targetCardName);
    void logConcede(int playerId);
    void logUnconcede(int playerId);
    void logConnectionStateChanged(Player *player, bool connectionState);
    void logCreateArrow(Player *player,
                        Player *startPlayer,
                        QString startCard,
                        Player *targetPlayer,
                        QString targetCard,
                        bool playerTarget);
    void logCreateToken(Player *player, QString cardName, QString pt, bool faceDown);
    void logDeckSelect(Player *player, QString deckHash, int sideboardSize);
    void logDestroyCard(Player *player, QString cardName);
    void logDrawCards(Player *player, int number, bool deckIsEmpty);
    void logDumpZone(Player *player, CardZoneLogic *zone, int numberCards, bool isReversed = false);
    void logFlipCard(Player *player, QString cardName, bool faceDown);
    void logGameClosed();
    void logGameStart();
    void logGameFlooded();
    void logJoin(Player *player);
    void logJoinSpectator(QString name);
    void logKicked();
    void logLeave(Player *player, QString reason);
    void logLeaveSpectator(QString name, QString reason);
    void logNotReadyStart(Player *player);
    void logMoveCard(Player *player,
                     CardItem *card,
                     CardZoneLogic *startZone,
                     int oldX,
                     CardZoneLogic *targetZone,
                     int newX);
    void logMulligan(Player *player, int number);
    void logReplayStarted(int gameId);
    void logReadyStart(Player *player);
    void logRevealCards(Player *player,
                        CardZoneLogic *zone,
                        int cardId,
                        QString cardName,
                        Player *otherPlayer,
                        bool faceDown,
                        int amount,
                        bool isLentToAnotherPlayer);
    void logReverseTurn(Player *player, bool reversed);
    void logRollDie(Player *player, int sides, const QList<uint> &rolls);
    void logSay(Player *player, QString message);
    void logSetActivePhase(int phase);
    void logSetActivePlayer(Player *player);
    void logSetAnnotation(Player *player, CardItem *card, QString newAnnotation);
    void logSetCardCounter(Player *player, QString cardName, int counterId, int value, int oldValue);
    void logSetCounter(Player *player, QString counterName, int value, int oldValue);
    void logSetDoesntUntap(Player *player, CardItem *card, bool doesntUntap);
    void logSetPT(Player *player, CardItem *card, QString newPT);
    void logSetSideboardLock(Player *player, bool locked);
    void logSetTapped(Player *player, CardItem *card, bool tapped);
    void logShuffle(Player *player, CardZoneLogic *zone, int start, int end);
    void logSpectatorSay(const ServerInfo_User &spectator, QString message);
    void logUnattachCard(Player *player, QString cardName);
    void logUndoDraw(Player *player, QString cardName);
    void setContextJudgeName(QString player);
    void appendHtmlServerMessage(const QString &html,
                                 bool optionalIsBold = false,
                                 QString optionalFontColor = QString()) override;
};

#endif
