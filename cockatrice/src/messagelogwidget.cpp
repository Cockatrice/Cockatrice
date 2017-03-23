#include "messagelogwidget.h"
#include "player.h"
#include "cardzone.h"
#include "carditem.h"
#include "soundengine.h"
#include "tab_supervisor.h"
#include "settingscache.h"
#include "pb/serverinfo_user.pb.h"
#include "pb/context_move_card.pb.h"
#include "pb/context_mulligan.pb.h"
#include <QScrollBar>
#include <QDateTime>

QString MessageLogWidget::sanitizeHtml(QString dirty) const
{
    return dirty
        .replace("&", "&amp;")
        .replace("<", "&lt;")
        .replace(">", "&gt;")
        .replace("\"", "&quot;");
}

QString MessageLogWidget::cardLink(const QString &cardName) const
{
    return QString("<i><a href=\"card://%1\">%2</a></i>").arg(cardName).arg(cardName);
}

void MessageLogWidget::logGameJoined(int gameId)
{
    soundEngine->playSound("player_join");
        appendHtmlServerMessage(tr("You have joined game #%1.").arg("<font color=\"blue\">"+ QString::number(gameId) + "</font>"));
}

void MessageLogWidget::logReplayStarted(int gameId)
{
        appendHtmlServerMessage(tr("You are watching a replay of game #%1.").arg(gameId));
}

void MessageLogWidget::logJoin(Player *player)
{
    soundEngine->playSound("player_join");
    appendHtmlServerMessage(tr("%1 has joined the game.").arg(sanitizeHtml(player->getName())));
}

void MessageLogWidget::logLeave(Player *player)
{
    soundEngine->playSound("player_leave");
    appendHtmlServerMessage(tr("%1 has left the game.").arg(sanitizeHtml(player->getName())));
}

void MessageLogWidget::logGameClosed()
{
    appendHtmlServerMessage(tr("The game has been closed."));
}

void MessageLogWidget::logKicked()
{
    appendHtmlServerMessage(tr("You have been kicked out of the game."), true);
}

void MessageLogWidget::logJoinSpectator(QString name)
{
    soundEngine->playSound("spectator_join");
    appendHtmlServerMessage(tr("%1 is now watching the game.").arg(sanitizeHtml(name)));
}

void MessageLogWidget::logLeaveSpectator(QString name)
{
    soundEngine->playSound("spectator_leave");
    appendHtmlServerMessage(tr("%1 is not watching the game any more.").arg(sanitizeHtml(name)));
}

void MessageLogWidget::logDeckSelect(Player *player, QString deckHash, int sideboardSize)
{
    if (sideboardSize < 0)
        appendHtmlServerMessage(tr("%1 has loaded a deck (%2).").arg(sanitizeHtml(player->getName())).arg(deckHash));
    else
        appendHtmlServerMessage(tr("%1 has loaded a deck with %2 sideboard cards (%3).").
                arg(sanitizeHtml(player->getName())).
                arg("<font color=\"blue\">" + QString::number(sideboardSize) + "</font>").
                arg(deckHash));
}

void MessageLogWidget::logReadyStart(Player *player)
{
    appendHtmlServerMessage(tr("%1 is ready to start the game.").arg(sanitizeHtml(player->getName())));
}

void MessageLogWidget::logNotReadyStart(Player *player)
{
    appendHtmlServerMessage(tr("%1 is not ready to start the game any more.").arg(sanitizeHtml(player->getName())));
}

void MessageLogWidget::logSetSideboardLock(Player *player, bool locked)
{
    if (locked) {
          appendHtmlServerMessage(tr("%1 has locked their sideboard.").arg(sanitizeHtml(player->getName())));
    } else {
          appendHtmlServerMessage(tr("%1 has unlocked their sideboard.").arg(sanitizeHtml(player->getName())));
    }
}

void MessageLogWidget::logConcede(Player *player)
{
    soundEngine->playSound("player_concede");
    appendHtmlServerMessage(tr("%1 has conceded the game.").arg(sanitizeHtml(player->getName())));
}

void MessageLogWidget::logGameStart()
{
    appendHtmlServerMessage(tr("The game has started."));
}

