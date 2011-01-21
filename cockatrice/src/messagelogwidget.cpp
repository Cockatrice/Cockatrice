#include "messagelogwidget.h"
#include "player.h"
#include "cardzone.h"
#include "cardinfowidget.h"
#include <QDebug>
#include <QMouseEvent>
#include <QTextBlock>

QString MessageLogWidget::sanitizeHtml(QString dirty) const
{
	return dirty
		.replace("&", "&amp;")
		.replace("<", "&lt;")
		.replace(">", "&gt;");
}

void MessageLogWidget::logConnecting(QString hostname)
{
	append(tr("Connecting to %1...").arg(sanitizeHtml(hostname)));
}

void MessageLogWidget::logConnected()
{
	append(tr("Connected."));
}

void MessageLogWidget::logDisconnected()
{
	append(tr("Disconnected from server."));
}

void MessageLogWidget::logSocketError(const QString &errorString)
{
	append(sanitizeHtml(errorString));
}

void MessageLogWidget::logServerError(ResponseCode response)
{
	switch (response) {
		case RespWrongPassword: append(tr("Invalid password.")); break;
		default: ;
	}
}

void MessageLogWidget::logProtocolVersionMismatch(int clientVersion, int serverVersion)
{
	append(tr("Protocol version mismatch. Client: %1, Server: %2").arg(clientVersion).arg(serverVersion));
}

void MessageLogWidget::logProtocolError()
{
	append(tr("Protocol error."));
}

void MessageLogWidget::logGameJoined(int gameId)
{
	append(tr("You have joined game #%1.").arg(gameId));
}

void MessageLogWidget::logJoin(Player *player)
{
	append(tr("%1 has joined the game.").arg(sanitizeHtml(player->getName())));
}

void MessageLogWidget::logLeave(Player *player)
{
	append(tr("%1 has left the game.").arg(sanitizeHtml(player->getName())));
}

void MessageLogWidget::logGameClosed()
{
	append(tr("The game has been closed."));
}

void MessageLogWidget::logJoinSpectator(QString name)
{
	append(tr("%1 is now watching the game.").arg(sanitizeHtml(name)));
}

void MessageLogWidget::logLeaveSpectator(QString name)
{
	append(tr("%1 is not watching the game any more.").arg(sanitizeHtml(name)));
}

void MessageLogWidget::logDeckSelect(Player *player, int deckId)
{
	if (deckId == -1)
		append(tr("%1 has loaded a local deck.").arg(sanitizeHtml(player->getName())));
	else
		append(tr("%1 has loaded deck #%2.").arg(sanitizeHtml(player->getName())).arg(deckId));
}

void MessageLogWidget::logReadyStart(Player *player)
{
	append(tr("%1 is ready to start the game.").arg(sanitizeHtml(player->getName())));
}

void MessageLogWidget::logNotReadyStart(Player *player)
{
	append(tr("%1 is not ready to start the game any more.").arg(sanitizeHtml(player->getName())));
}

void MessageLogWidget::logConcede(Player *player)
{
	append(tr("%1 has conceded the game.").arg(sanitizeHtml(player->getName())));
}

void MessageLogWidget::logGameStart()
{
	append(tr("The game has started."));
}

void MessageLogWidget::logSay(Player *player, QString message)
{
	append(QString("<b><font color=\"") + (player->getLocal() ? "red" : "#0000fe") + QString("\">%1:</font></b> %2").arg(sanitizeHtml(player->getName())).arg(sanitizeHtml(message)));
}

void MessageLogWidget::logSpectatorSay(QString spectatorName, QString message)
{
	append(QString("<font color=\"red\">%1:</font> %2").arg(sanitizeHtml(spectatorName)).arg(sanitizeHtml(message)));
}

void MessageLogWidget::logShuffle(Player *player)
{
	append(tr("%1 shuffles his library.").arg(sanitizeHtml(player->getName())));
}

void MessageLogWidget::logRollDie(Player *player, int sides, int roll)
{
	append(tr("%1 rolls a %2 with a %3-sided die.").arg(sanitizeHtml(player->getName())).arg(roll).arg(sides));
}

void MessageLogWidget::logDrawCards(Player *player, int number)
{
	if (number == 1)
		append(tr("%1 draws a card.").arg(sanitizeHtml(player->getName())));
	else
		append(tr("%1 draws %2 cards.").arg(sanitizeHtml(player->getName())).arg(number));
}

