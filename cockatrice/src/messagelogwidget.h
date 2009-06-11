#ifndef MESSAGELOGWIDGET_H
#define MESSAGELOGWIDGET_H

#include <QTextEdit>
#include <QAbstractSocket>

class Game;

class MessageLogWidget : public QTextEdit {
	Q_OBJECT
public slots:
	void logConnecting(QString hostname);
	void logConnected(const QStringList WelcomeMsg);
	void logDisconnected();
	void logSocketError(const QString &errorString);
private slots:
	void logPlayerListReceived(QStringList players);
	void logJoin(QString playerName);
	void logLeave(QString playerName);
	void logReadyStart(QString playerName);
	void logGameStart();
	void logSay(QString playerName, QString message);
	void logShuffle(QString playerName);
	void logRollDice(QString playerName, int sides, int roll);
	void logDraw(QString playerName, int number);
	void logMoveCard(QString playerName, QString cardName, QString startZone, QString targetZone);
	void logCreateToken(QString playerName, QString cardName);
	void logSetCardCounters(QString playerName, QString cardName, int value, int oldValue);
	void logSetTapped(QString playerName, QString cardName, bool tapped);
	void logSetCounter(QString playerName, QString counterName, int value, int oldValue);
	void logSetDoesntUntap(QString playerName, QString cardName, bool doesntUntap);
	void logDumpZone(QString playerName, QString zoneName, QString zoneOwner, int numberCards);
public:
	void connectToGame(Game *game);
	MessageLogWidget(QWidget *parent = 0);
};

#endif
