#include "message_log_widget.h"

#include "../client/sound_engine.h"
#include "../client/translate_counter_name.h"
#include "../game/board/card_item.h"
#include "../game/phase.h"
#include "../game/player/player.h"
#include "../game/zones/card_zone.h"
#include "pb/context_move_card.pb.h"
#include "pb/context_mulligan.pb.h"
#include "pb/serverinfo_user.pb.h"

#include <utility>

static const QString TABLE_ZONE_NAME = "table";
static const QString GRAVE_ZONE_NAME = "grave";
static const QString EXILE_ZONE_NAME = "rfg";
static const QString HAND_ZONE_NAME = "hand";
static const QString DECK_ZONE_NAME = "deck";
static const QString SIDEBOARD_ZONE_NAME = "sb";
static const QString STACK_ZONE_NAME = "stack";

static QString sanitizeHtml(QString dirty)
{
    return dirty.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;").replace("\"", "&quot;");
}

static QString cardLink(const QString &cardName)
{
    return QString("<i><a href=\"card://%1\">%2</a></i>").arg(cardName).arg(cardName);
}

QPair<QString, QString> MessageLogWidget::getFromStr(CardZone *zone, QString cardName, int position, bool ownerChange)
{
    bool cardNameContainsStartZone = false;
    QString fromStr;
    QString zoneName = zone->getName();

    if (zoneName == TABLE_ZONE_NAME) {
        fromStr = tr(" from play");
    } else if (zoneName == GRAVE_ZONE_NAME) {
        fromStr = tr(" from their graveyard");
    } else if (zoneName == EXILE_ZONE_NAME) {
        fromStr = tr(" from exile");
    } else if (zoneName == HAND_ZONE_NAME) {
        fromStr = tr(" from their hand");
    } else if (zoneName == DECK_ZONE_NAME) {
        if (position == 0) {
            if (cardName.isEmpty()) {
                if (ownerChange) {
                    cardName = tr("the top card of %1's library").arg(zone->getPlayer()->getName());
                } else {
                    cardName = tr("the top card of their library");
                }
                cardNameContainsStartZone = true;
            } else {
                if (ownerChange) {
                    fromStr = tr(" from the top of %1's library").arg(zone->getPlayer()->getName());
                } else {
                    fromStr = tr(" from the top of their library");
                }
            }
        } else if (position >= zone->getCards().size() - 1) {
            if (cardName.isEmpty()) {
                if (ownerChange) {
                    cardName = tr("the bottom card of %1's library").arg(zone->getPlayer()->getName());
                } else {
                    cardName = tr("the bottom card of their library");
                }
                cardNameContainsStartZone = true;
            } else {
                if (ownerChange) {
                    fromStr = tr(" from the bottom of %1's library").arg(zone->getPlayer()->getName());
                } else {
                    fromStr = tr(" from the bottom of their library");
                }
            }
        } else {
            if (ownerChange) {
                fromStr = tr(" from %1's library").arg(zone->getPlayer()->getName());
            } else {
                fromStr = tr(" from their library");
            }
        }
    } else if (zoneName == SIDEBOARD_ZONE_NAME) {
        fromStr = tr(" from sideboard");
    } else if (zoneName == STACK_ZONE_NAME) {
        fromStr = tr(" from the stack");
    } else {
        fromStr = tr(" from custom zone '%1'").arg(zoneName);
    }

    if (!cardNameContainsStartZone) {
        cardName.clear();
    }
    return {cardName, fromStr};
}

void MessageLogWidget::containerProcessingDone()
{
    currentContext = MessageContext_None;
    messageSuffix = messagePrefix = QString();
}

void MessageLogWidget::containerProcessingStarted(const GameEventContext &context)
{
    if (context.HasExtension(Context_MoveCard::ext)) {
        currentContext = MessageContext_MoveCard;
    } else if (context.HasExtension(Context_Mulligan::ext)) {
        currentContext = MessageContext_Mulligan;
    }
}

void MessageLogWidget::logAlwaysRevealTopCard(Player *player, CardZone *zone, bool reveal)
{
    appendHtmlServerMessage((reveal ? tr("%1 is now keeping the top card %2 revealed.")
                                    : tr("%1 is not revealing the top card %2 any longer."))
                                .arg(sanitizeHtml(player->getName()))
                                .arg(zone->getTranslatedName(true, CaseTopCardsOfZone)));
}