void MessageLogWidget::logConnectionStateChanged(Player *player, bool connectionState)
{
    if (connectionState) {
        soundEngine->playSound("player_reconnect");
        appendHtmlServerMessage(tr("%1 has restored connection to the game.").arg(sanitizeHtml(player->getName())));
    } else {
        soundEngine->playSound("player_disconnect");
        appendHtmlServerMessage(tr("%1 has lost connection to the game.").arg(sanitizeHtml(player->getName())));
    }
}

void MessageLogWidget::logSay(Player *player, QString message)
{
    appendMessage(message, 0, player->getName(), UserLevelFlags(player->getUserInfo()->user_level()), QString::fromStdString(player->getUserInfo()->privlevel()), true);
}

void MessageLogWidget::logSpectatorSay(QString spectatorName, UserLevelFlags spectatorUserLevel, QString userPrivLevel, QString message)
{
    appendMessage(message, 0, spectatorName, spectatorUserLevel, userPrivLevel, false);
}

void MessageLogWidget::logShuffle(Player *player, CardZone *zone)
{
    soundEngine->playSound("shuffle");
    if (currentContext != MessageContext_Mulligan) {
        appendHtmlServerMessage(
         tr("%1 shuffles %2.")
         .arg(sanitizeHtml(player->getName()))
         .arg(zone->getTranslatedName(true, CaseShuffleZone)));
    }
}

void MessageLogWidget::logRollDie(Player *player, int sides, int roll)
{
    QString coinOptions[2] = {tr("Heads")+" (1)", tr("Tails")+" (2)"};
    soundEngine->playSound("roll_dice");

    if (sides == 2)
        appendHtmlServerMessage(tr("%1 flipped a coin. It landed as %2.").arg(sanitizeHtml(player->getName())).arg("<font color=\"blue\">" + coinOptions[roll - 1] + "</font>"));
    else
        appendHtmlServerMessage(tr("%1 rolls a %2 with a %3-sided die.").arg(sanitizeHtml(player->getName())).arg("<font color=\"blue\">" + QString::number(roll) + "</font>").arg("<font color=\"blue\">" + QString::number(sides) + "</font>"));
}

void MessageLogWidget::logDrawCards(Player *player, int number)
{
    if (currentContext == MessageContext_Mulligan)
        mulliganPlayer = player;
    else {
        soundEngine->playSound("draw_card");
        appendHtmlServerMessage(tr("%1 draws %2 card(s).").arg(sanitizeHtml(player->getName())).arg("<font color=\"blue\">" + QString::number(number) + "</font>"));
    }
}

void MessageLogWidget::logUndoDraw(Player *player, QString cardName)
{
    if (cardName.isEmpty())
        appendHtmlServerMessage(tr("%1 undoes their last draw.").arg(sanitizeHtml(player->getName())));
    else
        appendHtmlServerMessage(tr("%1 undoes their last draw (%2).").arg(sanitizeHtml(player->getName())).arg(QString("<a href=\"card://%1\">%2</a>").arg(sanitizeHtml(cardName)).arg(sanitizeHtml(cardName))));
}

