#include "messagelogwidget.h"
#include "player.h"
#include "cardzone.h"
#include "carditem.h"
#include "soundengine.h"
#include "tab_supervisor.h"
#include "pb/serverinfo_user.pb.h"
#include "pb/context_move_card.pb.h"
#include "pb/context_mulligan.pb.h"
#include <QScrollBar>

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
    return QString("<a href=\"card://%1\">%2</a>").arg(cardName).arg(cardName);
}

bool MessageLogWidget::isFemale(Player *player) const
{
    return player->getUserInfo()->gender() == ServerInfo_User::Female;
}

bool MessageLogWidget::userIsFemale() const
{
    return (tabSupervisor && tabSupervisor->getUserInfo() && (tabSupervisor->getUserInfo()->gender() & ServerInfo_User::Female));
}

void MessageLogWidget::logGameJoined(int gameId)
{
    if (userIsFemale())
        appendHtml(tr("You have joined game #%1.", "female").arg(gameId));
    else
        appendHtml(tr("You have joined game #%1.", "male").arg(gameId));
}

void MessageLogWidget::logReplayStarted(int gameId)
{
    if (userIsFemale())
        appendHtml(tr("You are watching a replay of game #%1.", "female").arg(gameId));
    else
        appendHtml(tr("You are watching a replay of game #%1.", "male").arg(gameId));
}

void MessageLogWidget::logJoin(Player *player)
{
    soundEngine->cuckoo();
    if (isFemale(player))
        appendHtml(tr("%1 has joined the game.", "female").arg(sanitizeHtml(player->getName())));
    else
        appendHtml(tr("%1 has joined the game.", "male").arg(sanitizeHtml(player->getName())));
}

void MessageLogWidget::logLeave(Player *player)
{
    if (isFemale(player))
        appendHtml(tr("%1 has left the game.", "female").arg(sanitizeHtml(player->getName())));
    else
        appendHtml(tr("%1 has left the game.", "male").arg(sanitizeHtml(player->getName())));
}

void MessageLogWidget::logGameClosed()
{
    appendHtml(tr("The game has been closed."));
}

void MessageLogWidget::logKicked()
{
    appendHtml(tr("You have been kicked out of the game."));
}

void MessageLogWidget::logJoinSpectator(QString name)
{
    appendHtml(tr("%1 is now watching the game.").arg(sanitizeHtml(name)));
}

void MessageLogWidget::logLeaveSpectator(QString name)
{
    appendHtml(tr("%1 is not watching the game any more.").arg(sanitizeHtml(name)));
}

void MessageLogWidget::logDeckSelect(Player *player, QString deckHash)
{
    if (isFemale(player))
        appendHtml(tr("%1 has loaded a deck (%2).", "female").arg(sanitizeHtml(player->getName())).arg(deckHash));
    else
        appendHtml(tr("%1 has loaded a deck (%2).", "male").arg(sanitizeHtml(player->getName())).arg(deckHash));
}

void MessageLogWidget::logReadyStart(Player *player)
{
    if (isFemale(player))
        appendHtml(tr("%1 is ready to start the game.", "female").arg(sanitizeHtml(player->getName())));
    else
        appendHtml(tr("%1 is ready to start the game.", "male").arg(sanitizeHtml(player->getName())));
}

void MessageLogWidget::logNotReadyStart(Player *player)
{
    if (isFemale(player))
        appendHtml(tr("%1 is not ready to start the game any more.", "female").arg(sanitizeHtml(player->getName())));
    else
        appendHtml(tr("%1 is not ready to start the game any more.", "male").arg(sanitizeHtml(player->getName())));
}

void MessageLogWidget::logSetSideboardLock(Player *player, bool locked)
{
    if (locked) {
        if (isFemale(player))
            appendHtml(tr("%1 has locked her sideboard.", "female").arg(sanitizeHtml(player->getName())));
        else
            appendHtml(tr("%1 has locked his sideboard.", "male").arg(sanitizeHtml(player->getName())));
    } else {
        if (isFemale(player))
            appendHtml(tr("%1 has unlocked her sideboard.", "female").arg(sanitizeHtml(player->getName())));
        else
            appendHtml(tr("%1 has unlocked his sideboard.", "male").arg(sanitizeHtml(player->getName())));
    }
}