void MessageLogWidget::logAlwaysLookAtTopCard(Player *player, CardZone *zone, bool reveal)
{
    appendHtmlServerMessage((reveal ? tr("%1 can now look at top card %2 at any time.")
                                    : tr("%1 no longer can look at top card %2 at any time."))
                                .arg(sanitizeHtml(player->getName()))
                                .arg(zone->getTranslatedName(true, CaseTopCardsOfZone)));
}

void MessageLogWidget::logAttachCard(Player *player, QString cardName, Player *targetPlayer, QString targetCardName)
{
    appendHtmlServerMessage(tr("%1 attaches %2 to %3's %4.")
                                .arg(sanitizeHtml(player->getName()))
                                .arg(cardLink(std::move(cardName)))
                                .arg(sanitizeHtml(targetPlayer->getName()))
                                .arg(cardLink(std::move(targetCardName))));
}

void MessageLogWidget::logConcede(Player *player)
{
    soundEngine->playSound("player_concede");
    appendHtmlServerMessage(tr("%1 has conceded the game.").arg(sanitizeHtml(player->getName())), true);
}

void MessageLogWidget::logUnconcede(Player *player)
{
    soundEngine->playSound("player_concede");
    appendHtmlServerMessage(tr("%1 has unconceded the game.").arg(sanitizeHtml(player->getName())), true);
}

void MessageLogWidget::logConnectionStateChanged(Player *player, bool connectionState)
{
    if (connectionState) {
        soundEngine->playSound("player_reconnect");
        appendHtmlServerMessage(tr("%1 has restored connection to the game.").arg(sanitizeHtml(player->getName())),
                                true);
    } else {
        soundEngine->playSound("player_disconnect");
        appendHtmlServerMessage(tr("%1 has lost connection to the game.").arg(sanitizeHtml(player->getName())), true);
    }
}

void MessageLogWidget::logCreateArrow(Player *player,
                                      Player *startPlayer,
                                      QString startCard,
                                      Player *targetPlayer,
                                      QString targetCard,
                                      bool playerTarget)
{
    startCard = cardLink(startCard);
    targetCard = cardLink(targetCard);
    QString str;
    if (playerTarget) {
        if (player == startPlayer && player == targetPlayer) {
            str = tr("%1 points from their %2 to themselves.");
            appendHtmlServerMessage(str.arg(sanitizeHtml(player->getName())).arg(startCard));
        } else if (player == startPlayer) {
            str = tr("%1 points from their %2 to %3.");
            appendHtmlServerMessage(
                str.arg(sanitizeHtml(player->getName())).arg(startCard).arg(sanitizeHtml(targetPlayer->getName())));
        } else if (player == targetPlayer) {
            str = tr("%1 points from %2's %3 to themselves.");
            appendHtmlServerMessage(
                str.arg(sanitizeHtml(player->getName())).arg(sanitizeHtml(startPlayer->getName())).arg(startCard));
        } else {
            str = tr("%1 points from %2's %3 to %4.");
            appendHtmlServerMessage(str.arg(sanitizeHtml(player->getName()))
                                        .arg(sanitizeHtml(startPlayer->getName()))
                                        .arg(startCard)
                                        .arg(sanitizeHtml(targetPlayer->getName())));
        }
    } else {
        if (player == startPlayer && player == targetPlayer) {
            str = tr("%1 points from their %2 to their %3.");
            appendHtmlServerMessage(str.arg(sanitizeHtml(player->getName())).arg(startCard).arg(targetCard));
        } else if (player == startPlayer) {
            str = tr("%1 points from their %2 to %3's %4.");
            appendHtmlServerMessage(str.arg(sanitizeHtml(player->getName()))
                                        .arg(startCard)
                                        .arg(sanitizeHtml(targetPlayer->getName()))
                                        .arg(targetCard));
        } else if (player == targetPlayer) {
            str = tr("%1 points from %2's %3 to their own %4.");
            appendHtmlServerMessage(str.arg(sanitizeHtml(player->getName()))
                                        .arg(sanitizeHtml(startPlayer->getName()))
                                        .arg(startCard)
                                        .arg(targetCard));
        } else {
            str = tr("%1 points from %2's %3 to %4's %5.");
            appendHtmlServerMessage(str.arg(sanitizeHtml(player->getName()))
                                        .arg(sanitizeHtml(startPlayer->getName()))
                                        .arg(startCard)
                                        .arg(sanitizeHtml(targetPlayer->getName()))
                                        .arg(targetCard));
        }
    }
}

