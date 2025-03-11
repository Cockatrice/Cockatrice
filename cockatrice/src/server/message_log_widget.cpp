#include "message_log_widget.h"

#include "../client/sound_engine.h"
#include "../client/translate_counter_name.h"
#include "../game/cards/card_item.h"
#include "../game/phase.h"
#include "../game/player/player.h"
#include "../game/zones/card_zone.h"
#include "pb/context_move_card.pb.h"
#include "pb/context_mulligan.pb.h"
#include "pb/serverinfo_user.pb.h"

#include <utility>

const QString MessageLogWidget::getCurrentTime()
{
    int seconds = *elapsedSeconds;
    int minutes = seconds / 60;
    seconds -= minutes * 60;
    int hours = minutes / 60;
    minutes -= hours * 60;
    return QString("[%1:%2:%3] ")
        .arg(QString::number(hours).rightJustified(2, '0'))
        .arg(QString::number(minutes).rightJustified(2, '0'))
        .arg(QString::number(seconds).rightJustified(2, '0'));
}

const QString &MessageLogWidget::tableConstant() const
{
    static const QString constant("table");
    return constant;
}

const QString &MessageLogWidget::graveyardConstant() const
{
    static const QString constant("grave");
    return constant;
}

const QString &MessageLogWidget::exileConstant() const
{
    static const QString constant("rfg");
    return constant;
}

const QString &MessageLogWidget::handConstant() const
{
    static const QString constant("hand");
    return constant;
}

const QString &MessageLogWidget::deckConstant() const
{
    static const QString constant("deck");
    return constant;
}

const QString &MessageLogWidget::sideboardConstant() const
{
    static const QString constant("sb");
    return constant;
}

const QString &MessageLogWidget::stackConstant() const
{
    static const QString constant("stack");
    return constant;
}

QString MessageLogWidget::sanitizeHtml(QString dirty) const
{
    return dirty.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;").replace("\"", "&quot;");
}

QString MessageLogWidget::cardLink(const QString cardName) const
{
    return QString("<i><a href=\"card://%1\">%2</a></i>").arg(cardName).arg(cardName);
}