void MessageLogWidget::logConcede(Player *player)
{
    if (isFemale(player))
        appendHtml(tr("%1 has conceded the game.", "female").arg(sanitizeHtml(player->getName())));
    else
        appendHtml(tr("%1 has conceded the game.", "male").arg(sanitizeHtml(player->getName())));
}

void MessageLogWidget::logGameStart()
{
    appendHtml(tr("The game has started."));
}

void MessageLogWidget::logConnectionStateChanged(Player *player, bool connectionState)
{
    if (connectionState) {
        if (isFemale(player))
            appendHtml(tr("%1 has restored connection to the game.", "female").arg(sanitizeHtml(player->getName())));
        else
            appendHtml(tr("%1 has restored connection to the game.", "male").arg(sanitizeHtml(player->getName())));
    } else {
        if (isFemale(player))
            appendHtml(tr("%1 has lost connection to the game.", "female").arg(sanitizeHtml(player->getName())));
        else
            appendHtml(tr("%1 has lost connection to the game.", "male").arg(sanitizeHtml(player->getName())));
    }
}

void MessageLogWidget::logSay(Player *player, QString message)
{
    appendMessage(message, player->getName(), UserLevelFlags(player->getUserInfo()->user_level()), true);
}

void MessageLogWidget::logSpectatorSay(QString spectatorName, UserLevelFlags spectatorUserLevel, QString message)
{
    appendMessage(message, spectatorName, spectatorUserLevel, false);
}

void MessageLogWidget::logShuffle(Player *player, CardZone *zone)
{
    soundEngine->shuffle();
    if (currentContext != MessageContext_Mulligan) {
        appendHtml((isFemale(player)
            ? tr("%1 shuffles %2.", "female")
            : tr("%1 shuffles %2.", "male")
        ).arg(sanitizeHtml(player->getName()))
         .arg(zone->getTranslatedName(true, CaseShuffleZone)));
    }
}

void MessageLogWidget::logRollDie(Player *player, int sides, int roll)
{
    if (isFemale(player))
        appendHtml(tr("%1 rolls a %2 with a %3-sided die.", "female").arg(sanitizeHtml(player->getName())).arg(roll).arg(sides));
    else
        appendHtml(tr("%1 rolls a %2 with a %3-sided die.", "male").arg(sanitizeHtml(player->getName())).arg(roll).arg(sides));
}

void MessageLogWidget::logDrawCards(Player *player, int number)
{
    if (currentContext == MessageContext_Mulligan)
        mulliganPlayer = player;
    else {
        soundEngine->draw();
        if (isFemale(player))
            appendHtml(tr("%1 draws %n card(s).", "female", number).arg(sanitizeHtml(player->getName())));
        else
            appendHtml(tr("%1 draws %n card(s).", "male", number).arg(sanitizeHtml(player->getName())));
    }
}

void MessageLogWidget::logUndoDraw(Player *player, QString cardName)
{
    if (cardName.isEmpty())
        appendHtml((isFemale(player) ? tr("%1 undoes her last draw.") : tr("%1 undoes his last draw.")).arg(sanitizeHtml(player->getName())));
    else
        appendHtml((isFemale(player) ? tr("%1 undoes her last draw (%2).") : tr("%1 undoes his last draw (%2).")).arg(sanitizeHtml(player->getName())).arg(QString("<a href=\"card://%1\">%2</a>").arg(sanitizeHtml(cardName)).arg(sanitizeHtml(cardName))));
}

