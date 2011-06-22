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

class MessageLogWidget : public QTextEdit {
	Q_OBJECT
private:
	enum MessageContext { MessageContext_None, MessageContext_MoveCard, MessageContext_Mulligan };
	
	CardInfoWidget *infoWidget;
	QString sanitizeHtml(QString dirty) const;
	bool isFemale(Player *player) const;
	QPair<QString, QString> getFromStr(CardZone *zone, QString cardName, int position) const;
	QString getCardNameUnderMouse(const QPoint &pos) const;
	MessageContext currentContext;
	
	QList<LogMoveCard> moveCardQueue;
	QMap<CardItem *, QString> moveCardPT;
	QMap<CardItem *, bool> moveCardTapped;
	
	Player *mulliganPlayer;
	int mulliganNumber;
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
	void logRevealCards(Player *player, CardZone *zone, int cardId, QString cardName, Player *otherPlayer);
	void logSetActivePlayer(Player *player);
	void logSetActivePhase(int phase);
	void containerProcessingStarted(GameEventContext *context);
	void containerProcessingDone();
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