void MessageLogWidget::logCreateToken(Player *player, QString cardName, QString pt, bool faceDown)
{
    if (faceDown) {
        appendHtmlServerMessage(tr("%1 creates a face down token.").arg(sanitizeHtml(player->getName())));
    } else {
        appendHtmlServerMessage(tr("%1 creates token: %2%3.")
                                    .arg(sanitizeHtml(player->getName()))
                                    .arg(cardLink(std::move(cardName)))
                                    .arg(pt.isEmpty() ? QString() : QString(" (%1)").arg(sanitizeHtml(pt))));
    }
}

void MessageLogWidget::logDeckSelect(Player *player, QString deckHash, int sideboardSize)
{
    if (sideboardSize < 0) {
        appendHtmlServerMessage(tr("%1 has loaded a deck (%2).").arg(sanitizeHtml(player->getName())).arg(deckHash));
    } else {
        appendHtmlServerMessage(tr("%1 has loaded a deck with %2 sideboard cards (%3).")
                                    .arg(sanitizeHtml(player->getName()))
                                    .arg("<font class=\"blue\">" + QString::number(sideboardSize) + "</font>")
                                    .arg(deckHash));
    }
}

void MessageLogWidget::logDestroyCard(Player *player, QString cardName)
{
    appendHtmlServerMessage(
        tr("%1 destroys %2.").arg(sanitizeHtml(player->getName())).arg(cardLink(std::move(cardName))));
}

void MessageLogWidget::logMoveCard(Player *player,
                                   CardItem *card,
                                   CardZone *startZone,
                                   int oldX,
                                   CardZone *targetZone,
                                   int newX)
{
    if (currentContext == MessageContext_Mulligan) {
        return;
    }

    QString startZoneName = startZone->getName();
    QString targetZoneName = targetZone->getName();
    bool ownerChanged = startZone->getPlayer() != targetZone->getPlayer();

    // do not log if moved within the same zone
    if ((startZoneName == TABLE_ZONE_NAME && targetZoneName == TABLE_ZONE_NAME && !ownerChanged) ||
        (startZoneName == HAND_ZONE_NAME && targetZoneName == HAND_ZONE_NAME) ||
        (startZoneName == EXILE_ZONE_NAME && targetZoneName == EXILE_ZONE_NAME)) {
        return;
    }

    QString cardName = card->getName();
    QPair<QString, QString> nameFrom = getFromStr(startZone, cardName, oldX, ownerChanged);
    if (!nameFrom.first.isEmpty()) {
        cardName = nameFrom.first;
    }

    QString cardStr;
    if (!nameFrom.first.isEmpty()) {
        cardStr = cardName;
    } else if (cardName.isEmpty()) {
        cardStr = tr("a card");
    } else {
        cardStr = cardLink(cardName);
    }

    if (ownerChanged && (startZone->getPlayer() == player)) {
        appendHtmlServerMessage(tr("%1 gives %2 control over %3.")
                                    .arg(sanitizeHtml(player->getName()))
                                    .arg(sanitizeHtml(targetZone->getPlayer()->getName()))
                                    .arg(cardStr));
        return;
    }

    QString finalStr;
    bool usesNewX = false;
    if (targetZoneName == TABLE_ZONE_NAME) {
        soundEngine->playSound("play_card");
        if (card->getFaceDown()) {
            finalStr = tr("%1 puts %2 into play%3 face down.");
        } else {
            finalStr = tr("%1 puts %2 into play%3.");
        }
    } else if (targetZoneName == GRAVE_ZONE_NAME) {
        finalStr = tr("%1 puts %2%3 into their graveyard.");
    } else if (targetZoneName == EXILE_ZONE_NAME) {
        finalStr = tr("%1 exiles %2%3.");
    } else if (targetZoneName == HAND_ZONE_NAME) {
        finalStr = tr("%1 moves %2%3 to their hand.");
    } else if (targetZoneName == DECK_ZONE_NAME) {
        if (newX == -1) {
            finalStr = tr("%1 puts %2%3 into their library.");
        } else if (newX >= targetZone->getCards().size()) {
            finalStr = tr("%1 puts %2%3 onto the bottom of their library.");
        } else if (newX == 0) {
            finalStr = tr("%1 puts %2%3 on top of their library.");
        } else {
            ++newX;
            usesNewX = true;
            finalStr = tr("%1 puts %2%3 into their library %4 cards from the top.");
        }
    } else if (targetZoneName == SIDEBOARD_ZONE_NAME) {
        finalStr = tr("%1 moves %2%3 to sideboard.");
    } else if (targetZoneName == STACK_ZONE_NAME) {
        soundEngine->playSound("play_card");
        finalStr = tr("%1 plays %2%3.");
    } else {
        finalStr = tr("%1 moves %2%3 to custom zone '%4'.");
    }

    if (usesNewX) {
        appendHtmlServerMessage(
            finalStr.arg(sanitizeHtml(player->getName())).arg(cardStr).arg(nameFrom.second).arg(newX));
    } else {
        appendHtmlServerMessage(
            finalStr.arg(sanitizeHtml(player->getName())).arg(cardStr).arg(nameFrom.second).arg(targetZoneName));
    }
}