QPair<QString, QString> MessageLogWidget::getFromStr(CardZone *zone, QString cardName, int position, bool ownerChange) const
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
        if (position >= zone->getCards().size() - 1) {
            if (cardName.isEmpty()) {
                if (ownerChange)
                    cardName = tr("the bottom card of %1's library").arg(zone->getPlayer()->getName());
                else
                    cardName = isFemale(zone->getPlayer()) ? tr("the bottom card of her library") : tr("the bottom card of his library");
                cardNameContainsStartZone = true;
            } else {
                if (ownerChange)
                    fromStr = tr(" from the bottom of %1's library").arg(zone->getPlayer()->getName());
                else
                    fromStr = isFemale(zone->getPlayer()) ? tr(" from the bottom of her library") : tr(" from the bottom of his library");
            }
        } else if (position == 0) {
            if (cardName.isEmpty()) {
                if (ownerChange)
                    cardName = tr("the top card of %1's library").arg(zone->getPlayer()->getName());
                else
                    cardName = isFemale(zone->getPlayer()) ? tr("the top card of her library") : tr("the top card of his library");
                cardNameContainsStartZone = true;
            } else {
                if (ownerChange)
                    fromStr = tr(" from the top of %1's library").arg(zone->getPlayer()->getName());
                else
                    fromStr = isFemale(zone->getPlayer()) ? tr(" from the top of her library") : tr(" from the top of his library");
            }
        } else {
            if (ownerChange)
                fromStr = tr(" from %1's library").arg(zone->getPlayer()->getName());
            else
                fromStr = tr(" from library");
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
        appendHtml(tr("%1 gives %2 control over %3.").arg(sanitizeHtml(attributes.player->getName())).arg(sanitizeHtml(attributes.targetZone->getPlayer()->getName())).arg(cardStr));
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
    
    appendHtml(finalStr.arg(sanitizeHtml(attributes.player->getName())).arg(cardStr).arg(fromStr).arg(attributes.newX));
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
        if (isFemale(player))
            appendHtml(tr("%1 takes a mulligan to %n.", "female", number).arg(sanitizeHtml(player->getName())));
        else
            appendHtml(tr("%1 takes a mulligan to %n.", "male", number).arg(sanitizeHtml(player->getName())));
    } else
        appendHtml((isFemale(player) ? tr("%1 draws her initial hand.") : tr("%1 draws his initial hand.")).arg(sanitizeHtml(player->getName())));
}

void MessageLogWidget::logFlipCard(Player *player, QString cardName, bool faceDown)
{
    if (faceDown) {
        if (isFemale(player))
            appendHtml(tr("%1 flips %2 face-down.", "female").arg(sanitizeHtml(player->getName())).arg(cardName));
        else
            appendHtml(tr("%1 flips %2 face-down.", "male").arg(sanitizeHtml(player->getName())).arg(cardName));
    } else {
        if (isFemale(player))
            appendHtml(tr("%1 flips %2 face-up.", "female").arg(sanitizeHtml(player->getName())).arg(cardName));
        else
            appendHtml(tr("%1 flips %2 face-up.", "male").arg(sanitizeHtml(player->getName())).arg(cardName));
    }
}

void MessageLogWidget::logDestroyCard(Player *player, QString cardName)
{
    if (isFemale(player))
        appendHtml(tr("%1 destroys %2.", "female").arg(sanitizeHtml(player->getName())).arg(cardLink(cardName)));
    else
        appendHtml(tr("%1 destroys %2.", "male").arg(sanitizeHtml(player->getName())).arg(cardLink(cardName)));
}

void MessageLogWidget::logAttachCard(Player *player, QString cardName, Player *targetPlayer, QString targetCardName)
{
    QString str;
    if (isFemale(player)) {
        if (isFemale(targetPlayer))
            str = tr("%1 attaches %2 to %3's %4.", "p1 female, p2 female");
        else
            str = tr("%1 attaches %2 to %3's %4.", "p1 female, p2 male");
    } else {
        if (isFemale(targetPlayer))
            str = tr("%1 attaches %2 to %3's %4.", "p1 male, p2 female");
        else
            str = tr("%1 attaches %2 to %3's %4.", "p1 male, p2 male");
    }
    
    appendHtml(str.arg(sanitizeHtml(player->getName())).arg(cardLink(cardName)).arg(sanitizeHtml(targetPlayer->getName())).arg(cardLink(targetCardName)));
}

void MessageLogWidget::logUnattachCard(Player *player, QString cardName)
{
    if (isFemale(player))
        appendHtml(tr("%1 unattaches %2.", "female").arg(sanitizeHtml(player->getName())).arg(cardLink(cardName)));
    else
        appendHtml(tr("%1 unattaches %2.", "male").arg(sanitizeHtml(player->getName())).arg(cardLink(cardName)));
}

void MessageLogWidget::logCreateToken(Player *player, QString cardName, QString pt)
{
    if (isFemale(player))
        appendHtml(tr("%1 creates token: %2%3.", "female").arg(sanitizeHtml(player->getName())).arg(cardLink(cardName)).arg(pt.isEmpty() ? QString() : QString(" (%1)").arg(sanitizeHtml(pt))));
    else
        appendHtml(tr("%1 creates token: %2%3.", "male").arg(sanitizeHtml(player->getName())).arg(cardLink(cardName)).arg(pt.isEmpty() ? QString() : QString(" (%1)").arg(sanitizeHtml(pt))));
}

