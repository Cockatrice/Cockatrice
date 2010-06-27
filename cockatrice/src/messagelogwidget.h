#ifndef MESSAGELOGWIDGET_H
#define MESSAGELOGWIDGET_H

#include <QPlainTextEdit>
#include <QAbstractSocket>
#include "translation.h"
#include "protocol_datastructures.h"

class Player;
class CardZone;

class MessageLogWidget : public QTextEdit {
	Q_OBJECT
private:
	QString sanitizeHtml(QString dirty) const;
	QString trZoneName(CardZone *zone, Player *player, bool hisOwn, GrammaticalCase gc) const;
public slots:
	void logConnecting(QString hostname);
	void logConnected();
	void logDisconnected();
	void logSocketError(const QString &errorString);
	void logServerError(ResponseCode response);
	void logProtocolVersionMismatch(int clientVersion, int serverVersion);
	void logProtocolError();
	void logGameJoined(int gameId);
	void logJoin(Player *player);
	void logLeave(Player *player);
	void logGameClosed();
	void logJoinSpectator(QString name);
	void logLeaveSpectator(QString name);
	void logDeckSelect(Player *player, int deckId);
	void logReadyStart(Player *player);
	void logNotReadyStart(Player *player);
	void logConcede(Player *player);
	void logGameStart();
	void logSay(Player *player, QString message);
	void logSpectatorSay(QString spectatorName, QString message);
	void logShuffle(Player *player);
	void logRollDie(Player *player, int sides, int roll);
	void logDrawCards(Player *player, int number);
	void logMoveCard(Player *player, QString cardName, CardZone *startZone, int oldX, CardZone *targetZone, int newX);
	void logDestroyCard(Player *player, QString cardName);
	void logCreateToken(Player *player, QString cardName, QString pt);
	void logCreateArrow(Player *player, Player *startPlayer, QString startCard, Player *targetPlayer, QString targetCard, bool playerTarget);
	void logSetCardCounter(Player *player, QString cardName, int counterId, int value, int oldValue);
	void logSetTapped(Player *player, QString cardName, bool tapped);
	void logSetCounter(Player *player, QString counterName, int value, int oldValue);
	void logSetDoesntUntap(Player *player, QString cardName, bool doesntUntap);
	void logSetPT(Player *player, QString cardName, QString newPT);
	void logSetAnnotation(Player *player, QString cardName, QString newAnnotation);
	void logDumpZone(Player *player, CardZone *zone, int numberCards);
	void logStopDumpZone(Player *player, CardZone *zone);
	void logSetActivePlayer(Player *player);
	void logSetActivePhase(int phase);
public:
	void connectToPlayer(Player *player);
	MessageLogWidget(QWidget *parent = 0);
};

#endif