QPair<QString, QString> MessageLogWidget::getFromStr(CardZone *zone, QString cardName, int position) const
{
	bool cardNameContainsStartZone = false;
	QString fromStr;
	QString startName = zone->getName();
	
	if (startName == "table")
		fromStr = tr(" from table");
	else if (startName == "grave")
		fromStr = tr(" from graveyard");
	else if (startName == "rfg")
		fromStr = tr(" from exile");
	else if (startName == "hand")
		fromStr = tr(" from hand");
	else if (startName == "deck") {
		if (position == zone->getCards().size() - 1) {
			if (cardName.isEmpty()) {
				cardName = tr("the bottom card of his library");
				cardNameContainsStartZone = true;
			} else
				fromStr = tr(" from the bottom of his library");
		} else if (position == 0) {
			if (cardName.isEmpty()) {
				cardName = tr("the top card of his library");
				cardNameContainsStartZone = true;
			} else
				fromStr = tr(" from the top of his library");
		} else
			fromStr = tr(" from library");
	} else if (startName == "sb")
		fromStr = tr(" from sideboard");
	else if (startName == "stack")
		fromStr = tr(" from the stack");

	if (!cardNameContainsStartZone)
		cardName.clear();
	return QPair<QString, QString>(cardName, fromStr);
}

void MessageLogWidget::logMoveCard(Player *player, QString cardName, CardZone *startZone, int oldX, CardZone *targetZone, int newX)
{
	QString startName = startZone->getName();
	QString targetName = targetZone->getName();
	if (((startName == "table") && (targetName == "table") && (startZone == targetZone)) || ((startName == "hand") && (targetName == "hand")))
		return;
	QPair<QString, QString> temp = getFromStr(startZone, cardName, oldX);
	bool cardNameContainsStartZone = false;
	if (!temp.first.isEmpty()) {
		cardNameContainsStartZone = true;
		cardName = temp.first;
	}
	QString fromStr = temp.second;
	QString cardStr;
	if (cardNameContainsStartZone)
		cardStr = cardName;
	else if (cardName.isEmpty())
		cardStr = tr("a card");
	else
		cardStr = QString("<font color=\"blue\">%1</font>").arg(sanitizeHtml(cardName));

	if (startZone->getPlayer() != targetZone->getPlayer()) {
		append(tr("%1 gives %2 control over %3.").arg(sanitizeHtml(player->getName())).arg(sanitizeHtml(targetZone->getPlayer()->getName())).arg(cardStr));
		return;
	}
	
	QString finalStr;
	if (targetName == "table")
		finalStr = tr("%1 puts %2 into play%3.");
	else if (targetName == "grave")
		finalStr = tr("%1 puts %2%3 into graveyard.");
	else if (targetName == "rfg")
		finalStr = tr("%1 exiles %2%3.");
	else if (targetName == "hand")
		finalStr = tr("%1 moves %2%3 to hand.");
	else if (targetName == "deck") {
		if (newX == -1)
			finalStr = tr("%1 puts %2%3 into his library.");
		else if (newX == targetZone->getCards().size())
			finalStr = tr("%1 puts %2%3 on bottom of his library.");
		else if (newX == 0)
			finalStr = tr("%1 puts %2%3 on top of his library.");
		else
			finalStr = tr("%1 puts %2%3 into his library at position %4.");
	} else if (targetName == "sb")
		finalStr = tr("%1 moves %2%3 to sideboard.");
	else if (targetName == "stack")
		finalStr = tr("%1 plays %2%3.");
	
	append(finalStr.arg(sanitizeHtml(player->getName())).arg(cardStr).arg(fromStr).arg(newX));
}

void MessageLogWidget::logFlipCard(Player *player, QString cardName, bool faceDown)
{
	if (faceDown)
		append(tr("%1 flips %2 face-down.").arg(sanitizeHtml(player->getName())).arg(cardName));
	else
		append(tr("%1 flips %2 face-up.").arg(sanitizeHtml(player->getName())).arg(cardName));
}

void MessageLogWidget::logDestroyCard(Player *player, QString cardName)
{
	append(tr("%1 destroys %2.").arg(sanitizeHtml(player->getName())).arg(QString("<font color=\"blue\">%1</font>").arg(sanitizeHtml(cardName))));
}

void MessageLogWidget::logAttachCard(Player *player, QString cardName, Player *targetPlayer, QString targetCardName)
{
	append(tr("%1 attaches %2 to %3's %4.").arg(sanitizeHtml(player->getName())).arg(QString("<font color=\"blue\">%1</font>").arg(sanitizeHtml(cardName))).arg(sanitizeHtml(targetPlayer->getName())).arg(QString("<font color=\"blue\">%1</font>").arg(sanitizeHtml(targetCardName))));
}

