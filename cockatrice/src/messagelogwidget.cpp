#include "messagelogwidget.h"
#include "player.h"
#include "cardzone.h"
#include <QApplication>

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
	append(QString("<font color=\"red\">%1:</font> %2").arg(sanitizeHtml(player->getName())).arg(sanitizeHtml(message)));
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

void MessageLogWidget::logMoveCard(Player *player, QString cardName, CardZone *startZone, int oldX, CardZone *targetZone, int newX)
{
	QString startName = startZone->getName();
	QString targetName = targetZone->getName();
	if (((startName == "table") && (targetName == "table")) || ((startName == "hand") && (targetName == "hand")))
		return;
	QString fromStr;
	bool cardNameContainsStartZone = false;
	
	if (startName == "table")
		fromStr = tr(" from table");
	else if (startName == "grave")
		fromStr = tr(" from graveyard");
	else if (startName == "rfg")
		fromStr = tr(" from exile");
	else if (startName == "hand")
		fromStr = tr(" from hand");
	else if (startName == "deck") {
		if (oldX == startZone->getCards().size() - 1) {
			if (cardName.isEmpty()) {
				cardName = tr("the bottom card of his library");
				cardNameContainsStartZone = true;
			} else
				fromStr = tr(" from the bottom of his library");
		} else if (oldX == 0) {
			if (cardName.isEmpty()) {
				cardName = tr("the top card of his library");
				cardNameContainsStartZone = true;
			} else
				fromStr = tr(" from the top of his library");
		} else
			fromStr = tr(" from library");
	} else if (startName == "sb")
		fromStr = tr(" from sideboard");
	
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
	
	QString cardStr;
	if (cardNameContainsStartZone)
		cardStr = cardName;
	else if (cardName.isEmpty())
		cardStr = tr("a card");
	else
		cardStr = QString("<font color=\"blue\">%1</font>").arg(sanitizeHtml(cardName));

	append(finalStr.arg(sanitizeHtml(player->getName())).arg(cardStr).arg(fromStr).arg(newX));
}

void MessageLogWidget::logCreateToken(Player *player, QString cardName)
{
	append(tr("%1 creates token: %2.").arg(sanitizeHtml(player->getName())).arg(QString("<font color=\"blue\">%1</font>").arg(sanitizeHtml(cardName))));
}

void MessageLogWidget::logCreateArrow(Player *player, Player *startPlayer, QString startCard, Player *targetPlayer, QString targetCard)
{
	append(tr("%1 points from %2's %3 to %4's %5.")
		.arg(sanitizeHtml(player->getName()))
		.arg(sanitizeHtml(startPlayer->getName()))
		.arg(sanitizeHtml(startCard))
		.arg(sanitizeHtml(targetPlayer->getName()))
		.arg(sanitizeHtml(targetCard))
	);
}

