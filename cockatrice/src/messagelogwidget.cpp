#include "messagelogwidget.h"
#include "player.h"
#include "cardzone.h"
#include "protocol_items.h"
#include "soundengine.h"
#include <QScrollBar>

QString MessageLogWidget::sanitizeHtml(QString dirty) const
{
	return dirty
		.replace("&", "&amp;")
		.replace("<", "&lt;")
		.replace(">", "&gt;");
}

void MessageLogWidget::myAppend(const QString &message)
{
	QTextCursor cursor(document()->lastBlock());
	cursor.movePosition(QTextCursor::End);
	
	QTextBlockFormat blockFormat;
	blockFormat.setBottomMargin(2);
	cursor.insertBlock(blockFormat);
	
	cursor.insertHtml(message);
	
	verticalScrollBar()->setValue(verticalScrollBar()->maximum());
}

bool MessageLogWidget::isFemale(Player *player) const
{
	return player->getUserInfo()->getGender() == ServerInfo_User::Female;
}

void MessageLogWidget::logConnecting(QString hostname)
{
	myAppend(tr("Connecting to %1...").arg(sanitizeHtml(hostname)));
}

void MessageLogWidget::logConnected()
{
	myAppend(tr("Connected."));
}

void MessageLogWidget::logDisconnected()
{
	myAppend(tr("Disconnected from server."));
}

void MessageLogWidget::logSocketError(const QString &errorString)
{
	myAppend(sanitizeHtml(errorString));
}

void MessageLogWidget::logServerError(ResponseCode response)
{
	switch (response) {
		case RespWrongPassword: myAppend(tr("Invalid password.")); break;
		default: ;
	}
}

void MessageLogWidget::logProtocolVersionMismatch(int clientVersion, int serverVersion)
{
	myAppend(tr("Protocol version mismatch. Client: %1, Server: %2").arg(clientVersion).arg(serverVersion));
}

void MessageLogWidget::logProtocolError()
{
	myAppend(tr("Protocol error."));
}

void MessageLogWidget::logGameJoined(int gameId)
{
	myAppend(tr("You have joined game #%1.").arg(gameId));
}

void MessageLogWidget::logJoin(Player *player)
{
	soundEngine->cuckoo();
	myAppend(tr("%1 has joined the game.").arg(sanitizeHtml(player->getName())));
}

void MessageLogWidget::logLeave(Player *player)
{
	myAppend(tr("%1 has left the game.").arg(sanitizeHtml(player->getName())));
}

void MessageLogWidget::logGameClosed()
{
	myAppend(tr("The game has been closed."));
}

void MessageLogWidget::logJoinSpectator(QString name)
{
	myAppend(tr("%1 is now watching the game.").arg(sanitizeHtml(name)));
}

void MessageLogWidget::logLeaveSpectator(QString name)
{
	myAppend(tr("%1 is not watching the game any more.").arg(sanitizeHtml(name)));
}

void MessageLogWidget::logDeckSelect(Player *player, int deckId)
{
	if (deckId == -1)
		myAppend(tr("%1 has loaded a local deck.").arg(sanitizeHtml(player->getName())));
	else
		myAppend(tr("%1 has loaded deck #%2.").arg(sanitizeHtml(player->getName())).arg(deckId));
}

void MessageLogWidget::logReadyStart(Player *player)
{
	myAppend(tr("%1 is ready to start the game.").arg(sanitizeHtml(player->getName())));
}

void MessageLogWidget::logNotReadyStart(Player *player)
{
	myAppend(tr("%1 is not ready to start the game any more.").arg(sanitizeHtml(player->getName())));
}

void MessageLogWidget::logConcede(Player *player)
{
	myAppend(tr("%1 has conceded the game.").arg(sanitizeHtml(player->getName())));
}

void MessageLogWidget::logGameStart()
{
	myAppend(tr("The game has started."));
}

void MessageLogWidget::logConnectionStateChanged(Player *player, bool connectionState)
{
	if (connectionState)
		myAppend(tr("%1 has restored connection to the game.").arg(sanitizeHtml(player->getName())));
	else
		myAppend(tr("%1 has lost connection to the game.").arg(sanitizeHtml(player->getName())));
}

void MessageLogWidget::logSay(Player *player, QString message)
{
	appendMessage(player->getName(), message, QColor(), true);
}

void MessageLogWidget::logSpectatorSay(QString spectatorName, QString message)
{
	appendMessage(spectatorName, message, QColor(), false);
}

