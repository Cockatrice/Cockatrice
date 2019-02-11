#ifndef MESSAGELOGWIDGET_H
#define MESSAGELOGWIDGET_H

#include "chatview/chatview.h"
#include "translation.h"
#include "user_level.h"

class Player;
class CardZone;
class GameEventContext;
class CardItem;

struct LogMoveCard
{
    Player *player;
    CardItem *card;
    QString cardName;
    CardZone *startZone;
    int oldX;
    CardZone *targetZone;
    int newX;
};

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

    int mulliganNumber;
    Player *mulliganPlayer;
    MessageContext currentContext;
    QList<LogMoveCard> moveCardQueue;
    QMap<CardItem *, bool> moveCardTapped;
    QList<QString> moveCardExtras;

    const QString tableConstant() const;
    const QString graveyardConstant() const;
    const QString exileConstant() const;
    const QString handConstant() const;
    const QString deckConstant() const;
    const QString sideboardConstant() const;
    const QString stackConstant() const;

    QString sanitizeHtml(QString dirty) const;
    QString cardLink(QString cardName) const;
    QPair<QString, QString> getFromStr(CardZone *zone, QString cardName, int position, bool ownerChange) const;

public slots:
    void containerProcessingDone();
    void containerProcessingStarted(const GameEventContext &context);
    void logAlwaysRevealTopCard(Player *player, CardZone *zone, bool reveal);
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
    void logCreateToken(Player *player, QString cardName, QString pt);
    void logDeckSelect(Player *player, QString deckHash, int sideboardSize);
    void logDestroyCard(Player *player, QString cardName);
    void logDoMoveCard(LogMoveCard &lmc);
    void logDrawCards(Player *player, int number);
    void logDumpZone(Player *player, CardZone *zone, int numberCards);
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
                        int amount);
    void logRollDie(Player *player, int sides, int roll);
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
    void
    logSpectatorSay(QString spectatorName, UserLevelFlags spectatorUserLevel, QString userPrivLevel, QString message);
    void logStopDumpZone(Player *player, CardZone *zone);
    void logUnattachCard(Player *player, QString cardName);
    void logUndoDraw(Player *player, QString cardName);

public:
    void connectToPlayer(Player *player);
    MessageLogWidget(const TabSupervisor *_tabSupervisor,
                     const UserlistProxy *_userlistProxy,
                     TabGame *_game,
                     QWidget *parent = nullptr);
};

#endif