void MessageLogWidget::logCreateArrow(Player *player, Player *startPlayer, QString startCard, Player *targetPlayer, QString targetCard, bool playerTarget)
{
    startCard = cardLink(startCard);
    targetCard = cardLink(targetCard);
    QString str;
    if (playerTarget) {
        if ((player == startPlayer) && (player == targetPlayer)) {
            if (isFemale(player))
                str = tr("%1 points from her %2 to herself.", "female");
            else
                str = tr("%1 points from his %2 to himself.", "male");
            appendHtml(str.arg(sanitizeHtml(player->getName())).arg(startCard));
        } else if (player == startPlayer) {
            if (isFemale(player)) {
                if (isFemale(targetPlayer))
                    str = tr("%1 points from her %2 to %3.", "p1 female, p2 female");
                else
                    str = tr("%1 points from her %2 to %3.", "p1 female, p2 male");
            } else {
                if (isFemale(targetPlayer))
                    str = tr("%1 points from his %2 to %3.", "p1 male, p2 female");
                else
                    str = tr("%1 points from his %2 to %3.", "p1 male, p2 male");
            }
            appendHtml(str.arg(sanitizeHtml(player->getName())).arg(startCard).arg(sanitizeHtml(targetPlayer->getName())));
        } else if (player == targetPlayer) {
            if (isFemale(player)) {
                if (isFemale(startPlayer))
                    str = tr("%1 points from %2's %3 to herself.", "card owner female, target female");
                else
                    str = tr("%1 points from %2's %3 to herself.", "card owner male, target female");
            } else {
                if (isFemale(startPlayer))
                    str = tr("%1 points from %2's %3 to himself.", "card owner female, target male");
                else
                    str = tr("%1 points from %2's %3 to himself.", "card owner male, target male");
            }
            appendHtml(str.arg(sanitizeHtml(player->getName())).arg(sanitizeHtml(startPlayer->getName())).arg(startCard));
        } else {
            if (isFemale(player)) {
                if (isFemale(startPlayer)) {
                    if (isFemale(targetPlayer))
                        str = tr("%1 points from %2's %3 to %4.", "p1 female, p2 female, p3 female");
                    else
                        str = tr("%1 points from %2's %3 to %4.", "p1 female, p2 female, p3 male");
                } else {
                    if (isFemale(targetPlayer))
                        str = tr("%1 points from %2's %3 to %4.", "p1 female, p2 male, p3 female");
                    else
                        str = tr("%1 points from %2's %3 to %4.", "p1 female, p2 male, p3 male");
                }
            } else {
                if (isFemale(startPlayer)) {
                    if (isFemale(targetPlayer))
                        str = tr("%1 points from %2's %3 to %4.", "p1 male, p2 female, p3 female");
                    else
                        str = tr("%1 points from %2's %3 to %4.", "p1 male, p2 female, p3 male");
                } else {
                    if (isFemale(targetPlayer))
                        str = tr("%1 points from %2's %3 to %4.", "p1 male, p2 male, p3 female");
                    else
                        str = tr("%1 points from %2's %3 to %4.", "p1 male, p2 male, p3 male");
                }
            }
            appendHtml(str.arg(sanitizeHtml(player->getName())).arg(sanitizeHtml(startPlayer->getName())).arg(startCard).arg(sanitizeHtml(targetPlayer->getName())));
        }
    } else {
        if ((player == startPlayer) && (player == targetPlayer)) {
            if (isFemale(player))
                str = tr("%1 points from her %2 to her %3.", "female");
            else
                str = tr("%1 points from his %2 to his %3.", "male");
            appendHtml(str.arg(sanitizeHtml(player->getName())).arg(startCard).arg(targetCard));
        } else if (player == startPlayer) {
            if (isFemale(player)) {
                if (isFemale(targetPlayer))
                    str = tr("%1 points from her %2 to %3's %4.", "p1 female, p2 female");
                else
                    str = tr("%1 points from her %2 to %3's %4.", "p1 female, p2 male");
            } else {
                if (isFemale(targetPlayer))
                    str = tr("%1 points from his %2 to %3's %4.", "p1 male, p2 female");
                else
                    str = tr("%1 points from his %2 to %3's %4.", "p1 male, p2 male");
            }
            appendHtml(str.arg(sanitizeHtml(player->getName())).arg(startCard).arg(sanitizeHtml(targetPlayer->getName())).arg(targetCard));
        } else if (player == targetPlayer) {
            if (isFemale(player)) {
                if (isFemale(startPlayer))
                    str = tr("%1 points from %2's %3 to her own %4.", "card owner female, target female");
                else
                    str = tr("%1 points from %2's %3 to her own %4.", "card owner male, target female");
            } else {
                if (isFemale(startPlayer))
                    str = tr("%1 points from %2's %3 to his own %4.", "card owner female, target male");
                else
                    str = tr("%1 points from %2's %3 to his own %4.", "card owner male, target male");
            }
            appendHtml(str.arg(sanitizeHtml(player->getName())).arg(sanitizeHtml(startPlayer->getName())).arg(startCard).arg(targetCard));
        } else {
            if (isFemale(player)) {
                if (isFemale(startPlayer)) {
                    if (isFemale(targetPlayer))
                        str = tr("%1 points from %2's %3 to %4's %5.", "p1 female, p2 female, p3 female");
                    else
                        str = tr("%1 points from %2's %3 to %4's %5.", "p1 female, p2 female, p3 male");
                } else {
                    if (isFemale(targetPlayer))
                        str = tr("%1 points from %2's %3 to %4's %5.", "p1 female, p2 male, p3 female");
                    else
                        str = tr("%1 points from %2's %3 to %4's %5.", "p1 female, p2 male, p3 male");
                }
            } else {
                if (isFemale(startPlayer)) {
                    if (isFemale(targetPlayer))
                        str = tr("%1 points from %2's %3 to %4's %5.", "p1 male, p2 female, p3 female");
                    else
                        str = tr("%1 points from %2's %3 to %4's %5.", "p1 male, p2 female, p3 male");
                } else {
                    if (isFemale(targetPlayer))
                        str = tr("%1 points from %2's %3 to %4's %5.", "p1 male, p2 male, p3 female");
                    else
                        str = tr("%1 points from %2's %3 to %4's %5.", "p1 male, p2 male, p3 male");
                }
            }
            appendHtml(str.arg(sanitizeHtml(player->getName())).arg(sanitizeHtml(startPlayer->getName())).arg(startCard).arg(sanitizeHtml(targetPlayer->getName())).arg(targetCard));
        }
    }
}