void MessageLogWidget::logDrawCards(Player *player, int number, bool deckIsEmpty)
{
    soundEngine->playSound("draw_card");
    if (currentContext == MessageContext_Mulligan) {
        logMulligan(player, number);
    } else {
        if (deckIsEmpty && number == 0) {
            appendHtmlServerMessage(tr("%1 tries to draw from an empty library").arg(sanitizeHtml(player->getName())));
        } else {
            appendHtmlServerMessage(tr("%1 draws %2 card(s).", "", number)
                                        .arg(sanitizeHtml(player->getName()))
                                        .arg("<font class=\"blue\">" + QString::number(number) + "</font>"));
        }
    }
}

void MessageLogWidget::logDumpZone(Player *player, CardZone *zone, int numberCards, bool isReversed)
{
    if (numberCards == -1) {
        appendHtmlServerMessage(tr("%1 is looking at %2.")
                                    .arg(sanitizeHtml(player->getName()))
                                    .arg(zone->getTranslatedName(zone->getPlayer() == player, CaseLookAtZone)));
    } else {
        appendHtmlServerMessage(
            tr("%1 is looking at the %4 %3 card(s) %2.", "top card for singular, top %3 cards for plural", numberCards)
                .arg(sanitizeHtml(player->getName()))
                .arg(zone->getTranslatedName(zone->getPlayer() == player, CaseTopCardsOfZone))
                .arg("<font class=\"blue\">" + QString::number(numberCards) + "</font>")
                .arg(isReversed ? tr("bottom") : tr("top")));
    }
}

void MessageLogWidget::logFlipCard(Player *player, QString cardName, bool faceDown)
{
    if (faceDown) {
        appendHtmlServerMessage(
            tr("%1 turns %2 face-down.").arg(sanitizeHtml(player->getName())).arg(cardLink(cardName)));
    } else {
        appendHtmlServerMessage(
            tr("%1 turns %2 face-up.").arg(sanitizeHtml(player->getName())).arg(cardLink(cardName)));
    }
}

void MessageLogWidget::logGameClosed()
{
    appendHtmlServerMessage(tr("The game has been closed."));
}

void MessageLogWidget::logGameStart()
{
    appendHtmlServerMessage(tr("The game has started."));
}

void MessageLogWidget::logJoin(Player *player)
{
    soundEngine->playSound("player_join");
    appendHtmlServerMessage(tr("%1 has joined the game.").arg(sanitizeHtml(player->getName())));
}

void MessageLogWidget::logJoinSpectator(QString name)
{
    soundEngine->playSound("spectator_join");
    appendHtmlServerMessage(tr("%1 is now watching the game.").arg(sanitizeHtml(std::move(name))));
}

void MessageLogWidget::logKicked()
{
    appendHtmlServerMessage(tr("You have been kicked out of the game."), true);
}

void MessageLogWidget::logLeave(Player *player, QString reason)
{
    soundEngine->playSound("player_leave");
    appendHtmlServerMessage(
        tr("%1 has left the game (%2).").arg(sanitizeHtml(player->getName()), sanitizeHtml(std::move(reason))), true);
}

void MessageLogWidget::logLeaveSpectator(QString name, QString reason)
{
    soundEngine->playSound("spectator_leave");
    appendHtmlServerMessage(tr("%1 is not watching the game any more (%2).")
                                .arg(sanitizeHtml(std::move(name)), sanitizeHtml(std::move(reason))));
}

