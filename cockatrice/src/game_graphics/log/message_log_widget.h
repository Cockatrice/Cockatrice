/**
 * @file message_log_widget.h
 * @ingroup GameWidgets
 */
//! \todo Document this file.

#ifndef MESSAGELOGWIDGET_H
#define MESSAGELOGWIDGET_H

#include "../../game/zones/card_zone_logic.h"
#include "../../interface/widgets/server/chat_view/chat_view.h"

class AbstractGame;
class CardItem;
class GameEventContext;
class PlayerLogic;
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
    void logAlwaysRevealTopCard(PlayerLogic *player, CardZoneLogic *zone, bool reveal);
    void logAlwaysLookAtTopCard(PlayerLogic *player, CardZoneLogic *zone, bool reveal);
    void logAttachCard(PlayerLogic *player, QString cardName, PlayerLogic *targetPlayer, QString targetCardName);
    void logConcede(int playerId);
    void logUnconcede(int playerId);
    void logConnectionStateChanged(PlayerLogic *player, bool connectionState);
    void logCreateArrow(PlayerLogic *player,
                        PlayerLogic *startPlayer,
                        QString startCard,
                        PlayerLogic *targetPlayer,
                        QString targetCard,
                        bool playerTarget);
    void logCreateToken(PlayerLogic *player, QString cardName, QString pt, bool faceDown);
    void logDeckSelect(PlayerLogic *player, QString deckHash, int sideboardSize);
    void logDestroyCard(PlayerLogic *player, QString cardName);
    void logDrawCards(PlayerLogic *player, int number, bool deckIsEmpty);
    void logDumpZone(PlayerLogic *player, CardZoneLogic *zone, int numberCards, bool isReversed = false);
    void logFlipCard(PlayerLogic *player, QString cardName, bool faceDown);
    void logGameClosed();
    void logGameStart();
    void logGameFlooded();
    void logJoin(PlayerLogic *player);
    void logJoinSpectator(QString name);
    void logKicked();
    void logLeave(PlayerLogic *player, QString reason);
    void logLeaveSpectator(QString name, QString reason);
    void logNotReadyStart(PlayerLogic *player);
    void logMoveCard(PlayerLogic *player,
                     CardItem *card,
                     CardZoneLogic *startZone,
                     int oldX,
                     CardZoneLogic *targetZone,
                     int newX);
    void logMulligan(PlayerLogic *player, int number);
    void logReplayStarted(int gameId);
    void logReadyStart(PlayerLogic *player);
    void logRevealCards(PlayerLogic *player,
                        CardZoneLogic *zone,
                        int cardId,
                        QString cardName,
                        PlayerLogic *otherPlayer,
                        bool faceDown,
                        int amount,
                        bool isLentToAnotherPlayer);
    void logReverseTurn(PlayerLogic *player, bool reversed);
    void logRollDie(PlayerLogic *player, int sides, const QList<uint> &rolls);
    void logSay(PlayerLogic *player, QString message);
    void logSetActivePhase(int phase);
    void logSetActivePlayer(PlayerLogic *player);
    void logSetAnnotation(PlayerLogic *player, CardItem *card, QString newAnnotation);
    void logSetCardCounter(PlayerLogic *player, QString cardName, int counterId, int value, int oldValue);
    void logSetCounter(PlayerLogic *player, QString counterName, int value, int oldValue);
    void logSetDoesntUntap(PlayerLogic *player, CardItem *card, bool doesntUntap);
    void logSetPT(PlayerLogic *player, CardItem *card, QString newPT);
    void logSetSideboardLock(PlayerLogic *player, bool locked);
    void logSetTapped(PlayerLogic *player, CardItem *card, bool tapped);
    void logShuffle(PlayerLogic *player, CardZoneLogic *zone, int start, int end);
    void logSpectatorSay(const ServerInfo_User &spectator, QString message);
    void logUnattachCard(PlayerLogic *player, QString cardName);
    void logUndoDraw(PlayerLogic *player, QString cardName);
    void logUndoDrawFailed(PlayerLogic *player);
    void setContextJudgeName(QString player);
    void appendHtmlServerMessage(const QString &html,
                                 bool optionalIsBold = false,
                                 QString optionalFontColor = QString()) override;
};

#endif
