#ifndef MESSAGELOGWIDGET_H
#define MESSAGELOGWIDGET_H

#include <QPlainTextEdit>
#include <QAbstractSocket>
#include "client.h"
#include "translation.h"

class Game;
class Player;
class CardZone;

class MessageLogWidget : public QTextEdit {
	Q_OBJECT
private:
	QString sanitizeHtml(QString dirty) const;
	QString trZoneName(CardZone *zone, Player *player, bool hisOwn, GrammaticalCase gc) const;
public slots:
	void logConnecting(QString hostname);
	void logConnected(const QStringList WelcomeMsg);
	void logDisconnected();
	void logSocketError(const QString &errorString);
	void logServerError(ServerResponse response);
private slots:
	void logPlayerListReceived(QStringList players);
	void logJoin(Player *player);
	void logLeave(Player *player);
	void logReadyStart(Player *player);
	void logGameStart();
	void logSay(Player *player, QString message);
	void logShuffle(Player *player);
	void logRollDie(Player *player, int sides, int roll);
	void logDraw(Player *player, int number);
	void logMoveCard(Player *player, QString cardName, CardZone *startZone, int oldX, CardZone *targetZone, int newX);
	void logCreateToken(Player *player, QString cardName);
	void logSetCardCounters(Player *player, QString cardName, int value, int oldValue);
	void logSetTapped(Player *player, QString cardName, bool tapped);
	void logSetCounter(Player *player, QString counterName, int value, int oldValue);
	void logSetDoesntUntap(Player *player, QString cardName, bool doesntUntap);
	void logDumpZone(Player *player, CardZone *zone, int numberCards);
	void logStopDumpZone(Player *player, CardZone *zone);
	void logSetActivePlayer(Player *player);
	void logSetActivePhase(int phase);
	void msgAlert();
public:
	void connectToGame(Game *game);
	MessageLogWidget(QWidget *parent = 0);
};

#endif
