#include "messagelogwidget.h"
#include "game.h"
#include "player.h"

QString MessageLogWidget::sanitizeHtml(QString dirty)
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
	append(tr("Game has started."));
}

void MessageLogWidget::logSay(Player *player, QString message)
{
	append(QString("<font color=\"red\">%1:</font> %2").arg(sanitizeHtml(player->getName())).arg(sanitizeHtml(message)));
}

void MessageLogWidget::logShuffle(Player *player)
{
	append(tr("%1 shuffles his/her library").arg(sanitizeHtml(player->getName())));
}

void MessageLogWidget::logRollDice(Player *player, int sides, int roll)
{
	append(tr("%1 rolled a %2 with a %3-sided dice").arg(sanitizeHtml(player->getName())).arg(roll).arg(sides));
}

void MessageLogWidget::logDraw(Player *player, int number)
{
	if (number == 1)
		append(tr("%1 draws a card").arg(sanitizeHtml(player->getName())));
	else
		append(tr("%1 draws %2 cards").arg(sanitizeHtml(player->getName())).arg(number));
}

void MessageLogWidget::logMoveCard(Player *player, QString cardName, QString startZone, QString targetZone)
{
	append(tr("%1 moves <font color=\"blue\">%2</font> from %3 to %4").arg(sanitizeHtml(player->getName())).arg(cardName).arg(startZone).arg(targetZone));
}

void MessageLogWidget::logCreateToken(Player *player, QString cardName)
{
	append(tr("%1 creates token: <font color=\"blue\">%2</font>").arg(sanitizeHtml(player->getName())).arg(cardName));
}

void MessageLogWidget::logSetCardCounters(Player *player, QString cardName, int value, int oldValue)
{
	if (value > oldValue)
		append(tr("%1 places %2 counters on <font color=\"blue\">%3</font> (now %4)").arg(sanitizeHtml(player->getName())).arg(value - oldValue).arg(cardName).arg(value));
	else
		append(tr("%1 removes %2 counters from <font color=\"blue\">%3</font> (now %4)").arg(sanitizeHtml(player->getName())).arg(oldValue - value).arg(cardName).arg(value));
}

void MessageLogWidget::logSetTapped(Player *player, QString cardName, bool tapped)
{
	if (cardName == "-1")
		cardName = tr("his permanents");
	append(tr("%1 %2 <font color=\"blue\">%3</blue>").arg(sanitizeHtml(player->getName())).arg(tapped ? "taps" : "untaps").arg(cardName));
}

void MessageLogWidget::logSetCounter(Player *player, QString counterName, int value, int oldValue)
{
	append(tr("%1 sets counter \"%2\" to %3 (%4%5)").arg(sanitizeHtml(player->getName())).arg(counterName).arg(value).arg(value > oldValue ? "+" : "").arg(value - oldValue));
}

void MessageLogWidget::logSetDoesntUntap(Player *player, QString cardName, bool doesntUntap)
{
	if (doesntUntap)
		append(tr("%1 sets <font color=\"blue\">%2</font> to not untap normally.").arg(sanitizeHtml(player->getName())).arg(cardName));
	else
		append(tr("%1 sets <font color=\"blue\">%2</font> to untap normally.").arg(sanitizeHtml(player->getName())).arg(cardName));
}

void MessageLogWidget::logDumpZone(Player *player, QString zoneName, QString zoneOwner, int numberCards)
{
	if (numberCards)
		append(tr("%1 is looking at the top %2 cards of %3's %4").arg(sanitizeHtml(player->getName())).arg(numberCards).arg(zoneOwner).arg(zoneName));
	else
		append(tr("%1 is looking at %2's %3").arg(sanitizeHtml(player->getName())).arg(zoneOwner).arg(zoneName));
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
	connect(game, SIGNAL(logRollDice(Player *, int, int)), this, SLOT(logRollDice(Player *, int, int)));
	connect(game, SIGNAL(logDraw(Player *, int)), this, SLOT(logDraw(Player *, int)));
	connect(game, SIGNAL(logMoveCard(Player *, QString, QString, QString)), this, SLOT(logMoveCard(Player *, QString, QString, QString)));
	connect(game, SIGNAL(logCreateToken(Player *, QString)), this, SLOT(logCreateToken(Player *, QString)));
	connect(game, SIGNAL(logSetCardCounters(Player *, QString, int, int)), this, SLOT(logSetCardCounters(Player *, QString, int, int)));
	connect(game, SIGNAL(logSetTapped(Player *, QString, bool)), this, SLOT(logSetTapped(Player *, QString, bool)));
	connect(game, SIGNAL(logSetCounter(Player *, QString, int, int)), this, SLOT(logSetCounter(Player *, QString, int, int)));
	connect(game, SIGNAL(logSetDoesntUntap(Player *, QString, bool)), this, SLOT(logSetDoesntUntap(Player *, QString, bool)));
	connect(game, SIGNAL(logDumpZone(Player *, QString, QString, int)), this, SLOT(logDumpZone(Player *, QString, QString, int)));
}

MessageLogWidget::MessageLogWidget(QWidget *parent)
	: QTextEdit(parent)
{
	setReadOnly(true);
	QFont f;
	f.setPixelSize(11);
	setFont(f);
}