void MessageLogWidget::logNotReadyStart(Player *player)
{
    appendHtmlServerMessage(tr("%1 is not ready to start the game any more.").arg(sanitizeHtml(player->getName())));
}

void MessageLogWidget::logMulligan(Player *player, int number)
{
    if (!player) {
        return;
    }
    if (number > 0) {
        appendHtmlServerMessage(tr("%1 shuffles their deck and draws a new hand of %2 card(s).", "", number)
                                    .arg(sanitizeHtml(player->getName()))
                                    .arg(number));
    } else {
        appendHtmlServerMessage(
            tr("%1 shuffles their deck and draws a new hand.").arg(sanitizeHtml(player->getName())));
    }
}

void MessageLogWidget::logReplayStarted(int gameId)
{
    appendHtmlServerMessage(tr("You are watching a replay of game #%1.").arg(gameId));
}

void MessageLogWidget::logReadyStart(Player *player)
{
    appendHtmlServerMessage(tr("%1 is ready to start the game.").arg(sanitizeHtml(player->getName())));
}

void MessageLogWidget::logRevealCards(Player *player,
                                      CardZone *zone,
                                      int cardId,
                                      QString cardName,
                                      Player *otherPlayer,
                                      bool faceDown,
                                      int amount,
                                      bool isLentToAnotherPlayer)
{
    // getFromStr uses cardname.empty() to check if it should contain the start zone, it's not actually used
    QPair<QString, QString> temp = getFromStr(zone, amount == 1 ? cardName : QString::number(amount), cardId, false);
    bool cardNameContainsStartZone = false;
    if (!temp.first.isEmpty()) {
        cardNameContainsStartZone = true;
        cardName = temp.first;
    }
    QString fromStr = temp.second;

    QString cardStr;
    if (cardNameContainsStartZone) {
        cardStr = cardName;
    } else if (cardName.isEmpty()) {
        if (amount == 0) {
            cardStr = tr("cards", "an unknown amount of cards");
        } else {
            cardStr = tr("%1 card(s)", "a card for singular, %1 cards for plural", amount)
                          .arg("<font class=\"blue\">" + QString::number(amount) + "</font>");
        }
    } else {
        cardStr = cardLink(cardName);
    }
    if (cardId == -1) {
        if (otherPlayer) {
            if (isLentToAnotherPlayer) {
                appendHtmlServerMessage(tr("%1 lends %2 to %3.")
                                            .arg(sanitizeHtml(player->getName()))
                                            .arg(zone->getTranslatedName(true, CaseRevealZone))
                                            .arg(sanitizeHtml(otherPlayer->getName())));
            } else {
                appendHtmlServerMessage(tr("%1 reveals %2 to %3.")
                                            .arg(sanitizeHtml(player->getName()))
                                            .arg(zone->getTranslatedName(true, CaseRevealZone))
                                            .arg(sanitizeHtml(otherPlayer->getName())));
            }
        } else {
            appendHtmlServerMessage(tr("%1 reveals %2.")
                                        .arg(sanitizeHtml(player->getName()))
                                        .arg(zone->getTranslatedName(true, CaseRevealZone)));
        }
    } else if (cardId == -2) {
        if (otherPlayer) {
            appendHtmlServerMessage(tr("%1 randomly reveals %2%3 to %4.")
                                        .arg(sanitizeHtml(player->getName()))
                                        .arg(cardStr)
                                        .arg(fromStr)
                                        .arg(sanitizeHtml(otherPlayer->getName())));
        } else {
            appendHtmlServerMessage(
                tr("%1 randomly reveals %2%3.").arg(sanitizeHtml(player->getName())).arg(cardStr).arg(fromStr));
        }
    } else {
        if (faceDown && player == otherPlayer) {
            if (cardName.isEmpty()) {
                appendHtmlServerMessage(
                    tr("%1 peeks at face down card #%2.").arg(sanitizeHtml(player->getName())).arg(cardId));
            } else {
                appendHtmlServerMessage(tr("%1 peeks at face down card #%2: %3.")
                                            .arg(sanitizeHtml(player->getName()))
                                            .arg(cardId)
                                            .arg(cardStr));
            }
        } else if (otherPlayer) {
            appendHtmlServerMessage(tr("%1 reveals %2%3 to %4.")
                                        .arg(sanitizeHtml(player->getName()))
                                        .arg(cardStr)
                                        .arg(fromStr)
                                        .arg(sanitizeHtml(otherPlayer->getName())));
        } else {
            appendHtmlServerMessage(
                tr("%1 reveals %2%3.").arg(sanitizeHtml(player->getName())).arg(cardStr).arg(fromStr));
        }
    }
}