void MessageLogWidget::logUnattachCard(Player *player, QString cardName)
{
	append(tr("%1 unattaches %2.").arg(sanitizeHtml(player->getName())).arg(QString("<font color=\"blue\">%1</font>").arg(sanitizeHtml(cardName))));
}

void MessageLogWidget::logCreateToken(Player *player, QString cardName, QString pt)
{
	append(tr("%1 creates token: %2%3.").arg(sanitizeHtml(player->getName())).arg(QString("<font color=\"blue\"><a name=\"foo\">%1</a></font>").arg(sanitizeHtml(cardName))).arg(pt.isEmpty() ? QString() : QString(" (%1)").arg(sanitizeHtml(pt))));
}

void MessageLogWidget::logCreateArrow(Player *player, Player *startPlayer, QString startCard, Player *targetPlayer, QString targetCard, bool playerTarget)
{
	if (playerTarget)
		append(tr("%1 points from %2's %3 to %4.")
			.arg(sanitizeHtml(player->getName()))
			.arg(sanitizeHtml(startPlayer->getName()))
			.arg(sanitizeHtml(startCard))
			.arg(sanitizeHtml(targetPlayer->getName()))
		);
	else
		append(tr("%1 points from %2's %3 to %4's %5.")
			.arg(sanitizeHtml(player->getName()))
			.arg(sanitizeHtml(startPlayer->getName()))
			.arg(sanitizeHtml(startCard))
			.arg(sanitizeHtml(targetPlayer->getName()))
			.arg(sanitizeHtml(targetCard))
		);
}

void MessageLogWidget::logSetCardCounter(Player *player, QString cardName, int counterId, int value, int oldValue)
{
	QString finalStr, colorStr;
	
	int delta = abs(oldValue - value);
	if (value > oldValue)
		finalStr = tr("%1 places %n counter(s) (%2) on %3 (now %4).", "", delta);
	else
		finalStr = tr("%1 removes %n counter(s) (%2) from %3 (now %4).", "", delta);
	
	switch (counterId) {
		case 0: colorStr = tr("red"); break;
		case 1: colorStr = tr("yellow"); break;
		case 2: colorStr = tr("green"); break;
		default: ;
	}
	
	append(finalStr.arg(sanitizeHtml(player->getName())).arg(colorStr).arg(QString("<font color=\"blue\">%1</font>").arg(sanitizeHtml(cardName))).arg(value));
}

void MessageLogWidget::logSetTapped(Player *player, QString cardName, bool tapped)
{
	QString cardStr;
	if (cardName == "-1")
		cardStr = tr("his permanents");
	else
		cardStr = QString("<font color=\"blue\">%1</font>").arg(sanitizeHtml(cardName));
	append(tr("%1 %2 %3.").arg(sanitizeHtml(player->getName())).arg(tapped ? tr("taps") : tr("untaps")).arg(cardStr));
}

void MessageLogWidget::logSetCounter(Player *player, QString counterName, int value, int oldValue)
{
	append(tr("%1 sets counter %2 to %3 (%4%5).").arg(sanitizeHtml(player->getName())).arg(QString("<font color=\"blue\">%1</font>").arg(sanitizeHtml(counterName))).arg(QString("<font color=\"blue\">%1</font>").arg(value)).arg(value > oldValue ? "+" : "").arg(value - oldValue));
}

void MessageLogWidget::logSetDoesntUntap(Player *player, QString cardName, bool doesntUntap)
{
	QString finalStr;
	if (doesntUntap)
		finalStr = tr("%1 sets %2 to not untap normally.");
	else
		finalStr = tr("%1 sets %2 to untap normally.");
	append(finalStr.arg(sanitizeHtml(player->getName())).arg(QString("<font color=\"blue\">%1</font>").arg(sanitizeHtml(cardName))));
}

void MessageLogWidget::logSetPT(Player *player, QString cardName, QString newPT)
{
	append(tr("%1 sets PT of %2 to %3.").arg(sanitizeHtml(player->getName())).arg(QString("<font color=\"blue\">%1</font>").arg(sanitizeHtml(cardName))).arg(QString("<font color=\"blue\">%1</font>").arg(sanitizeHtml(newPT))));
}

