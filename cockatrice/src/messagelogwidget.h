#ifndef MESSAGELOGWIDGET_H
#define MESSAGELOGWIDGET_H

#include <QTextEdit>
#include <QAbstractSocket>
#include "translation.h"
#include "protocol_datastructures.h"

class Player;
class CardZone;
class QMouseEvent;
class QEvent;
class CardInfoWidget;

class MessageLogWidget : public QTextEdit {
	Q_OBJECT
private:
	CardInfoWidget *infoWidget;
	QString sanitizeHtml(QString dirty) const;
	QPair<QString, QString> getFromStr(CardZone *zone, QString cardName, int position) const;
	QString getCardNameUnderMouse(const QPoint &pos) const;
signals:
	void cardNameHovered(QString cardName);
	void showCardInfoPopup(QPoint pos, QString cardName);
	void deleteCardInfoPopup();
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
	void logFlipCard(Player *player, QString cardName, bool faceDown);
	void logDestroyCard(Player *player, QString cardName);
	void logAttachCard(Player *player, QString cardName, Player *targetPlayer, QString targetCardName);
	void logUnattachCard(Player *player, QString cardName);
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
	void logRevealCards(Player *player, CardZone *zone, int cardId, QString cardName, Player *otherPlayer);
	void logSetActivePlayer(Player *player);
	void logSetActivePhase(int phase);
public:
	void connectToPlayer(Player *player);
	MessageLogWidget(QWidget *parent = 0);
protected:
	void enterEvent(QEvent *event);
	void leaveEvent(QEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
};

#endif
