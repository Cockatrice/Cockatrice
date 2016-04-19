#ifndef MESSAGELOGWIDGET_H
#define MESSAGELOGWIDGET_H

#include "chatview.h"
#include "translation.h"
#include "user_level.h"

class Player;
class CardZone;
class GameEventContext;
class CardItem;

struct LogMoveCard {
    Player *player;
    CardItem *card;
    QString cardName;
    CardZone *startZone;
    int oldX;
    CardZone *targetZone;
    int newX;
};

class MessageLogWidget : public ChatView {
    Q_OBJECT
private:
    enum MessageContext { MessageContext_None, MessageContext_MoveCard, MessageContext_Mulligan };

    QString sanitizeHtml(QString dirty) const;
    QString cardLink(const QString &cardName) const;
    QPair<QString, QString> getFromStr(CardZone *zone, QString cardName, int position, bool ownerChange) const;
    MessageContext currentContext;

    QList<LogMoveCard> moveCardQueue;
    QMap<CardItem *, QString> moveCardPT;
    QMap<CardItem *, bool> moveCardTapped;

    Player *mulliganPlayer;
    int mulliganNumber;
public slots:
    void logGameJoined(int gameId);
    void logReplayStarted(int gameId);
    void logJoin(Player *player);
    void logLeave(Player *player);
    void logGameClosed();
    void logKicked();
    void logJoinSpectator(QString name);
    void logLeaveSpectator(QString name);
    void logDeckSelect(Player *player, QString deckHash, int sideboardSize);
    void logReadyStart(Player *player);
    void logNotReadyStart(Player *player);
    void logSetSideboardLock(Player *player, bool locked);
    void logConcede(Player *player);
    void logGameStart();
    void logConnectionStateChanged(Player *player, bool connectionState);
    void logSay(Player *player, QString message);
    void logSpectatorSay(QString spectatorName, UserLevelFlags spectatorUserLevel, QString message);
    void logShuffle(Player *player, CardZone *zone);
    void logRollDie(Player *player, int sides, int roll);
    void logDrawCards(Player *player, int number);
    void logUndoDraw(Player *player, QString cardName);
    void doMoveCard(LogMoveCard &attributes);
    void logMoveCard(Player *player, CardItem *card, CardZone *startZone, int oldX, CardZone *targetZone, int newX);
    void logMulligan(Player *player, int number);
    void logFlipCard(Player *player, QString cardName, bool faceDown);
    void logDestroyCard(Player *player, QString cardName);
    void logAttachCard(Player *player, QString cardName, Player *targetPlayer, QString targetCardName);
    void logUnattachCard(Player *player, QString cardName);
    void logCreateToken(Player *player, QString cardName, QString pt);
    void logCreateArrow(Player *player, Player *startPlayer, QString startCard, Player *targetPlayer, QString targetCard, bool playerTarget);
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
    void logSetActivePlayer(Player *player);
    void logSetActivePhase(int phase);
    void containerProcessingStarted(const GameEventContext &context);
    void containerProcessingDone();
public:
    void connectToPlayer(Player *player);
    MessageLogWidget(const TabSupervisor *_tabSupervisor, TabGame *_game, QWidget *parent = 0);
};

#endif