void MessageLogWidget::logReverseTurn(Player *player, bool reversed)
{
    appendHtmlServerMessage(tr("%1 reversed turn order, now it's %2.")
                                .arg(sanitizeHtml(player->getName()))
                                .arg(reversed ? tr("reversed") : tr("normal")));
}

void MessageLogWidget::logRollDie(Player *player, int sides, const QList<uint> &rolls)
{
    if (rolls.length() == 1) {
        const auto roll = rolls.at(0);
        if (sides == 2) {
            QString coinOptions[2] = {tr("Heads") + " (1)", tr("Tails") + " (2)"};
            appendHtmlServerMessage(tr("%1 flipped a coin. It landed as %2.")
                                        .arg(sanitizeHtml(player->getName()))
                                        .arg("<font class=\"blue\">" + coinOptions[roll - 1] + "</font>"));
        } else {
            appendHtmlServerMessage(tr("%1 rolls a %2 with a %3-sided die.")
                                        .arg(sanitizeHtml(player->getName()))
                                        .arg("<font class=\"blue\">" + QString::number(roll) + "</font>")
                                        .arg("<font class=\"blue\">" + QString::number(sides) + "</font>"));
        }
    } else {
        if (sides == 2) {
            appendHtmlServerMessage(tr("%1 flips %2 coins. There are %3 heads and %4 tails.")
                                        .arg(sanitizeHtml(player->getName()))
                                        .arg("<font class=\"blue\">" + QString::number(rolls.length()) + "</font>")
                                        .arg("<font class=\"blue\">" + QString::number(rolls.count(1)) + "</font>")
                                        .arg("<font class=\"blue\">" + QString::number(rolls.count(2)) + "</font>"));
        } else {
            QStringList rollsStrings;
            for (const auto &roll : rolls) {
                rollsStrings.append(QString::number(roll));
            }
            appendHtmlServerMessage(tr("%1 rolls a %2-sided dice %3 times: %4.")
                                        .arg(sanitizeHtml(player->getName()))
                                        .arg("<font class=\"blue\">" + QString::number(sides) + "</font>")
                                        .arg("<font class=\"blue\">" + QString::number(rolls.length()) + "</font>")
                                        .arg("<font class=\"blue\">" + rollsStrings.join(", ") + "</font>"));
        }
    }
    soundEngine->playSound("roll_dice");
}

void MessageLogWidget::logSay(Player *player, QString message)
{
    appendMessage(std::move(message), {}, *player->getUserInfo(), true);
}

void MessageLogWidget::logSetActivePhase(int phaseNumber)
{
    Phase phase = Phases::getPhase(phaseNumber);

    soundEngine->playSound(phase.soundFileName);

    appendHtml("<font color=\"" + phase.color + "\"><b>" + QDateTime::currentDateTime().toString("[hh:mm:ss] ") +
               phase.getName() + "</b></font>");
}

void MessageLogWidget::logSetActivePlayer(Player *player)
{
    appendHtml("<br><font color=\"green\"><b>" + QDateTime::currentDateTime().toString("[hh:mm:ss] ") +
               QString(tr("%1's turn.")).arg(player->getName()) + "</b></font><br>");
}

void MessageLogWidget::logSetAnnotation(Player *player, CardItem *card, QString newAnnotation)
{
    appendHtmlServerMessage(
        QString(tr("%1 sets annotation of %2 to %3."))
            .arg(sanitizeHtml(player->getName()))
            .arg(cardLink(card->getName()))
            .arg(QString("&quot;<font class=\"blue\">%1</font>&quot;").arg(sanitizeHtml(std::move(newAnnotation)))));
}

