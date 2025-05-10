#ifndef MESSAGELOGWIDGET_H
#define MESSAGELOGWIDGET_H

#include "../client/translation.h"
#include "chat_view/chat_view.h"
#include "user_level.h"

class Player;
class CardZone;
class GameEventContext;
class CardItem;

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

    static QPair<QString, QString> getFromStr(CardZone *zone, QString cardName, int position, bool ownerChange);

public slots:
    void containerProcessingDone();
    void containerProcessingStarted(const GameEventContext &context);
    void logAlwaysRevealTopCard(Player *player, CardZone *zone, bool reveal);
    void logAlwaysLookAtTopCard(Player *player, CardZone *zone, bool reveal);
    void logAttachCard(Player *player, QString cardName, Player *targetPlayer, QString targetCardName);
    void logConcede(Player *player);
    void logUnconcede(Player *player);
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
    void logDumpZone(Player *player, CardZone *zone, int numberCards, bool isReversed = false);
    void logFlipCard(Player *player, QString cardName, bool faceDown);
    void logGameClosed();
    void logGameStart();
    void logJoin(Player *player);
    void logJoinSpectator(QString name);
    void logKicked();
    void logLeave(Player *player, QString reason);
    void logLeaveSpectator(QString name, QString reason);
    void logNotReadyStart(Player *player);
    void logMoveCard(Player *player, CardItem *card, CardZone *startZone, int oldX, CardZone *targetZone, int newX);
    void logMulligan(Player *player, int number);
    void logReplayStarted(int gameId);
    void logReadyStart(Player *player);
    void logRevealCards(Player *player,
                        CardZone *zone,
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
    void logShuffle(Player *player, CardZone *zone, int start, int end);
    void logSpectatorSay(const ServerInfo_User &spectator, QString message);
    void logUnattachCard(Player *player, QString cardName);
    void logUndoDraw(Player *player, QString cardName);
    void setContextJudgeName(QString player);
    void appendHtmlServerMessage(const QString &html,
                                 bool optionalIsBold = false,
                                 QString optionalFontColor = QString()) override;

public:
    void connectToPlayer(Player *player);
    MessageLogWidget(TabSupervisor *_tabSupervisor, TabGame *_game, QWidget *parent = nullptr);
};

#endif