void MessageLogWidget::logSetCardCounter(Player *player, QString cardName, int counterId, int value, int oldValue)
{
    QString finalStr, colorStr;
    
    int delta = abs(oldValue - value);
    if (value > oldValue) {
        if (isFemale(player))
            finalStr = tr("%1 places %n %2 counter(s) on %3 (now %4).", "female", delta);
        else
            finalStr = tr("%1 places %n %2 counter(s) on %3 (now %4).", "male", delta);
    } else {
        if (isFemale(player))
            finalStr = tr("%1 removes %n %2 counter(s) from %3 (now %4).", "female", delta);
        else
            finalStr = tr("%1 removes %n %2 counter(s) from %3 (now %4).", "male", delta);
    }
    
    switch (counterId) {
        case 0: colorStr = tr("red", "", delta); break;
        case 1: colorStr = tr("yellow", "", delta); break;
        case 2: colorStr = tr("green", "", delta); break;
        default: ;
    }
    
    appendHtml(finalStr.arg(sanitizeHtml(player->getName())).arg(colorStr).arg(cardLink(cardName)).arg(value));
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
        QString str;
        if (!card) {
            if (isFemale(player)) {
                if (tapped)
                    str = tr("%1 taps her permanents.", "female");
                else
                    str = tr("%1 untaps her permanents.", "female");
            } else {
                if (tapped)
                    str = tr("%1 taps his permanents.", "male");
                else
                    str = tr("%1 untaps his permanents.", "male");
            }
            appendHtml(str.arg(sanitizeHtml(player->getName())));
        } else {
            if (isFemale(player)) {
                if (tapped)
                    str = tr("%1 taps %2.", "female");
                else
                    str = tr("%1 untaps %2.", "female");
            } else {
                if (tapped)
                    str = tr("%1 taps %2.", "male");
                else
                    str = tr("%1 untaps %2.", "male");
            }
            appendHtml(str.arg(sanitizeHtml(player->getName())).arg(cardLink(card->getName())));
        }
    }
}