void MessageLogWidget::logSetCardCounter(Player *player, QString cardName, int counterId, int value, int oldValue)
{
    QString finalStr;
    int delta = abs(oldValue - value);
    if (value > oldValue) {
        finalStr = tr("%1 places %2 %3 on %4 (now %5).");
    } else {
        finalStr = tr("%1 removes %2 %3 from %4 (now %5).");
    }

    QString colorStr;
    switch (counterId) {
        case 0:
            colorStr = tr("red counter(s)", "", delta);
            break;
        case 1:
            colorStr = tr("yellow counter(s)", "", delta);
            break;
        case 2:
            colorStr = tr("green counter(s)", "", delta);
            break;
        default:;
    }

    appendHtmlServerMessage(finalStr.arg(sanitizeHtml(player->getName()))
                                .arg("<font class=\"blue\">" + QString::number(delta) + "</font>")
                                .arg(colorStr)
                                .arg(cardLink(std::move(cardName)))
                                .arg(value));
}

void MessageLogWidget::logSetCounter(Player *player, QString counterName, int value, int oldValue)
{
    if (counterName == "life") {
        soundEngine->playSound("life_change");
    }

    QString counterDisplayName = TranslateCounterName::getDisplayName(counterName);
    appendHtmlServerMessage(tr("%1 sets counter %2 to %3 (%4%5).")
                                .arg(sanitizeHtml(player->getName()))
                                .arg(QString("<font class=\"blue\">%1</font>").arg(sanitizeHtml(counterDisplayName)))
                                .arg(QString("<font class=\"blue\">%1</font>").arg(value))
                                .arg(value > oldValue ? "+" : "")
                                .arg(value - oldValue));
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
    if (currentContext == MessageContext_MoveCard) {
        return;
    }

    QString name = card->getName();
    if (name.isEmpty()) {
        name = QString("<font class=\"blue\">card #%1</font>").arg(sanitizeHtml(QString::number(card->getId())));
    } else {
        name = cardLink(name);
    }
    QString playerName = sanitizeHtml(player->getName());
    if (newPT.isEmpty()) {
        appendHtmlServerMessage(tr("%1 removes the PT of %2.").arg(playerName).arg(name));
    } else {
        QString oldPT = card->getPT();
        if (oldPT.isEmpty()) {
            appendHtmlServerMessage(
                tr("%1 changes the PT of %2 from nothing to %4.").arg(playerName).arg(name).arg(newPT));
        } else {
            appendHtmlServerMessage(
                tr("%1 changes the PT of %2 from %3 to %4.").arg(playerName).arg(name).arg(oldPT).arg(newPT));
        }
    }
}

void MessageLogWidget::logSetSideboardLock(Player *player, bool locked)
{
    if (locked) {
        appendHtmlServerMessage(tr("%1 has locked their sideboard.").arg(sanitizeHtml(player->getName())));
    } else {
        appendHtmlServerMessage(tr("%1 has unlocked their sideboard.").arg(sanitizeHtml(player->getName())));
    }
}

void MessageLogWidget::logSetTapped(Player *player, CardItem *card, bool tapped)
{
    if (currentContext == MessageContext_MoveCard) {
        return;
    }

    if (tapped) {
        soundEngine->playSound("tap_card");
    } else {
        soundEngine->playSound("untap_card");
    }

    QString str;
    if (!card) {
        appendHtmlServerMessage((tapped ? tr("%1 taps their permanents.") : tr("%1 untaps their permanents."))
                                    .arg(sanitizeHtml(player->getName())));
    } else {
        appendHtmlServerMessage((tapped ? tr("%1 taps %2.") : tr("%1 untaps %2."))
                                    .arg(sanitizeHtml(player->getName()))
                                    .arg(cardLink(card->getName())));
    }
}

void MessageLogWidget::logShuffle(Player *player, CardZone *zone, int start, int end)
{
    if (currentContext == MessageContext_Mulligan) {
        return;
    }

    soundEngine->playSound("shuffle");
    // start and end are indexes into the portion of the deck that was shuffled
    // with negitive numbers counging from the bottom up.
    if (start == 0 && end == -1) {
        appendHtmlServerMessage(tr("%1 shuffles %2.")
                                    .arg(sanitizeHtml(player->getName()))
                                    .arg(zone->getTranslatedName(true, CaseShuffleZone)));
    } else if (start < 0 && end == -1) {
        appendHtmlServerMessage(tr("%1 shuffles the bottom %3 cards of %2.")
                                    .arg(sanitizeHtml(player->getName()))
                                    .arg(zone->getTranslatedName(true, CaseShuffleZone))
                                    .arg(-start));
    } else if (start < 0 && end > 0) {
        appendHtmlServerMessage(tr("%1 shuffles the top %3 cards of %2.")
                                    .arg(sanitizeHtml(player->getName()))
                                    .arg(zone->getTranslatedName(true, CaseShuffleZone))
                                    .arg(end + 1));
    } else {
        appendHtmlServerMessage(tr("%1 shuffles cards %3 - %4 of %2.")
                                    .arg(sanitizeHtml(player->getName()))
                                    .arg(zone->getTranslatedName(true, CaseShuffleZone))
                                    .arg(start)
                                    .arg(end));
    }
}