void MessageLogWidget::logSetCardCounters(Player *player, QString cardName, int value, int oldValue)
{
	QString finalStr;
	if (value > oldValue)
		finalStr = tr("%1 places %2 counters on %3 (now %4).");
	else
		finalStr = tr("%1 removes %2 counters from %3 (now %4).");
	append(finalStr.arg(sanitizeHtml(player->getName())).arg(abs(oldValue - value)).arg(QString("<font color=\"blue\">%1</font>").arg(sanitizeHtml(cardName))).arg(value));
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
	append(tr("%1 sets counter \"%2\" to %3 (%4%5).").arg(sanitizeHtml(player->getName())).arg(counterName).arg(value).arg(value > oldValue ? "+" : "").arg(value - oldValue));
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

void MessageLogWidget::logSetActivePlayer(Player *player)
{
	append("---");
	append("<font color=\"green\">" + tr("It is now %1's turn.").arg(player->getName()) + "</font>");
	append("---");
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
	append("<font color=\"green\">" + tr("It is now the %1.").arg(phaseName) + "</font>");
}

void MessageLogWidget::connectToPlayer(Player *player)
{
	connect(player, SIGNAL(logSay(Player *, QString)), this, SLOT(logSay(Player *, QString)));
	connect(player, SIGNAL(logDeckSelect(Player *, int)), this, SLOT(logDeckSelect(Player *, int)));
	connect(player, SIGNAL(logReadyStart(Player *)), this, SLOT(logReadyStart(Player *)));
	connect(player, SIGNAL(logConcede(Player *)), this, SLOT(logConcede(Player *)));
	connect(player, SIGNAL(logShuffle(Player *)), this, SLOT(logShuffle(Player *)));
	connect(player, SIGNAL(logRollDie(Player *, int, int)), this, SLOT(logRollDie(Player *, int, int)));
	connect(player, SIGNAL(logCreateArrow(Player *, Player *, QString, Player *, QString)), this, SLOT(logCreateArrow(Player *, Player *, QString, Player *, QString)));
	connect(player, SIGNAL(logCreateToken(Player *, QString)), this, SLOT(logCreateToken(Player *, QString)));
	connect(player, SIGNAL(logSetCounter(Player *, QString, int, int)), this, SLOT(logSetCounter(Player *, QString, int, int)));
	connect(player, SIGNAL(logSetCardCounters(Player *, QString, int, int)), this, SLOT(logSetCardCounters(Player *, QString, int, int)));
	connect(player, SIGNAL(logSetTapped(Player *, QString, bool)), this, SLOT(logSetTapped(Player *, QString, bool)));
	connect(player, SIGNAL(logSetDoesntUntap(Player *, QString, bool)), this, SLOT(logSetDoesntUntap(Player *, QString, bool)));
	connect(player, SIGNAL(logMoveCard(Player *, QString, CardZone *, int, CardZone *, int)), this, SLOT(logMoveCard(Player *, QString, CardZone *, int, CardZone *, int)));
	connect(player, SIGNAL(logDumpZone(Player *, CardZone *, int)), this, SLOT(logDumpZone(Player *, CardZone *, int)));
	connect(player, SIGNAL(logStopDumpZone(Player *, CardZone *)), this, SLOT(logStopDumpZone(Player *, CardZone *)));
	connect(player, SIGNAL(logDrawCards(Player *, int)), this, SLOT(logDrawCards(Player *, int)));
/*
	connect(game, SIGNAL(logJoin(Player *)), this, SLOT(logJoin(Player *)));
	connect(game, SIGNAL(logLeave(Player *)), this, SLOT(logLeave(Player *)));
	connect(game, SIGNAL(logGameClosed()), this, SLOT(logGameClosed()));
	connect(game, SIGNAL(logJoinSpectator(QString)), this, SLOT(logJoinSpectator(QString)));
	connect(game, SIGNAL(logLeaveSpectator(QString)), this, SLOT(logLeaveSpectator(QString)));
	
	//Alert Test
	connect(game, SIGNAL(logSay(Player *, QString)), this, SLOT(msgAlert()));
	connect(game, SIGNAL(logJoin(Player *)), this, SLOT(msgAlert()));
	connect(game, SIGNAL(logLeave(Player *)), this, SLOT(msgAlert()));
	connect(game, SIGNAL(logSetActivePlayer(Player *)), this, SLOT(msgAlert()));
	connect(game, SIGNAL(setActivePhase(int)), this, SLOT(msgAlert()));
	connect(game, SIGNAL(logDraw(Player *, int)), this, SLOT(msgAlert()));
	connect(game, SIGNAL(logMoveCard(Player *, QString, CardZone *, int, CardZone *, int)), this, SLOT(msgAlert()));
	connect(game, SIGNAL(logGameStart()), this, SLOT(msgAlert()));
*/}

void MessageLogWidget::msgAlert()
{
	QApplication::alert(this);
}

MessageLogWidget::MessageLogWidget(QWidget *parent)
	: QTextEdit(parent)
{
	setReadOnly(true);
	QFont f;
	f.setPixelSize(11);
	setFont(f);
}
