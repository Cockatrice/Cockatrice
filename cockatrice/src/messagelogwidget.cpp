#include "messagelogwidget.h"
#include "game.h"
#include "player.h"
#include "cardzone.h"

QString MessageLogWidget::sanitizeHtml(QString dirty) const
{
	return dirty
		.replace("&", "&amp;")
		.replace("<", "&lt;")
		.replace(">", "&gt;");
}

QString MessageLogWidget::trZoneName(CardZone *zone, Player *owner, bool hisOwn, GrammaticalCase gc) const
{
	if (zone->getName() == "hand")
		switch (gc) {
//			case CaseNominative: return hisOwn ? tr("his hand") : tr("%1's hand").arg(owner->getName());
			case CaseGenitive: return hisOwn ? tr("of his hand") : tr("of %1's hand").arg(owner->getName());
			case CaseAccusative: return hisOwn ? tr("his hand") : tr("%1's hand").arg(owner->getName());
		}
	else if (zone->getName() == "deck")
		switch (gc) {
//			case CaseNominative: return hisOwn ? tr("his library") : tr("%1's library").arg(owner->getName());
			case CaseGenitive: return hisOwn ? tr("of his library") : tr("of %1's library").arg(owner->getName());
			case CaseAccusative: return hisOwn ? tr("his library") : tr("%1's library").arg(owner->getName());
		}
	else if (zone->getName() == "sb")
		switch (gc) {
//			case CaseNominative: return hisOwn ? tr("his sideboard") : tr("%1's sideboard").arg(owner->getName());
			case CaseGenitive: return hisOwn ? tr("of his sideboard") : tr("of %1's sideboard").arg(owner->getName());
			case CaseAccusative: return hisOwn ? tr("his sideboard") : tr("%1's sideboard").arg(owner->getName());
		}
	return QString();
}

void MessageLogWidget::logConnecting(QString hostname)
{
	append(tr("Connecting to %1...").arg(sanitizeHtml(hostname)));
}

void MessageLogWidget::logConnected(const QStringList WelcomeMsg)
{
	append(tr("Connected."));

	QStringListIterator i(WelcomeMsg);
	while (i.hasNext())
		append(i.next());
}

void MessageLogWidget::logDisconnected()
{
	append(tr("Disconnected from server."));
}

void MessageLogWidget::logSocketError(const QString &errorString)
{
	append(sanitizeHtml(errorString));
}

void MessageLogWidget::logServerError(ServerResponse response)
{
	switch (response) {
		case RespPassword: append(tr("Invalid password.")); break;
		default: ;
	}
}

void MessageLogWidget::logPlayerListReceived(QStringList players)
{
	append("---");
	append(tr("You have joined the game. Player list:"));
	for (int i = 0; i < players.size(); i++)
		append(sanitizeHtml(players.at(i)));
	append("---");
}

void MessageLogWidget::logJoin(Player *player)
{
	append(tr("%1 has joined the game").arg(sanitizeHtml(player->getName())));
}

void MessageLogWidget::logLeave(Player *player)
{
	append(tr("%1 has left the game").arg(sanitizeHtml(player->getName())));
}

void MessageLogWidget::logReadyStart(Player *player)
{
	append(tr("%1 is ready to start a new game.").arg(sanitizeHtml(player->getName())));
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

void MessageLogWidget::logDraw(Player *player, int number)
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

void MessageLogWidget::logDumpZone(Player *player, CardZone *zone, Player *zoneOwner, int numberCards)
{
	if (numberCards != -1)
		append(tr("%1 is looking at the top %2 cards %3.").arg(sanitizeHtml(player->getName())).arg(numberCards).arg(trZoneName(zone, zoneOwner, zoneOwner == player, CaseGenitive)));
	else
		append(tr("%1 is looking at %2.").arg(sanitizeHtml(player->getName())).arg(trZoneName(zone, zoneOwner, zoneOwner == player, CaseAccusative)));
}

void MessageLogWidget::logStopDumpZone(Player *player, CardZone *zone, Player *zoneOwner)
{
	QString zoneName = trZoneName(zone, zoneOwner, zoneOwner == player, CaseAccusative);
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

void MessageLogWidget::connectToGame(Game *game)
{
	connect(game, SIGNAL(logPlayerListReceived(QStringList)), this, SLOT(logPlayerListReceived(QStringList)));
	connect(game, SIGNAL(logJoin(Player *)), this, SLOT(logJoin(Player *)));
	connect(game, SIGNAL(logLeave(Player *)), this, SLOT(logLeave(Player *)));
	connect(game, SIGNAL(logReadyStart(Player *)), this, SLOT(logReadyStart(Player *)));
	connect(game, SIGNAL(logGameStart()), this, SLOT(logGameStart()));
	connect(game, SIGNAL(logSay(Player *, QString)), this, SLOT(logSay(Player *, QString)));
	connect(game, SIGNAL(logShuffle(Player *)), this, SLOT(logShuffle(Player *)));
	connect(game, SIGNAL(logRollDie(Player *, int, int)), this, SLOT(logRollDie(Player *, int, int)));
	connect(game, SIGNAL(logDraw(Player *, int)), this, SLOT(logDraw(Player *, int)));
	connect(game, SIGNAL(logMoveCard(Player *, QString, CardZone *, int, CardZone *, int)), this, SLOT(logMoveCard(Player *, QString, CardZone *, int, CardZone *, int)));
	connect(game, SIGNAL(logCreateToken(Player *, QString)), this, SLOT(logCreateToken(Player *, QString)));
	connect(game, SIGNAL(logSetCardCounters(Player *, QString, int, int)), this, SLOT(logSetCardCounters(Player *, QString, int, int)));
	connect(game, SIGNAL(logSetTapped(Player *, QString, bool)), this, SLOT(logSetTapped(Player *, QString, bool)));
	connect(game, SIGNAL(logSetCounter(Player *, QString, int, int)), this, SLOT(logSetCounter(Player *, QString, int, int)));
	connect(game, SIGNAL(logSetDoesntUntap(Player *, QString, bool)), this, SLOT(logSetDoesntUntap(Player *, QString, bool)));
	connect(game, SIGNAL(logDumpZone(Player *, CardZone *, Player *, int)), this, SLOT(logDumpZone(Player *, CardZone *, Player *, int)));
	connect(game, SIGNAL(logStopDumpZone(Player *, CardZone *, Player *)), this, SLOT(logStopDumpZone(Player *, CardZone *, Player *)));
	connect(game, SIGNAL(logSetActivePlayer(Player *)), this, SLOT(logSetActivePlayer(Player *)));
	connect(game, SIGNAL(setActivePhase(int)), this, SLOT(logSetActivePhase(int)));
}

MessageLogWidget::MessageLogWidget(QWidget *parent)
	: QTextEdit(parent)
{
	setReadOnly(true);
	QFont f;
	f.setPixelSize(11);
	setFont(f);
}