void MessageLogWidget::logSpectatorSay(const ServerInfo_User &spectator, QString message)
{
    appendMessage(std::move(message), {}, spectator, false);
}

void MessageLogWidget::logUnattachCard(Player *player, QString cardName)
{
    appendHtmlServerMessage(
        tr("%1 unattaches %2.").arg(sanitizeHtml(player->getName())).arg(cardLink(std::move(cardName))));
}

void MessageLogWidget::logUndoDraw(Player *player, QString cardName)
{
    if (cardName.isEmpty()) {
        appendHtmlServerMessage(tr("%1 undoes their last draw.").arg(sanitizeHtml(player->getName())));
    } else {
        appendHtmlServerMessage(
            tr("%1 undoes their last draw (%2).")
                .arg(sanitizeHtml(player->getName()))
                .arg(QString("<a href=\"card://%1\">%2</a>").arg(sanitizeHtml(cardName)).arg(sanitizeHtml(cardName))));
    }
}

void MessageLogWidget::setContextJudgeName(QString name)
{
    messagePrefix = QString("<span style=\"color:black\">");
    messageSuffix = QString("</span> [<img height=12 src=\"theme:icons/scales\"> %1]").arg(sanitizeHtml(name));
}

void MessageLogWidget::appendHtmlServerMessage(const QString &html, bool optionalIsBold, QString optionalFontColor)
{

    ChatView::appendHtmlServerMessage(messagePrefix + html + messageSuffix, optionalIsBold, optionalFontColor);
}

void MessageLogWidget::connectToPlayer(Player *player)
{

    connect(player, &Player::logSay, this, &MessageLogWidget::logSay);
    connect(player, &Player::logShuffle, this, &MessageLogWidget::logShuffle);
    connect(player, &Player::logRollDie, this, &MessageLogWidget::logRollDie);
    connect(player, &Player::logCreateArrow, this, &MessageLogWidget::logCreateArrow);
    connect(player, &Player::logCreateToken, this, &MessageLogWidget::logCreateToken);
    connect(player, &Player::logSetCounter, this, &MessageLogWidget::logSetCounter);
    connect(player, &Player::logSetCardCounter, this, &MessageLogWidget::logSetCardCounter);
    connect(player, &Player::logSetTapped, this, &MessageLogWidget::logSetTapped);
    connect(player, &Player::logSetDoesntUntap, this, &MessageLogWidget::logSetDoesntUntap);
    connect(player, &Player::logSetPT, this, &MessageLogWidget::logSetPT);
    connect(player, &Player::logSetAnnotation, this, &MessageLogWidget::logSetAnnotation);
    connect(player, &Player::logMoveCard, this, &MessageLogWidget::logMoveCard);
    connect(player, &Player::logFlipCard, this, &MessageLogWidget::logFlipCard);
    connect(player, &Player::logDestroyCard, this, &MessageLogWidget::logDestroyCard);
    connect(player, &Player::logAttachCard, this, &MessageLogWidget::logAttachCard);
    connect(player, &Player::logUnattachCard, this, &MessageLogWidget::logUnattachCard);
    connect(player, &Player::logDumpZone, this, &MessageLogWidget::logDumpZone);
    connect(player, &Player::logDrawCards, this, &MessageLogWidget::logDrawCards);
    connect(player, &Player::logUndoDraw, this, &MessageLogWidget::logUndoDraw);
    connect(player, &Player::logRevealCards, this, &MessageLogWidget::logRevealCards);
    connect(player, &Player::logAlwaysRevealTopCard, this, &MessageLogWidget::logAlwaysRevealTopCard);
    connect(player, &Player::logAlwaysLookAtTopCard, this, &MessageLogWidget::logAlwaysLookAtTopCard);
}

MessageLogWidget::MessageLogWidget(TabSupervisor *_tabSupervisor, TabGame *_game, QWidget *parent)
    : ChatView(_tabSupervisor, _game, true, parent), currentContext(MessageContext_None)
{
}