QPair<QString, QString>
MessageLogWidget::getFromStr(CardZone *zone, QString cardName, int position, bool ownerChange) const
{
    bool cardNameContainsStartZone = false;
    QString fromStr;
    QString zoneName = zone->getName();

    if (zoneName == tableConstant()) {
        fromStr = tr(" from play");
    } else if (zoneName == graveyardConstant()) {
        fromStr = tr(" from their graveyard");
    } else if (zoneName == exileConstant()) {
        fromStr = tr(" from exile");
    } else if (zoneName == handConstant()) {
        fromStr = tr(" from their hand");
    } else if (zoneName == deckConstant()) {
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
    } else if (zoneName == sideboardConstant()) {
        fromStr = tr(" from sideboard");
    } else if (zoneName == stackConstant()) {
        fromStr = tr(" from the stack");
    }

    if (!cardNameContainsStartZone) {
        cardName.clear();
    }
    return QPair<QString, QString>(cardName, fromStr);
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

void MessageLogWidget::logCreateToken(Player *player, QString cardName, QString pt)
{
    appendHtmlServerMessage(tr("%1 creates token: %2%3.")
                                .arg(sanitizeHtml(player->getName()))
                                .arg(cardLink(std::move(cardName)))
                                .arg(pt.isEmpty() ? QString() : QString(" (%1)").arg(sanitizeHtml(pt))));
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
    if ((startZoneName == tableConstant() && targetZoneName == tableConstant() && !ownerChanged) ||
        (startZoneName == handConstant() && targetZoneName == handConstant()) ||
        (startZoneName == exileConstant() && targetZoneName == exileConstant())) {
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
    if (targetZoneName == tableConstant()) {
        soundEngine->playSound("play_card");
        if (card->getFaceDown()) {
            finalStr = tr("%1 puts %2 into play%3 face down.");
        } else {
            finalStr = tr("%1 puts %2 into play%3.");
        }
    } else if (targetZoneName == graveyardConstant()) {
        finalStr = tr("%1 puts %2%3 into their graveyard.");
    } else if (targetZoneName == exileConstant()) {
        finalStr = tr("%1 exiles %2%3.");
    } else if (targetZoneName == handConstant()) {
        finalStr = tr("%1 moves %2%3 to their hand.");
    } else if (targetZoneName == deckConstant()) {
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
    } else if (targetZoneName == sideboardConstant()) {
        finalStr = tr("%1 moves %2%3 to sideboard.");
    } else if (targetZoneName == stackConstant()) {
        soundEngine->playSound("play_card");
        finalStr = tr("%1 plays %2%3.");
    }

    if (usesNewX) {
        appendHtmlServerMessage(
            finalStr.arg(sanitizeHtml(player->getName())).arg(cardStr).arg(nameFrom.second).arg(newX));
    } else {
        appendHtmlServerMessage(finalStr.arg(sanitizeHtml(player->getName())).arg(cardStr).arg(nameFrom.second));
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
               phase.name + "</b></font>");
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

    connect(player, SIGNAL(logSay(Player *, QString)), this, SLOT(logSay(Player *, QString)));
    connect(player, &Player::logShuffle, this, &MessageLogWidget::logShuffle);
    connect(player, SIGNAL(logRollDie(Player *, int, const QList<uint> &)), this,
            SLOT(logRollDie(Player *, int, const QList<uint> &)));
    connect(player, SIGNAL(logCreateArrow(Player *, Player *, QString, Player *, QString, bool)), this,
            SLOT(logCreateArrow(Player *, Player *, QString, Player *, QString, bool)));
    connect(player, SIGNAL(logCreateToken(Player *, QString, QString)), this,
            SLOT(logCreateToken(Player *, QString, QString)));
    connect(player, SIGNAL(logSetCounter(Player *, QString, int, int)), this,
            SLOT(logSetCounter(Player *, QString, int, int)));
    connect(player, SIGNAL(logSetCardCounter(Player *, QString, int, int, int)), this,
            SLOT(logSetCardCounter(Player *, QString, int, int, int)));
    connect(player, SIGNAL(logSetTapped(Player *, CardItem *, bool)), this,
            SLOT(logSetTapped(Player *, CardItem *, bool)));
    connect(player, SIGNAL(logSetDoesntUntap(Player *, CardItem *, bool)), this,
            SLOT(logSetDoesntUntap(Player *, CardItem *, bool)));
    connect(player, SIGNAL(logSetPT(Player *, CardItem *, QString)), this,
            SLOT(logSetPT(Player *, CardItem *, QString)));
    connect(player, SIGNAL(logSetAnnotation(Player *, CardItem *, QString)), this,
            SLOT(logSetAnnotation(Player *, CardItem *, QString)));
    connect(player, SIGNAL(logMoveCard(Player *, CardItem *, CardZone *, int, CardZone *, int)), this,
            SLOT(logMoveCard(Player *, CardItem *, CardZone *, int, CardZone *, int)));
    connect(player, SIGNAL(logFlipCard(Player *, QString, bool)), this, SLOT(logFlipCard(Player *, QString, bool)));
    connect(player, SIGNAL(logDestroyCard(Player *, QString)), this, SLOT(logDestroyCard(Player *, QString)));
    connect(player, SIGNAL(logAttachCard(Player *, QString, Player *, QString)), this,
            SLOT(logAttachCard(Player *, QString, Player *, QString)));
    connect(player, SIGNAL(logUnattachCard(Player *, QString)), this, SLOT(logUnattachCard(Player *, QString)));
    connect(player, &Player::logDumpZone, this, &MessageLogWidget::logDumpZone);
    connect(player, SIGNAL(logDrawCards(Player *, int, bool)), this, SLOT(logDrawCards(Player *, int, bool)));
    connect(player, SIGNAL(logUndoDraw(Player *, QString)), this, SLOT(logUndoDraw(Player *, QString)));
    connect(player, SIGNAL(logRevealCards(Player *, CardZone *, int, QString, Player *, bool, int, bool)), this,
            SLOT(logRevealCards(Player *, CardZone *, int, QString, Player *, bool, int, bool)));
    connect(player, SIGNAL(logAlwaysRevealTopCard(Player *, CardZone *, bool)), this,
            SLOT(logAlwaysRevealTopCard(Player *, CardZone *, bool)));
    connect(player, SIGNAL(logAlwaysLookAtTopCard(Player *, CardZone *, bool)), this,
            SLOT(logAlwaysLookAtTopCard(Player *, CardZone *, bool)));
}

MessageLogWidget::MessageLogWidget(TabSupervisor *_tabSupervisor, TabGame *_game, int *seconds, QWidget *parent)
    : ChatView(_tabSupervisor, _game, true, parent), mulliganNumber(0), currentContext(MessageContext_None)
{
    elapsedSeconds = seconds;
    ChatView::setTime(seconds);
}