void MessageLogWidget::logSetCounter(Player *player, QString counterName, int value, int oldValue)
{
    QString str;
    if (isFemale(player))
        str = tr("%1 sets counter %2 to %3 (%4%5).", "female");
    else
        str = tr("%1 sets counter %2 to %3 (%4%5).", "male");
    appendHtml(str.arg(sanitizeHtml(player->getName())).arg(QString("<font color=\"blue\">%1</font>").arg(sanitizeHtml(counterName))).arg(QString("<font color=\"blue\">%1</font>").arg(value)).arg(value > oldValue ? "+" : "").arg(value - oldValue));
}

void MessageLogWidget::logSetDoesntUntap(Player *player, CardItem *card, bool doesntUntap)
{
    QString str;
    if (doesntUntap) {
        if (isFemale(player))
            str = tr("%1 sets %2 to not untap normally.", "female");
        else
            str = tr("%1 sets %2 to not untap normally.", "male");
    } else {
        if (isFemale(player))
            str = tr("%1 sets %2 to untap normally.", "female");
        else
            str = tr("%1 sets %2 to untap normally.", "male");
    }
    appendHtml(str.arg(sanitizeHtml(player->getName())).arg(cardLink(card->getName())));
}

void MessageLogWidget::logSetPT(Player *player, CardItem *card, QString newPT)
{
    if (currentContext == MessageContext_MoveCard)
        moveCardPT.insert(card, newPT);
    else {
        QString str;
        if (isFemale(player))
            str = tr("%1 sets PT of %2 to %3.", "female");
        else
            str = tr("%1 sets PT of %2 to %3.", "male");
        appendHtml(str.arg(sanitizeHtml(player->getName())).arg(cardLink(card->getName())).arg(QString("<font color=\"blue\">%1</font>").arg(sanitizeHtml(newPT))));
    }
}

void MessageLogWidget::logSetAnnotation(Player *player, CardItem *card, QString newAnnotation)
{
    QString str;
    if (isFemale(player))
        str = tr("%1 sets annotation of %2 to %3.", "female");
    else
        str = tr("%1 sets annotation of %2 to %3.", "male");
    appendHtml(str.arg(sanitizeHtml(player->getName())).arg(cardLink(card->getName())).arg(QString("&quot;<font color=\"blue\">%1</font>&quot;").arg(sanitizeHtml(newAnnotation))));
}

void MessageLogWidget::logDumpZone(Player *player, CardZone *zone, int numberCards)
{
    if (numberCards == -1)
        appendHtml((isFemale(player)
            ? tr("%1 is looking at %2.", "female")
            : tr("%1 is looking at %2.", "male")
        ).arg(sanitizeHtml(player->getName()))
         .arg(zone->getTranslatedName(zone->getPlayer() == player, CaseLookAtZone)));
    else
        appendHtml((isFemale(player)
            ? tr("%1 is looking at the top %n card(s) %2.", "female", numberCards)
            : tr("%1 is looking at the top %n card(s) %2.", "male", numberCards)
        ).arg(sanitizeHtml(player->getName()))
         .arg(zone->getTranslatedName(zone->getPlayer() == player, CaseTopCardsOfZone)));
}