void MessageLogWidget::logShuffle(Player *player, CardZone *zone)
{
	soundEngine->shuffle();
	if (currentContext != MessageContext_Mulligan)
		myAppend(tr("%1 shuffles %2.").arg(sanitizeHtml(player->getName())).arg(zone->getTranslatedName(true, CaseAccusative)));
}

void MessageLogWidget::logRollDie(Player *player, int sides, int roll)
{
	myAppend(tr("%1 rolls a %2 with a %3-sided die.").arg(sanitizeHtml(player->getName())).arg(roll).arg(sides));
}

void MessageLogWidget::logDrawCards(Player *player, int number)
{
	if (currentContext == MessageContext_Mulligan)
		mulliganPlayer = player;
	else {
		soundEngine->draw();
		myAppend(tr("%1 draws %n card(s).", "", number).arg(sanitizeHtml(player->getName())));
	}
}

void MessageLogWidget::logUndoDraw(Player *player, QString cardName)
{
	if (cardName.isEmpty())
		myAppend((isFemale(player) ? tr("%1 undoes her last draw.") : tr("%1 undoes his last draw.")).arg(sanitizeHtml(player->getName())));
	else
		myAppend((isFemale(player) ? tr("%1 undoes her last draw (%2).") : tr("%1 undoes his last draw (%2).")).arg(sanitizeHtml(player->getName())).arg(QString("<font color=\"blue\">%1</font>").arg(sanitizeHtml(cardName))));
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
				cardName = isFemale(zone->getPlayer()) ? tr("the bottom card of her library") : tr("the bottom card of his library");
				cardNameContainsStartZone = true;
			} else
				fromStr = isFemale(zone->getPlayer()) ? tr(" from the bottom of her library") : tr(" from the bottom of his library");
		} else if (position == 0) {
			if (cardName.isEmpty()) {
				cardName = isFemale(zone->getPlayer()) ? tr("the top card of her library") : tr("the top card of his library");
				cardNameContainsStartZone = true;
			} else
				fromStr = isFemale(zone->getPlayer()) ? tr(" from the top of her library") : tr(" from the top of his library");
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

void MessageLogWidget::doMoveCard(LogMoveCard &attributes)
{
	QString startName = attributes.startZone->getName();
	QString targetName = attributes.targetZone->getName();
	if (((startName == "table") && (targetName == "table") && (attributes.startZone == attributes.targetZone)) || ((startName == "hand") && (targetName == "hand")))
		return;
	QString cardName = attributes.cardName;
	QPair<QString, QString> temp = getFromStr(attributes.startZone, cardName, attributes.oldX);
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

	if (attributes.startZone->getPlayer() != attributes.targetZone->getPlayer()) {
		myAppend(tr("%1 gives %2 control over %3.").arg(sanitizeHtml(attributes.player->getName())).arg(sanitizeHtml(attributes.targetZone->getPlayer()->getName())).arg(cardStr));
		return;
	}
	
	QString finalStr;
	if (targetName == "table") {
		soundEngine->playCard();
		if (moveCardTapped.value(attributes.card))
			finalStr = tr("%1 puts %2 into play tapped%3.");
		else
			finalStr = tr("%1 puts %2 into play%3.");
	} else if (targetName == "grave")
		finalStr = tr("%1 puts %2%3 into graveyard.");
	else if (targetName == "rfg")
		finalStr = tr("%1 exiles %2%3.");
	else if (targetName == "hand")
		finalStr = tr("%1 moves %2%3 to hand.");
	else if (targetName == "deck") {
		if (attributes.newX == -1)
			finalStr = isFemale(attributes.targetZone->getPlayer()) ? tr("%1 puts %2%3 into her library.") : tr("%1 puts %2%3 into his library.");
		else if (attributes.newX == attributes.targetZone->getCards().size() - 1)
			finalStr = isFemale(attributes.targetZone->getPlayer()) ? tr("%1 puts %2%3 on bottom of her library.") : tr("%1 puts %2%3 on bottom of his library.");
		else if (attributes.newX == 0)
			finalStr = isFemale(attributes.targetZone->getPlayer()) ? tr("%1 puts %2%3 on top of her library.") : tr("%1 puts %2%3 on top of his library.");
		else
			finalStr = isFemale(attributes.targetZone->getPlayer()) ? tr("%1 puts %2%3 into her library at position %4.") : tr("%1 puts %2%3 into his library at position %4.");
	} else if (targetName == "sb")
		finalStr = tr("%1 moves %2%3 to sideboard.");
	else if (targetName == "stack") {
		soundEngine->playCard();
		finalStr = tr("%1 plays %2%3.");
	}
	
	myAppend(finalStr.arg(sanitizeHtml(attributes.player->getName())).arg(cardStr).arg(fromStr).arg(attributes.newX));
}

void MessageLogWidget::logMoveCard(Player *player, CardItem *card, CardZone *startZone, int oldX, CardZone *targetZone, int newX)
{
	LogMoveCard attributes = {player, card, card->getName(), startZone, oldX, targetZone, newX};
	if (currentContext == MessageContext_MoveCard)
		moveCardQueue.append(attributes);
	else if (currentContext == MessageContext_Mulligan)
		mulliganPlayer = player;
	else
		doMoveCard(attributes);
}

void MessageLogWidget::logMulligan(Player *player, int number)
{
	if (!player)
		return;

	if (number > -1)
		myAppend(tr("%1 takes a mulligan to %n.", "", number).arg(sanitizeHtml(player->getName())));
	else
		myAppend((isFemale(player) ? tr("%1 draws her initial hand.") : tr("%1 draws his initial hand.")).arg(sanitizeHtml(player->getName())));
}

void MessageLogWidget::logFlipCard(Player *player, QString cardName, bool faceDown)
{
	if (faceDown)
		myAppend(tr("%1 flips %2 face-down.").arg(sanitizeHtml(player->getName())).arg(cardName));
	else
		myAppend(tr("%1 flips %2 face-up.").arg(sanitizeHtml(player->getName())).arg(cardName));
}

void MessageLogWidget::logDestroyCard(Player *player, QString cardName)
{
	myAppend(tr("%1 destroys %2.").arg(sanitizeHtml(player->getName())).arg(QString("<font color=\"blue\">%1</font>").arg(sanitizeHtml(cardName))));
}

void MessageLogWidget::logAttachCard(Player *player, QString cardName, Player *targetPlayer, QString targetCardName)
{
	myAppend(tr("%1 attaches %2 to %3's %4.").arg(sanitizeHtml(player->getName())).arg(QString("<font color=\"blue\">%1</font>").arg(sanitizeHtml(cardName))).arg(sanitizeHtml(targetPlayer->getName())).arg(QString("<font color=\"blue\">%1</font>").arg(sanitizeHtml(targetCardName))));
}

void MessageLogWidget::logUnattachCard(Player *player, QString cardName)
{
	myAppend(tr("%1 unattaches %2.").arg(sanitizeHtml(player->getName())).arg(QString("<font color=\"blue\">%1</font>").arg(sanitizeHtml(cardName))));
}

void MessageLogWidget::logCreateToken(Player *player, QString cardName, QString pt)
{
	myAppend(tr("%1 creates token: %2%3.").arg(sanitizeHtml(player->getName())).arg(QString("<font color=\"blue\"><a name=\"foo\">%1</a></font>").arg(sanitizeHtml(cardName))).arg(pt.isEmpty() ? QString() : QString(" (%1)").arg(sanitizeHtml(pt))));
}

void MessageLogWidget::logCreateArrow(Player *player, Player *startPlayer, QString startCard, Player *targetPlayer, QString targetCard, bool playerTarget)
{
	if (playerTarget)
		myAppend(tr("%1 points from %2's %3 to %4.")
			.arg(sanitizeHtml(player->getName()))
			.arg(sanitizeHtml(startPlayer->getName()))
			.arg(sanitizeHtml(startCard))
			.arg(sanitizeHtml(targetPlayer->getName()))
		);
	else
		myAppend(tr("%1 points from %2's %3 to %4's %5.")
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
		finalStr = tr("%1 places %n %2 counter(s) on %3 (now %4).", "", delta);
	else
		finalStr = tr("%1 removes %n %2 counter(s) from %3 (now %4).", "", delta);
	
	switch (counterId) {
		case 0: colorStr = tr("red", "", delta); break;
		case 1: colorStr = tr("yellow", "", delta); break;
		case 2: colorStr = tr("green", "", delta); break;
		default: ;
	}
	
	myAppend(finalStr.arg(sanitizeHtml(player->getName())).arg(colorStr).arg(QString("<font color=\"blue\">%1</font>").arg(sanitizeHtml(cardName))).arg(value));
}

void MessageLogWidget::logSetTapped(Player *player, CardItem *card, bool tapped)
{
	if (tapped)
		soundEngine->tap();
	else
		soundEngine->untap();
	
	if (currentContext == MessageContext_MoveCard)
		moveCardTapped.insert(card, tapped);
	else {
		QString cardStr;
		if (!card)
			cardStr = isFemale(player) ? tr("her permanents") : tr("his permanents");
		else
			cardStr = QString("<font color=\"blue\">%1</font>").arg(sanitizeHtml(card->getName()));
		myAppend(tr("%1 %2 %3.").arg(sanitizeHtml(player->getName())).arg(tapped ? tr("taps") : tr("untaps")).arg(cardStr));
	}
}

void MessageLogWidget::logSetCounter(Player *player, QString counterName, int value, int oldValue)
{
	myAppend(tr("%1 sets counter %2 to %3 (%4%5).").arg(sanitizeHtml(player->getName())).arg(QString("<font color=\"blue\">%1</font>").arg(sanitizeHtml(counterName))).arg(QString("<font color=\"blue\">%1</font>").arg(value)).arg(value > oldValue ? "+" : "").arg(value - oldValue));
}

void MessageLogWidget::logSetDoesntUntap(Player *player, CardItem *card, bool doesntUntap)
{
	QString finalStr;
	if (doesntUntap)
		finalStr = tr("%1 sets %2 to not untap normally.");
	else
		finalStr = tr("%1 sets %2 to untap normally.");
	myAppend(finalStr.arg(sanitizeHtml(player->getName())).arg(QString("<font color=\"blue\">%1</font>").arg(sanitizeHtml(card->getName()))));
}

void MessageLogWidget::logSetPT(Player *player, CardItem *card, QString newPT)
{
	if (currentContext == MessageContext_MoveCard)
		moveCardPT.insert(card, newPT);
	else
		myAppend(tr("%1 sets PT of %2 to %3.").arg(sanitizeHtml(player->getName())).arg(QString("<font color=\"blue\">%1</font>").arg(sanitizeHtml(card->getName()))).arg(QString("<font color=\"blue\">%1</font>").arg(sanitizeHtml(newPT))));
}

void MessageLogWidget::logSetAnnotation(Player *player, CardItem *card, QString newAnnotation)
{
	myAppend(tr("%1 sets annotation of %2 to %3.").arg(sanitizeHtml(player->getName())).arg(QString("<font color=\"blue\">%1</font>").arg(sanitizeHtml(card->getName()))).arg(QString("<font color=\"blue\">%1</font>").arg(sanitizeHtml(newAnnotation))));
}

void MessageLogWidget::logDumpZone(Player *player, CardZone *zone, int numberCards)
{
	if (numberCards != -1)
		myAppend(tr("%1 is looking at the top %2 cards %3.").arg(sanitizeHtml(player->getName())).arg(numberCards).arg(zone->getTranslatedName(zone->getPlayer() == player, CaseGenitive)));
	else
		myAppend(tr("%1 is looking at %2.").arg(sanitizeHtml(player->getName())).arg(zone->getTranslatedName(zone->getPlayer() == player, CaseAccusative)));
}

void MessageLogWidget::logStopDumpZone(Player *player, CardZone *zone)
{
	QString zoneName = zone->getTranslatedName(zone->getPlayer() == player, CaseAccusative);
	myAppend(tr("%1 stops looking at %2.").arg(sanitizeHtml(player->getName())).arg(zoneName));
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
			myAppend(tr("%1 reveals %2 to %3.").arg(sanitizeHtml(player->getName())).arg(zone->getTranslatedName(true, CaseAccusative)).arg(sanitizeHtml(otherPlayer->getName())));
		else
			myAppend(tr("%1 reveals %2.").arg(sanitizeHtml(player->getName())).arg(zone->getTranslatedName(true, CaseAccusative)));
	} else if (cardId == -2) {
		if (otherPlayer)
			myAppend(tr("%1 randomly reveals %2%3 to %4.").arg(sanitizeHtml(player->getName())).arg(cardStr).arg(fromStr).arg(sanitizeHtml(otherPlayer->getName())));
		else
			myAppend(tr("%1 randomly reveals %2%3.").arg(sanitizeHtml(player->getName())).arg(cardStr).arg(fromStr));
	} else {
		if (otherPlayer)
			myAppend(tr("%1 reveals %2%3 to %4.").arg(sanitizeHtml(player->getName())).arg(cardStr).arg(fromStr).arg(sanitizeHtml(otherPlayer->getName())));
		else
			myAppend(tr("%1 reveals %2%3.").arg(sanitizeHtml(player->getName())).arg(cardStr).arg(fromStr));
	}
}

void MessageLogWidget::logSetActivePlayer(Player *player)
{
	soundEngine->notification();
	myAppend(QString());
	myAppend("<font color=\"green\"><b>" + tr("It is now %1's turn.").arg(player->getName()) + "</b></font>");
	myAppend(QString());
}

void MessageLogWidget::logSetActivePhase(int phase)
{
	soundEngine->notification();
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
	myAppend("<font color=\"green\"><b>" + tr("It is now the %1.").arg(phaseName) + "</b></font>");
}

void MessageLogWidget::containerProcessingStarted(GameEventContext *_context)
{
	if (qobject_cast<Context_MoveCard *>(_context))
		currentContext = MessageContext_MoveCard;
	else if (qobject_cast<Context_Mulligan *>(_context)) {
		currentContext = MessageContext_Mulligan;
		mulliganPlayer = 0;
		mulliganNumber = static_cast<Context_Mulligan *>(_context)->getNumber();
	}
}

void MessageLogWidget::containerProcessingDone()
{
	if (currentContext == MessageContext_MoveCard) {
		for (int i = 0; i < moveCardQueue.size(); ++i)
			doMoveCard(moveCardQueue[i]);
		moveCardQueue.clear();
		moveCardPT.clear();
		moveCardTapped.clear();
	} else if (currentContext == MessageContext_Mulligan) {
		logMulligan(mulliganPlayer, mulliganNumber);
		mulliganPlayer = 0;
		mulliganNumber = 0;
	}
	
	currentContext = MessageContext_None;
}

void MessageLogWidget::connectToPlayer(Player *player)
{
	connect(player, SIGNAL(logConnectionStateChanged(Player *, bool)), this, SLOT(logConnectionStateChanged(Player *, bool)));
	connect(player, SIGNAL(logSay(Player *, QString)), this, SLOT(logSay(Player *, QString)));
	connect(player, SIGNAL(logShuffle(Player *, CardZone *)), this, SLOT(logShuffle(Player *, CardZone *)));
	connect(player, SIGNAL(logRollDie(Player *, int, int)), this, SLOT(logRollDie(Player *, int, int)));
	connect(player, SIGNAL(logCreateArrow(Player *, Player *, QString, Player *, QString, bool)), this, SLOT(logCreateArrow(Player *, Player *, QString, Player *, QString, bool)));
	connect(player, SIGNAL(logCreateToken(Player *, QString, QString)), this, SLOT(logCreateToken(Player *, QString, QString)));
	connect(player, SIGNAL(logSetCounter(Player *, QString, int, int)), this, SLOT(logSetCounter(Player *, QString, int, int)));
	connect(player, SIGNAL(logSetCardCounter(Player *, QString, int, int, int)), this, SLOT(logSetCardCounter(Player *, QString, int, int, int)));
	connect(player, SIGNAL(logSetTapped(Player *, CardItem *, bool)), this, SLOT(logSetTapped(Player *, CardItem *, bool)));
	connect(player, SIGNAL(logSetDoesntUntap(Player *, CardItem *, bool)), this, SLOT(logSetDoesntUntap(Player *, CardItem *, bool)));
	connect(player, SIGNAL(logSetPT(Player *, CardItem *, QString)), this, SLOT(logSetPT(Player *, CardItem *, QString)));
	connect(player, SIGNAL(logSetAnnotation(Player *, CardItem *, QString)), this, SLOT(logSetAnnotation(Player *, CardItem *, QString)));
	connect(player, SIGNAL(logMoveCard(Player *, CardItem *, CardZone *, int, CardZone *, int)), this, SLOT(logMoveCard(Player *, CardItem *, CardZone *, int, CardZone *, int)));
	connect(player, SIGNAL(logFlipCard(Player *, QString, bool)), this, SLOT(logFlipCard(Player *, QString, bool)));
	connect(player, SIGNAL(logDestroyCard(Player *, QString)), this, SLOT(logDestroyCard(Player *, QString)));
	connect(player, SIGNAL(logAttachCard(Player *, QString, Player *, QString)), this, SLOT(logAttachCard(Player *, QString, Player *, QString)));
	connect(player, SIGNAL(logUnattachCard(Player *, QString)), this, SLOT(logUnattachCard(Player *, QString)));
	connect(player, SIGNAL(logDumpZone(Player *, CardZone *, int)), this, SLOT(logDumpZone(Player *, CardZone *, int)));
	connect(player, SIGNAL(logStopDumpZone(Player *, CardZone *)), this, SLOT(logStopDumpZone(Player *, CardZone *)));
	connect(player, SIGNAL(logDrawCards(Player *, int)), this, SLOT(logDrawCards(Player *, int)));
	connect(player, SIGNAL(logUndoDraw(Player *, QString)), this, SLOT(logUndoDraw(Player *, QString)));
	connect(player, SIGNAL(logRevealCards(Player *, CardZone *, int, QString, Player *)), this, SLOT(logRevealCards(Player *, CardZone *, int, QString, Player *)));
}

MessageLogWidget::MessageLogWidget(const QString &_ownName, QWidget *parent)
	: ChatView(_ownName, false, parent)
{
}