void MessageLogWidget::logSetAnnotation(Player *player, QString cardName, QString newAnnotation)
{
	append(tr("%1 sets annotation of %2 to %3.").arg(sanitizeHtml(player->getName())).arg(QString("<font color=\"blue\">%1</font>").arg(sanitizeHtml(cardName))).arg(QString("<font color=\"blue\">%1</font>").arg(sanitizeHtml(newAnnotation))));
}

void MessageLogWidget::logDumpZone(Player *player, CardZone *zone, int numberCards)
{
	if (numberCards != -1)
		append(tr("%1 is looking at the top %2 cards %3.").arg(sanitizeHtml(player->getName())).arg(numberCards).arg(zone->getTranslatedName(zone->getPlayer() == player, CaseGenitive)));
	else
		append(tr("%1 is looking at %2.").arg(sanitizeHtml(player->getName())).arg(zone->getTranslatedName(zone->getPlayer() == player, CaseAccusative)));
}

void MessageLogWidget::logStopDumpZone(Player *player, CardZone *zone)
{
	QString zoneName = zone->getTranslatedName(zone->getPlayer() == player, CaseAccusative);
	append(tr("%1 stops looking at %2.").arg(sanitizeHtml(player->getName())).arg(zoneName));
}

void MessageLogWidget::logRevealCards(Player *player, CardZone *zone, int cardId, QString cardName, Player *otherPlayer)
{
	QPair<QString, QString> temp = getFromStr(zone, cardName, cardId);
	bool cardNameContainsStartZone = false;
	if (!temp.first.isEmpty()) {
		cardNameContainsStartZone = true;
		cardName = temp.first;
	}
	QString fromStr = temp.second;

	QString cardStr;
	if (cardNameContainsStartZone)
		cardStr = cardName;
	else if (cardName.isEmpty())
		cardStr = tr("a card");
	else
		cardStr = QString("<font color=\"blue\">%1</font>").arg(sanitizeHtml(cardName));

	if (cardId == -1) {
		if (otherPlayer)
			append(tr("%1 reveals %2 to %3.").arg(sanitizeHtml(player->getName())).arg(zone->getTranslatedName(true, CaseAccusative)).arg(sanitizeHtml(otherPlayer->getName())));
		else
			append(tr("%1 reveals %2.").arg(sanitizeHtml(player->getName())).arg(zone->getTranslatedName(true, CaseAccusative)));
	} else if (cardId == -2) {
		if (otherPlayer)
			append(tr("%1 randomly reveals %2%3 to %4.").arg(sanitizeHtml(player->getName())).arg(cardStr).arg(fromStr).arg(sanitizeHtml(otherPlayer->getName())));
		else
			append(tr("%1 randomly reveals %2%3.").arg(sanitizeHtml(player->getName())).arg(cardStr).arg(fromStr));
	} else {
		if (otherPlayer)
			append(tr("%1 reveals %2%3 to %4.").arg(sanitizeHtml(player->getName())).arg(cardStr).arg(fromStr).arg(sanitizeHtml(otherPlayer->getName())));
		else
			append(tr("%1 reveals %2%3.").arg(sanitizeHtml(player->getName())).arg(cardStr).arg(fromStr));
	}
}

void MessageLogWidget::logSetActivePlayer(Player *player)
{
	append(QString());
	append("<font color=\"green\"><b>" + tr("It is now %1's turn.").arg(player->getName()) + "</b></font>");
	append(QString());
}

void MessageLogWidget::logSetActivePhase(int phase)
{
	QString phaseName;
	switch (phase) {
		case 0: phaseName = tr("untap step"); break;
		case 1: phaseName = tr("upkeep step"); break;
		case 2: phaseName = tr("draw step"); break;
		case 3: phaseName = tr("first main phase"); break;
		case 4: phaseName = tr("beginning of combat step"); break;
		case 5: phaseName = tr("declare attackers step"); break;
		case 6: phaseName = tr("declare blockers step"); break;
		case 7: phaseName = tr("combat damage step"); break;
		case 8: phaseName = tr("end of combat step"); break;
		case 9: phaseName = tr("second main phase"); break;
		case 10: phaseName = tr("ending phase"); break;
	}
	append("<font color=\"green\"><b>" + tr("It is now the %1.").arg(phaseName) + "</b></font>");
}