void MessageLogWidget::logStopDumpZone(Player *player, CardZone *zone)
{
    appendHtml((isFemale(player)
        ? tr("%1 stops looking at %2.", "female")
        : tr("%1 stops looking at %2.", "male")
    ).arg(sanitizeHtml(player->getName()))
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
            if (isFemale(player)) {
                if (isFemale(otherPlayer))
                    str = tr("%1 reveals %2 to %3.", "p1 female, p2 female");
                else
                    str = tr("%1 reveals %2 to %3.", "p1 female, p2 male");
            } else {
                if (isFemale(otherPlayer))
                    str = tr("%1 reveals %2 to %3.", "p1 male, p2 female");
                else
                    str = tr("%1 reveals %2 to %3.", "p1 male, p2 male");
            }
            appendHtml(str.arg(sanitizeHtml(player->getName())).arg(zone->getTranslatedName(true, CaseRevealZone)).arg(sanitizeHtml(otherPlayer->getName())));
        } else {
            appendHtml((isFemale(player)
                ? tr("%1 reveals %2.", "female")
                : tr("%1 reveals %2.", "male")
            ).arg(sanitizeHtml(player->getName()))
             .arg(zone->getTranslatedName(true, CaseRevealZone)));
        }
    } else if (cardId == -2) {
        if (otherPlayer) {
            if (isFemale(player)) {
                if (isFemale(otherPlayer))
                    str = tr("%1 randomly reveals %2%3 to %4.", "p1 female, p2 female");
                else
                    str = tr("%1 randomly reveals %2%3 to %4.", "p1 female, p2 male");
            } else {
                if (isFemale(otherPlayer))
                    str = tr("%1 randomly reveals %2%3 to %4.", "p1 male, p2 female");
                else
                    str = tr("%1 randomly reveals %2%3 to %4.", "p1 male, p2 male");
            }
            appendHtml(str.arg(sanitizeHtml(player->getName())).arg(cardStr).arg(fromStr).arg(sanitizeHtml(otherPlayer->getName())));
        } else {
            if (isFemale(player))
                appendHtml(tr("%1 randomly reveals %2%3.", "female").arg(sanitizeHtml(player->getName())).arg(cardStr).arg(fromStr));
            else
                appendHtml(tr("%1 randomly reveals %2%3.", "male").arg(sanitizeHtml(player->getName())).arg(cardStr).arg(fromStr));
        }
    } else {
        if (faceDown && (player == otherPlayer)) {
            if (cardName.isEmpty()) {
                if (isFemale(player))
                    str = tr("%1 peeks at face down card #%2.", "female");
                else
                    str = tr("%1 peeks at face down card #%2.", "male");
                appendHtml(str.arg(sanitizeHtml(player->getName())).arg(cardId));
            } else {
                if (isFemale(player))
                    str = tr("%1 peeks at face down card #%2: %3.", "female");
                else
                    str = tr("%1 peeks at face down card #%2: %3.", "male");
                appendHtml(str.arg(sanitizeHtml(player->getName())).arg(cardId).arg(cardStr));
            }
        } else if (otherPlayer) {
            if (isFemale(player)) {
                if (isFemale(otherPlayer))
                    str = tr("%1 reveals %2%3 to %4.", "p1 female, p2 female");
                else
                    str = tr("%1 reveals %2%3 to %4.", "p1 female, p2 male");
            } else {
                if (isFemale(otherPlayer))
                    str = tr("%1 reveals %2%3 to %4.", "p1 male, p2 female");
                else
                    str = tr("%1 reveals %2%3 to %4.", "p1 male, p2 male");
            }
            appendHtml(str.arg(sanitizeHtml(player->getName())).arg(cardStr).arg(fromStr).arg(sanitizeHtml(otherPlayer->getName())));
        } else {
            if (isFemale(player))
                appendHtml(tr("%1 reveals %2%3.", "female").arg(sanitizeHtml(player->getName())).arg(cardStr).arg(fromStr));
            else
                appendHtml(tr("%1 reveals %2%3.", "male").arg(sanitizeHtml(player->getName())).arg(cardStr).arg(fromStr));
        }
    }
}

void MessageLogWidget::logAlwaysRevealTopCard(Player *player, CardZone *zone, bool reveal)
{
    appendHtml((reveal
        ? tr("%1 is now keeping the top card %2 revealed.")
        : tr("%1 is not revealing the top card %2 any longer.")
        ).arg(sanitizeHtml(player->getName()))
         .arg(zone->getTranslatedName(true, CaseTopCardsOfZone))
    );
}

void MessageLogWidget::logSetActivePlayer(Player *player)
{
    soundEngine->notification();
    
    QString str;
    if (isFemale(player))
        str = tr("It is now %1's turn.", "female");
    else
        str = tr("It is now %1's turn.", "male");
    appendHtml("<br><font color=\"green\"><b>" + str.arg(player->getName()) + "</b></font><br>");
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
    appendHtml("<font color=\"green\"><b>" + tr("It is now the %1.").arg(phaseName) + "</b></font>");
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
    : ChatView(_tabSupervisor, _game, false, parent)
{
}