QPair<QString, QString> MessageLogWidget::getFromStr(CardZone *zone, QString cardName, int position, bool ownerChange) const
{
    bool cardNameContainsStartZone = false;
    QString fromStr;
    QString startName = zone->getName();

    if (startName == "table")
        fromStr = tr(" from play");
    else if (startName == "grave")
        fromStr = tr(" from their graveyard");
    else if (startName == "rfg")
        fromStr = tr(" from exile");
    else if (startName == "hand")
        fromStr = tr(" from their hand");
    else if (startName == "deck") {
        if (position == 0) {
            if (cardName.isEmpty()) {
                if (ownerChange)
                    cardName = tr("the top card of %1's library").arg(zone->getPlayer()->getName());
                else
                    cardName = tr("the top card of their library");
                cardNameContainsStartZone = true;
            } else {
                if (ownerChange)
                    fromStr = tr(" from the top of %1's library").arg(zone->getPlayer()->getName());
                else
                    fromStr = tr(" from the top of their library");
            }
        } else if (position >= zone->getCards().size() - 1) {
            if (cardName.isEmpty()) {
                if (ownerChange)
                    cardName = tr("the bottom card of %1's library").arg(zone->getPlayer()->getName());
                else
                    cardName = tr("the bottom card of their library");
                cardNameContainsStartZone = true;
            } else {
                if (ownerChange)
                    fromStr = tr(" from the bottom of %1's library").arg(zone->getPlayer()->getName());
                else
                    fromStr = tr(" from the bottom of their library");
            }
        } else {
            if (ownerChange)
                fromStr = tr(" from %1's library").arg(zone->getPlayer()->getName());
            else
                fromStr = tr(" from their library");
        }
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
    bool ownerChange = attributes.startZone->getPlayer() != attributes.targetZone->getPlayer();
    QString startName = attributes.startZone->getName();
    QString targetName = attributes.targetZone->getName();
    if (((startName == "table") && (targetName == "table") && (attributes.startZone == attributes.targetZone)) || ((startName == "hand") && (targetName == "hand")))
        return;
    QString cardName = attributes.cardName;
    QPair<QString, QString> temp = getFromStr(attributes.startZone, cardName, attributes.oldX, ownerChange);
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
        cardStr = cardLink(cardName);

    if (ownerChange && (attributes.startZone->getPlayer() == attributes.player)) {
        appendHtmlServerMessage(tr("%1 gives %2 control over %3.").arg(sanitizeHtml(attributes.player->getName())).arg(sanitizeHtml(attributes.targetZone->getPlayer()->getName())).arg(cardStr));
        return;
    }

    QString finalStr;
    if (targetName == "table") {
        soundEngine->playSound("play_card");
        if (moveCardTapped.value(attributes.card))
            finalStr = tr("%1 puts %2 into play tapped%3.");
        else
            finalStr = tr("%1 puts %2 into play%3.");
    } else if (targetName == "grave")
        finalStr = tr("%1 puts %2%3 into their graveyard.");
    else if (targetName == "rfg")
        finalStr = tr("%1 exiles %2%3.");
    else if (targetName == "hand")
        finalStr = tr("%1 moves %2%3 to their hand.");
    else if (targetName == "deck") {
        if (attributes.newX == -1)
            finalStr = tr("%1 puts %2%3 into their library.");
        else if (attributes.newX == attributes.targetZone->getCards().size() - 1)
            finalStr = tr("%1 puts %2%3 on bottom of their library.");
        else if (attributes.newX == 0)
            finalStr = tr("%1 puts %2%3 on top of their library.");
        else
            finalStr = tr("%1 puts %2%3 into their library at position %4.");
    } else if (targetName == "sb")
        finalStr = tr("%1 moves %2%3 to sideboard.");
    else if (targetName == "stack") {
        soundEngine->playSound("play_card");
        finalStr = tr("%1 plays %2%3.");
    }

    appendHtmlServerMessage(finalStr.arg(sanitizeHtml(attributes.player->getName())).arg(cardStr).arg(fromStr).arg(attributes.newX));
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

    if (number > -1) {
        appendHtmlServerMessage(tr("%1 takes a mulligan to %2.").arg(sanitizeHtml(player->getName())).arg(number));
    } else {
        appendHtmlServerMessage(tr("%1 draws their initial hand.").arg(sanitizeHtml(player->getName())));
    }
}

void MessageLogWidget::logFlipCard(Player *player, QString cardName, bool faceDown)
{
    if (faceDown) {
        appendHtmlServerMessage(tr("%1 flips %2 face-down.").arg(sanitizeHtml(player->getName())).arg(cardName));
    } else {
        appendHtmlServerMessage(tr("%1 flips %2 face-up.").arg(sanitizeHtml(player->getName())).arg(cardName));
    }
}

void MessageLogWidget::logDestroyCard(Player *player, QString cardName)
{
      appendHtmlServerMessage(tr("%1 destroys %2.").arg(sanitizeHtml(player->getName())).arg(cardLink(cardName)));
}

void MessageLogWidget::logAttachCard(Player *player, QString cardName, Player *targetPlayer, QString targetCardName)
{
    QString str;
    str = tr("%1 attaches %2 to %3's %4.");
    appendHtmlServerMessage(str.arg(sanitizeHtml(player->getName())).arg(cardLink(cardName)).arg(sanitizeHtml(targetPlayer->getName())).arg(cardLink(targetCardName)));
}

void MessageLogWidget::logUnattachCard(Player *player, QString cardName)
{
    appendHtmlServerMessage(tr("%1 unattaches %2.").arg(sanitizeHtml(player->getName())).arg(cardLink(cardName)));
}

void MessageLogWidget::logCreateToken(Player *player, QString cardName, QString pt)
{
    appendHtmlServerMessage(tr("%1 creates token: %2%3.").arg(sanitizeHtml(player->getName())).arg(cardLink(cardName)).arg(pt.isEmpty() ? QString() : QString(" (%1)").arg(sanitizeHtml(pt))));
}

void MessageLogWidget::logCreateArrow(Player *player, Player *startPlayer, QString startCard, Player *targetPlayer, QString targetCard, bool playerTarget)
{
    startCard = cardLink(startCard);
    targetCard = cardLink(targetCard);
    QString str;
    if (playerTarget) {
        if ((player == startPlayer) && (player == targetPlayer)) {
                str = tr("%1 points from their %2 to themselves.");
            appendHtmlServerMessage(str.arg(sanitizeHtml(player->getName())).arg(startCard));
        } else if (player == startPlayer) {
                    str = tr("%1 points from their %2 to %3.");
            appendHtmlServerMessage(str.arg(sanitizeHtml(player->getName())).arg(startCard).arg(sanitizeHtml(targetPlayer->getName())));
        } else if (player == targetPlayer) {
                    str = tr("%1 points from %2's %3 to themselves.");
            appendHtmlServerMessage(str.arg(sanitizeHtml(player->getName())).arg(sanitizeHtml(startPlayer->getName())).arg(startCard));
        } else {
                    str = tr("%1 points from %2's %3 to %4.");
            appendHtmlServerMessage(str.arg(sanitizeHtml(player->getName())).arg(sanitizeHtml(startPlayer->getName())).arg(startCard).arg(sanitizeHtml(targetPlayer->getName())));
        }
    } else {
        if ((player == startPlayer) && (player == targetPlayer)) {
            str = tr("%1 points from their %2 to their %3.");
            appendHtmlServerMessage(str.arg(sanitizeHtml(player->getName())).arg(startCard).arg(targetCard));
        } else if (player == startPlayer) {
            str = tr("%1 points from their %2 to %3's %4.");
            appendHtmlServerMessage(str.arg(sanitizeHtml(player->getName())).arg(startCard).arg(sanitizeHtml(targetPlayer->getName())).arg(targetCard));
        } else if (player == targetPlayer) {
            str = tr("%1 points from %2's %3 to their own %4.");
            appendHtmlServerMessage(str.arg(sanitizeHtml(player->getName())).arg(sanitizeHtml(startPlayer->getName())).arg(startCard).arg(targetCard));
        } else {
            str = tr("%1 points from %2's %3 to %4's %5.");
            appendHtmlServerMessage(str.arg(sanitizeHtml(player->getName())).arg(sanitizeHtml(startPlayer->getName())).arg(startCard).arg(sanitizeHtml(targetPlayer->getName())).arg(targetCard));
        }
    }
}

void MessageLogWidget::logSetCardCounter(Player *player, QString cardName, int counterId, int value, int oldValue)
{
    QString finalStr, colorStr;

    int delta = abs(oldValue - value);
    if (value > oldValue) {
            finalStr = tr("%1 places %2 %3 counter(s) on %4 (now %5).");
    } else {
            finalStr = tr("%1 removes %2 %3 counter(s) from %4 (now %5).");
    }

    switch (counterId) {
        case 0: colorStr = tr("red", "", delta); break;
        case 1: colorStr = tr("yellow", "", delta); break;
        case 2: colorStr = tr("green", "", delta); break;
        default: ;
    }

    appendHtmlServerMessage(finalStr.arg(sanitizeHtml(player->getName())).arg("<font color=\"blue\">" + QString::number(delta) + "</font>").arg(colorStr).arg(cardLink(cardName)).arg(value));
}

void MessageLogWidget::logSetTapped(Player *player, CardItem *card, bool tapped)
{
    if (tapped)
        soundEngine->playSound("tap_card");
    else
        soundEngine->playSound("untap_card");

    if (currentContext == MessageContext_MoveCard)
        moveCardTapped.insert(card, tapped);
    else {
        QString str;
        if (!card) {
                if (tapped)
                    str = tr("%1 taps their permanents.");
                else
                    str = tr("%1 untaps their permanents.");
            appendHtmlServerMessage(str.arg(sanitizeHtml(player->getName())));
        } else {
                if (tapped)
                    str = tr("%1 taps %2.");
                else
                    str = tr("%1 untaps %2.");
            appendHtmlServerMessage(str.arg(sanitizeHtml(player->getName())).arg(cardLink(card->getName())));
        }
    }
}

void MessageLogWidget::logSetCounter(Player *player, QString counterName, int value, int oldValue)
{
    if (counterName == "life")
        soundEngine->playSound("life_change");

    QString str;
        str = tr("%1 sets counter %2 to %3 (%4%5).");
    appendHtmlServerMessage(str.arg(sanitizeHtml(player->getName())).arg(QString("<font color=\"blue\">%1</font>").arg(sanitizeHtml(counterName))).arg(QString("<font color=\"blue\">%1</font>").arg(value)).arg(value > oldValue ? "+" : "").arg(value - oldValue));
}

void MessageLogWidget::logSetDoesntUntap(Player *player, CardItem *card, bool doesntUntap)
{
    QString str;
    if (doesntUntap) {
            str = tr("%1 sets %2 to not untap normally.");
    } else {
            str = tr("%1 sets %2 to untap normally.");
    }
    appendHtmlServerMessage(str.arg(sanitizeHtml(player->getName())).arg(cardLink(card->getName())));
}

void MessageLogWidget::logSetPT(Player *player, CardItem *card, QString newPT)
{
    if (currentContext == MessageContext_MoveCard)
        moveCardPT.insert(card, newPT);
    else {
        QString str;
            str = tr("%1 sets PT of %2 to %3.");
        appendHtmlServerMessage(str.arg(sanitizeHtml(player->getName())).arg(cardLink(card->getName())).arg(QString("<font color=\"blue\">%1</font>").arg(sanitizeHtml(newPT))));
    }
}

void MessageLogWidget::logSetAnnotation(Player *player, CardItem *card, QString newAnnotation)
{
    QString str;
        str = tr("%1 sets annotation of %2 to %3.");
    appendHtmlServerMessage(str.arg(sanitizeHtml(player->getName())).arg(cardLink(card->getName())).arg(QString("&quot;<font color=\"blue\">%1</font>&quot;").arg(sanitizeHtml(newAnnotation))));
}

void MessageLogWidget::logDumpZone(Player *player, CardZone *zone, int numberCards)
{
    if (numberCards == -1)
        appendHtmlServerMessage((
            tr("%1 is looking at %2.")
        ).arg(sanitizeHtml(player->getName()))
         .arg(zone->getTranslatedName(zone->getPlayer() == player, CaseLookAtZone)));
    else
        appendHtmlServerMessage((
          tr("%1 is looking at the top %2 card(s) %3.")
        ).arg(sanitizeHtml(player->getName())).arg("<font color=\"blue\">" + QString::number(numberCards) + "</font>")
         .arg(zone->getTranslatedName(zone->getPlayer() == player, CaseTopCardsOfZone)));
}

void MessageLogWidget::logStopDumpZone(Player *player, CardZone *zone)
{
    appendHtmlServerMessage(
     tr("%1 stops looking at %2.")
     .arg(sanitizeHtml(player->getName()))
     .arg(zone->getTranslatedName(zone->getPlayer() == player, CaseLookAtZone)));
}

void MessageLogWidget::logRevealCards(Player *player, CardZone *zone, int cardId, QString cardName, Player *otherPlayer, bool faceDown)
{
    QPair<QString, QString> temp = getFromStr(zone, cardName, cardId, false);
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
        cardStr = cardLink(cardName);

    QString str;
    if (cardId == -1) {
        if (otherPlayer) {
                    str = tr("%1 reveals %2 to %3.");
            appendHtmlServerMessage(str.arg(sanitizeHtml(player->getName())).arg(zone->getTranslatedName(true, CaseRevealZone)).arg(sanitizeHtml(otherPlayer->getName())));
        } else {
            appendHtmlServerMessage((
              tr("%1 reveals %2.")
            ).arg(sanitizeHtml(player->getName()))
             .arg(zone->getTranslatedName(true, CaseRevealZone)));
        }
    } else if (cardId == -2) {
        if (otherPlayer) {

                    str = tr("%1 randomly reveals %2%3 to %4.");
            appendHtmlServerMessage(str.arg(sanitizeHtml(player->getName())).arg(cardStr).arg(fromStr).arg(sanitizeHtml(otherPlayer->getName())));
        } else {
                appendHtmlServerMessage(tr("%1 randomly reveals %2%3.").arg(sanitizeHtml(player->getName())).arg(cardStr).arg(fromStr));
        }
    } else {
        if (faceDown && (player == otherPlayer)) {
            if (cardName.isEmpty()) {
                    str = tr("%1 peeks at face down card #%2.");
                appendHtmlServerMessage(str.arg(sanitizeHtml(player->getName())).arg(cardId));
            } else {
                    str = tr("%1 peeks at face down card #%2: %3.");
                appendHtmlServerMessage(str.arg(sanitizeHtml(player->getName())).arg(cardId).arg(cardStr));
            }
        } else if (otherPlayer) {

                    str = tr("%1 reveals %2%3 to %4.");
            appendHtmlServerMessage(str.arg(sanitizeHtml(player->getName())).arg(cardStr).arg(fromStr).arg(sanitizeHtml(otherPlayer->getName())));
        } else {
                appendHtmlServerMessage(tr("%1 reveals %2%3.").arg(sanitizeHtml(player->getName())).arg(cardStr).arg(fromStr));
        }
    }
}

void MessageLogWidget::logAlwaysRevealTopCard(Player *player, CardZone *zone, bool reveal)
{
    appendHtmlServerMessage((reveal
        ? tr("%1 is now keeping the top card %2 revealed.")
        : tr("%1 is not revealing the top card %2 any longer.")
        ).arg(sanitizeHtml(player->getName()))
         .arg(zone->getTranslatedName(true, CaseTopCardsOfZone))
    );
}

void MessageLogWidget::logSetActivePlayer(Player *player)
{

    QString str;
        str = tr("It is now %1's turn.");
    appendHtml("<br><font color=\"green\"><b>" + QDateTime::currentDateTime().toString("[hh:mm:ss] ") + str.arg(player->getName()) + "</b></font><br>");
}

void MessageLogWidget::logSetActivePhase(int phase)
{
    QString phaseName;
    switch (phase) {
        case 0: phaseName = tr("untap step"); soundEngine->playSound("untap_step"); break;
        case 1: phaseName = tr("upkeep step"); soundEngine->playSound("upkeep_step"); break;
        case 2: phaseName = tr("draw step"); soundEngine->playSound("draw_step"); break;
        case 3: phaseName = tr("first main phase"); soundEngine->playSound("main_1"); break;
        case 4: phaseName = tr("beginning of combat step"); soundEngine->playSound("start_combat"); break;
        case 5: phaseName = tr("declare attackers step"); soundEngine->playSound("attack_step"); break;
        case 6: phaseName = tr("declare blockers step"); soundEngine->playSound("block_step"); break;
        case 7: phaseName = tr("combat damage step"); soundEngine->playSound("damage_step"); break;
        case 8: phaseName = tr("end of combat step"); soundEngine->playSound("end_combat"); break;
        case 9: phaseName = tr("second main phase"); soundEngine->playSound("main_2"); break;
        case 10: phaseName = tr("ending phase"); soundEngine->playSound("end_step"); break;
    }
    appendHtml("<font color=\"green\"><b>" + QDateTime::currentDateTime().toString("[hh:mm:ss] ") + tr("It is now the %1.").arg(phaseName) + "</b></font>");
}

void MessageLogWidget::containerProcessingStarted(const GameEventContext &_context)
{
    if (_context.HasExtension(Context_MoveCard::ext))
        currentContext = MessageContext_MoveCard;
    else if (_context.HasExtension(Context_Mulligan::ext)) {
        const Context_Mulligan &contextMulligan = _context.GetExtension(Context_Mulligan::ext);
        currentContext = MessageContext_Mulligan;
        mulliganPlayer = 0;
        mulliganNumber = contextMulligan.number();
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
    connect(player, SIGNAL(logRevealCards(Player *, CardZone *, int, QString, Player *, bool)), this, SLOT(logRevealCards(Player *, CardZone *, int, QString, Player *, bool)));
    connect(player, SIGNAL(logAlwaysRevealTopCard(Player *, CardZone *, bool)), this, SLOT(logAlwaysRevealTopCard(Player *, CardZone *, bool)));
}

MessageLogWidget::MessageLogWidget(const TabSupervisor *_tabSupervisor, TabGame *_game, QWidget *parent)
    : ChatView(_tabSupervisor, _game, true, parent), currentContext(MessageContext_None)
{
}