void MessageLogWidget::connectToPlayer(Player *player)
{
	connect(player, SIGNAL(logSay(Player *, QString)), this, SLOT(logSay(Player *, QString)));
	connect(player, SIGNAL(logShuffle(Player *)), this, SLOT(logShuffle(Player *)));
	connect(player, SIGNAL(logRollDie(Player *, int, int)), this, SLOT(logRollDie(Player *, int, int)));
	connect(player, SIGNAL(logCreateArrow(Player *, Player *, QString, Player *, QString, bool)), this, SLOT(logCreateArrow(Player *, Player *, QString, Player *, QString, bool)));
	connect(player, SIGNAL(logCreateToken(Player *, QString, QString)), this, SLOT(logCreateToken(Player *, QString, QString)));
	connect(player, SIGNAL(logSetCounter(Player *, QString, int, int)), this, SLOT(logSetCounter(Player *, QString, int, int)));
	connect(player, SIGNAL(logSetCardCounter(Player *, QString, int, int, int)), this, SLOT(logSetCardCounter(Player *, QString, int, int, int)));
	connect(player, SIGNAL(logSetTapped(Player *, QString, bool)), this, SLOT(logSetTapped(Player *, QString, bool)));
	connect(player, SIGNAL(logSetDoesntUntap(Player *, QString, bool)), this, SLOT(logSetDoesntUntap(Player *, QString, bool)));
	connect(player, SIGNAL(logSetPT(Player *, QString, QString)), this, SLOT(logSetPT(Player *, QString, QString)));
	connect(player, SIGNAL(logSetAnnotation(Player *, QString, QString)), this, SLOT(logSetAnnotation(Player *, QString, QString)));
	connect(player, SIGNAL(logMoveCard(Player *, QString, CardZone *, int, CardZone *, int)), this, SLOT(logMoveCard(Player *, QString, CardZone *, int, CardZone *, int)));
	connect(player, SIGNAL(logFlipCard(Player *, QString, bool)), this, SLOT(logFlipCard(Player *, QString, bool)));
	connect(player, SIGNAL(logDestroyCard(Player *, QString)), this, SLOT(logDestroyCard(Player *, QString)));
	connect(player, SIGNAL(logAttachCard(Player *, QString, Player *, QString)), this, SLOT(logAttachCard(Player *, QString, Player *, QString)));
	connect(player, SIGNAL(logUnattachCard(Player *, QString)), this, SLOT(logUnattachCard(Player *, QString)));
	connect(player, SIGNAL(logDumpZone(Player *, CardZone *, int)), this, SLOT(logDumpZone(Player *, CardZone *, int)));
	connect(player, SIGNAL(logStopDumpZone(Player *, CardZone *)), this, SLOT(logStopDumpZone(Player *, CardZone *)));
	connect(player, SIGNAL(logDrawCards(Player *, int)), this, SLOT(logDrawCards(Player *, int)));
	connect(player, SIGNAL(logRevealCards(Player *, CardZone *, int, QString, Player *)), this, SLOT(logRevealCards(Player *, CardZone *, int, QString, Player *)));
}

MessageLogWidget::MessageLogWidget(QWidget *parent)
	: QTextEdit(parent)
{
	setReadOnly(true);
	QFont f;
	f.setPixelSize(11);
	setFont(f);
}

void MessageLogWidget::enterEvent(QEvent * /*event*/)
{
	setMouseTracking(true);
}

void MessageLogWidget::leaveEvent(QEvent * /*event*/)
{
	setMouseTracking(false);
}

QString MessageLogWidget::getCardNameUnderMouse(const QPoint &pos) const
{
	QTextCursor cursor(cursorForPosition(pos));
	QTextBlock block(cursor.block());
	QTextBlock::iterator it;
	for (it = block.begin(); !(it.atEnd()); ++it) {
		QTextFragment frag = it.fragment();
		if (!frag.contains(cursor.position()))
			continue;
		
		if (frag.charFormat().foreground().color() == Qt::blue)
			return frag.text();
		
		break;
	}
	return QString();
}

void MessageLogWidget::mouseMoveEvent(QMouseEvent *event)
{
	QString cardName = getCardNameUnderMouse(event->pos());
	if (!cardName.isEmpty()) {
		viewport()->setCursor(Qt::PointingHandCursor);
		emit cardNameHovered(cardName);
	} else
		viewport()->setCursor(Qt::IBeamCursor);

	QTextEdit::mouseMoveEvent(event);
}

void MessageLogWidget::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::MidButton) {
		QString cardName = getCardNameUnderMouse(event->pos());
		if (!cardName.isEmpty())
			emit showCardInfoPopup(event->globalPos(), cardName);
	}
		
	QTextEdit::mousePressEvent(event);
}

void MessageLogWidget::mouseReleaseEvent(QMouseEvent *event)
{
	emit deleteCardInfoPopup();
	
	QTextEdit::mouseReleaseEvent(event);
}
