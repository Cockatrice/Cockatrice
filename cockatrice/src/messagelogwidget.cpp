#include <utility>

#include "carditem.h"
#include "cardzone.h"
#include "messagelogwidget.h"
#include "pb/context_move_card.pb.h"
#include "pb/context_mulligan.pb.h"
#include "pb/serverinfo_user.pb.h"
#include "player.h"
#include "soundengine.h"

const QString MessageLogWidget::tableConstant() const
{
    static const QString constant("table");
    return constant;
}

const QString MessageLogWidget::graveyardConstant() const
{
    static const QString constant("grave");
    return constant;
}

const QString MessageLogWidget::exileConstant() const
{
    static const QString constant("rfg");
    return constant;
}

const QString MessageLogWidget::handConstant() const
{
    static const QString constant("hand");
    return constant;
}

const QString MessageLogWidget::deckConstant() const
{
    static const QString constant("deck");
    return constant;
}

const QString MessageLogWidget::sideboardConstant() const
{
    static const QString constant("sb");
    return constant;
}

const QString MessageLogWidget::stackConstant() const
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

    if (zoneName == tableConstant())
        fromStr = tr(" from play");
    else if (zoneName == graveyardConstant())
        fromStr = tr(" from their graveyard");
    else if (zoneName == exileConstant())
        fromStr = tr(" from exile");
    else if (zoneName == handConstant())
        fromStr = tr(" from their hand");
    else if (zoneName == deckConstant()) {
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
    } else if (zoneName == sideboardConstant())
        fromStr = tr(" from sideboard");
    else if (zoneName == stackConstant())
        fromStr = tr(" from the stack");

    if (!cardNameContainsStartZone)
        cardName.clear();
    return QPair<QString, QString>(cardName, fromStr);
}

void MessageLogWidget::containerProcessingDone()
{
    if (currentContext == MessageContext_MoveCard) {
        for (auto &i : moveCardQueue)
            logDoMoveCard(i);
        moveCardQueue.clear();
        moveCardTapped.clear();
        moveCardExtras.clear();
    } else if (currentContext == MessageContext_Mulligan) {
        logMulligan(mulliganPlayer, mulliganNumber);
        mulliganPlayer = nullptr;
        mulliganNumber = 0;
    }

    currentContext = MessageContext_None;
}

void MessageLogWidget::containerProcessingStarted(const GameEventContext &context)
{
    if (context.HasExtension(Context_MoveCard::ext))
        currentContext = MessageContext_MoveCard;
    else if (context.HasExtension(Context_Mulligan::ext)) {
        const Context_Mulligan &contextMulligan = context.GetExtension(Context_Mulligan::ext);
        currentContext = MessageContext_Mulligan;
        mulliganPlayer = nullptr;
        mulliganNumber = contextMulligan.number();
    }
}

void MessageLogWidget::logAlwaysRevealTopCard(Player *player, CardZone *zone, bool reveal)
{
    appendHtmlServerMessage((reveal ? tr("%1 is now keeping the top card %2 revealed.")
                                    : tr("%1 is not revealing the top card %2 any longer."))
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
    if (sideboardSize < 0)
        appendHtmlServerMessage(tr("%1 has loaded a deck (%2).").arg(sanitizeHtml(player->getName())).arg(deckHash));
    else
        appendHtmlServerMessage(tr("%1 has loaded a deck with %2 sideboard cards (%3).")
                                    .arg(sanitizeHtml(player->getName()))
                                    .arg("<font color=\"blue\">" + QString::number(sideboardSize) + "</font>")
                                    .arg(deckHash));
}

void MessageLogWidget::logDestroyCard(Player *player, QString cardName)
{
    appendHtmlServerMessage(
        tr("%1 destroys %2.").arg(sanitizeHtml(player->getName())).arg(cardLink(std::move(cardName))));
}

void MessageLogWidget::logDoMoveCard(LogMoveCard &lmc)
{
    QString startZone = lmc.startZone->getName();
    QString targetZone = lmc.targetZone->getName();
    bool ownerChanged = lmc.startZone->getPlayer() != lmc.targetZone->getPlayer();

    // do not log if moved within the same zone
    if ((startZone == tableConstant() && targetZone == tableConstant() && !ownerChanged) ||
        (startZone == handConstant() && targetZone == handConstant()) ||
        (startZone == exileConstant() && targetZone == exileConstant()))
        return;

    QString cardName = lmc.cardName;
    QPair<QString, QString> nameFrom = getFromStr(lmc.startZone, cardName, lmc.oldX, ownerChanged);
    if (!nameFrom.first.isEmpty())
        cardName = nameFrom.first;

    QString cardStr;
    if (!nameFrom.first.isEmpty())
        cardStr = cardName;
    else if (cardName.isEmpty())
        cardStr = tr("a card");
    else
        cardStr = cardLink(cardName);

    if (ownerChanged && (lmc.startZone->getPlayer() == lmc.player)) {
        appendHtmlServerMessage(tr("%1 gives %2 control over %3.")
                                    .arg(sanitizeHtml(lmc.player->getName()))
                                    .arg(sanitizeHtml(lmc.targetZone->getPlayer()->getName()))
                                    .arg(cardStr));
        return;
    }

    QString finalStr;
    bool usesNewX = false;
    if (targetZone == tableConstant()) {
        soundEngine->playSound("play_card");
        if (moveCardTapped.value(lmc.card))
            finalStr = tr("%1 puts %2 into play tapped%3.");
        else
            finalStr = tr("%1 puts %2 into play%3.");
    } else if (targetZone == graveyardConstant())
        finalStr = tr("%1 puts %2%3 into their graveyard.");
    else if (targetZone == exileConstant())
        finalStr = tr("%1 exiles %2%3.");
    else if (targetZone == handConstant())
        finalStr = tr("%1 moves %2%3 to their hand.");
    else if (targetZone == deckConstant()) {
        if (moveCardExtras.contains("shuffle_partial")) {
            finalStr = tr("%1 puts %2%3 on bottom of their library randomly.");
        } else if (lmc.newX == -1) {
            finalStr = tr("%1 puts %2%3 into their library.");
        } else if (lmc.newX == lmc.targetZone->getCards().size() - 1) {
            finalStr = tr("%1 puts %2%3 on bottom of their library.");
        } else if (lmc.newX == 0) {
            finalStr = tr("%1 puts %2%3 on top of their library.");
        } else {
            lmc.newX++;
            usesNewX = true;
            finalStr = tr("%1 puts %2%3 into their library %4 cards from the top.");
        }
    } else if (targetZone == sideboardConstant())
        finalStr = tr("%1 moves %2%3 to sideboard.");
    else if (targetZone == stackConstant()) {
        soundEngine->playSound("play_card");
        finalStr = tr("%1 plays %2%3.");
    }

    if (usesNewX) {
        appendHtmlServerMessage(
            finalStr.arg(sanitizeHtml(lmc.player->getName())).arg(cardStr).arg(nameFrom.second).arg(lmc.newX));
    } else {
        appendHtmlServerMessage(finalStr.arg(sanitizeHtml(lmc.player->getName())).arg(cardStr).arg(nameFrom.second));
    }
}

void MessageLogWidget::logDrawCards(Player *player, int number)
{
    if (currentContext == MessageContext_Mulligan)
        mulliganPlayer = player;
    else {
        soundEngine->playSound("draw_card");
        appendHtmlServerMessage(tr("%1 draws %2 card(s).", "", number)
                                    .arg(sanitizeHtml(player->getName()))
                                    .arg("<font color=\"blue\">" + QString::number(number) + "</font>"));
    }
}

void MessageLogWidget::logDumpZone(Player *player, CardZone *zone, int numberCards)
{
    if (numberCards == -1)
        appendHtmlServerMessage(tr("%1 is looking at %2.")
                                    .arg(sanitizeHtml(player->getName()))
                                    .arg(zone->getTranslatedName(zone->getPlayer() == player, CaseLookAtZone)));
    else
        appendHtmlServerMessage(
            tr("%1 is looking at the top %3 card(s) %2.", "top card for singular, top %3 cards for plural", numberCards)
                .arg(sanitizeHtml(player->getName()))
                .arg(zone->getTranslatedName(zone->getPlayer() == player, CaseTopCardsOfZone))
                .arg("<font color=\"blue\">" + QString::number(numberCards) + "</font>"));
}

void MessageLogWidget::logFlipCard(Player *player, QString cardName, bool faceDown)
{
    if (faceDown) {
        appendHtmlServerMessage(tr("%1 turns %2 face-down.").arg(sanitizeHtml(player->getName())).arg(cardName));
    } else {
        appendHtmlServerMessage(tr("%1 turns %2 face-up.").arg(sanitizeHtml(player->getName())).arg(cardName));
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

void MessageLogWidget::logMoveCard(Player *player,
                                   CardItem *card,
                                   CardZone *startZone,
                                   int oldX,
                                   CardZone *targetZone,
                                   int newX)
{
    LogMoveCard attributes = {player, card, card->getName(), startZone, oldX, targetZone, newX};
    if (currentContext == MessageContext_MoveCard)
        moveCardQueue.append(attributes);
    else if (currentContext == MessageContext_Mulligan)
        mulliganPlayer = player;
    else
        logDoMoveCard(attributes);
}

void MessageLogWidget::logMulligan(Player *player, int number)
{
    if (!player)
        return;
    if (number > -1)
        appendHtmlServerMessage(tr("%1 takes a mulligan to %2.").arg(sanitizeHtml(player->getName())).arg(number));
    else
        appendHtmlServerMessage(tr("%1 draws their initial hand.").arg(sanitizeHtml(player->getName())));
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
                                      int amount)
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
                          .arg("<font color=\"blue\">" + QString::number(amount) + "</font>");
        }
    } else {
        cardStr = cardLink(cardName);
    }
    if (cardId == -1) {
        if (otherPlayer) {
            appendHtmlServerMessage(tr("%1 reveals %2 to %3.")
                                        .arg(sanitizeHtml(player->getName()))
                                        .arg(zone->getTranslatedName(true, CaseRevealZone))
                                        .arg(sanitizeHtml(otherPlayer->getName())));
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

void MessageLogWidget::logRollDie(Player *player, int sides, int roll)
{
    if (sides == 2) {
        QString coinOptions[2] = {tr("Heads") + " (1)", tr("Tails") + " (2)"};
        appendHtmlServerMessage(tr("%1 flipped a coin. It landed as %2.")
                                    .arg(sanitizeHtml(player->getName()))
                                    .arg("<font color=\"blue\">" + coinOptions[roll - 1] + "</font>"));
    } else
        appendHtmlServerMessage(tr("%1 rolls a %2 with a %3-sided die.")
                                    .arg(sanitizeHtml(player->getName()))
                                    .arg("<font color=\"blue\">" + QString::number(roll) + "</font>")
                                    .arg("<font color=\"blue\">" + QString::number(sides) + "</font>"));
    soundEngine->playSound("roll_dice");
}

void MessageLogWidget::logSay(Player *player, QString message)
{
    appendMessage(std::move(message), nullptr, player->getName(), UserLevelFlags(player->getUserInfo()->user_level()),
                  QString::fromStdString(player->getUserInfo()->privlevel()), true);
}

void MessageLogWidget::logSetActivePhase(int phase)
{
    QString phaseName;
    QString color;
    switch (phase) {
        case 0:
            phaseName = tr("Untap");
            soundEngine->playSound("untap_step");
            color = "green";
            break;
        case 1:
            phaseName = tr("Upkeep");
            soundEngine->playSound("upkeep_step");
            color = "green";
            break;
        case 2:
            phaseName = tr("Draw");
            soundEngine->playSound("draw_step");
            color = "green";
            break;
        case 3:
            phaseName = tr("First Main");
            soundEngine->playSound("main_1");
            color = "blue";
            break;
        case 4:
            phaseName = tr("Beginning of Combat");
            soundEngine->playSound("start_combat");
            color = "red";
            break;
        case 5:
            phaseName = tr("Declare Attackers");
            soundEngine->playSound("attack_step");
            color = "red";
            break;
        case 6:
            phaseName = tr("Declare Blockers");
            soundEngine->playSound("block_step");
            color = "red";
            break;
        case 7:
            phaseName = tr("Combat Damage");
            soundEngine->playSound("damage_step");
            color = "red";
            break;
        case 8:
            phaseName = tr("End of Combat");
            soundEngine->playSound("end_combat");
            color = "red";
            break;
        case 9:
            phaseName = tr("Second Main");
            soundEngine->playSound("main_2");
            color = "blue";
            break;
        case 10:
            phaseName = tr("End/Cleanup");
            soundEngine->playSound("end_step");
            color = "green";
            break;
        default:
            phaseName = tr("Unknown Phase");
            color = "black";
            break;
    }
    appendHtml("<font color=\"" + color + "\"><b>" + QDateTime::currentDateTime().toString("[hh:mm:ss] ") +
               QString("%1").arg(phaseName) + "</b></font>");
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
            .arg(QString("&quot;<font color=\"blue\">%1</font>&quot;").arg(sanitizeHtml(std::move(newAnnotation)))));
}

void MessageLogWidget::logSetCardCounter(Player *player, QString cardName, int counterId, int value, int oldValue)
{
    QString finalStr;
    int delta = abs(oldValue - value);
    if (value > oldValue)
        finalStr = tr("%1 places %2 %3 on %4 (now %5).");
    else
        finalStr = tr("%1 removes %2 %3 from %4 (now %5).");

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
                                .arg("<font color=\"blue\">" + QString::number(delta) + "</font>")
                                .arg(colorStr)
                                .arg(cardLink(std::move(cardName)))
                                .arg(value));
}

void MessageLogWidget::logSetCounter(Player *player, QString counterName, int value, int oldValue)
{
    if (counterName == "life")
        soundEngine->playSound("life_change");

    appendHtmlServerMessage(tr("%1 sets counter %2 to %3 (%4%5).")
                                .arg(sanitizeHtml(player->getName()))
                                .arg(QString("<font color=\"blue\">%1</font>").arg(sanitizeHtml(counterName)))
                                .arg(QString("<font color=\"blue\">%1</font>").arg(value))
                                .arg(value > oldValue ? "+" : "")
                                .arg(value - oldValue));
}

void MessageLogWidget::logSetDoesntUntap(Player *player, CardItem *card, bool doesntUntap)
{
    QString str;
    if (doesntUntap)
        str = tr("%1 sets %2 to not untap normally.");
    else
        str = tr("%1 sets %2 to untap normally.");
    appendHtmlServerMessage(str.arg(sanitizeHtml(player->getName())).arg(cardLink(card->getName())));
}

void MessageLogWidget::logSetPT(Player *player, CardItem *card, QString newPT)
{
    if (currentContext == MessageContext_MoveCard) {
        return;
    }

    QString name = card->getName();
    if (name.isEmpty()) {
        name = QString("<font color=\"blue\">card #%1</font>").arg(sanitizeHtml(QString::number(card->getId())));
    } else {
        name = cardLink(name);
    }
    if (newPT.isEmpty()) {
        appendHtmlServerMessage(tr("%1 removes the PT of %2.").arg(sanitizeHtml(player->getName())).arg(name));
    } else {
        appendHtmlServerMessage(
            tr("%1 sets PT of %2 to %3.").arg(sanitizeHtml(player->getName())).arg(name).arg(newPT));
    }
}

void MessageLogWidget::logSetSideboardLock(Player *player, bool locked)
{
    if (locked)
        appendHtmlServerMessage(tr("%1 has locked their sideboard.").arg(sanitizeHtml(player->getName())));
    else
        appendHtmlServerMessage(tr("%1 has unlocked their sideboard.").arg(sanitizeHtml(player->getName())));
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
        if (!card)
            appendHtmlServerMessage((tapped ? tr("%1 taps their permanents.") : tr("%1 untaps their permanents."))
                                        .arg(sanitizeHtml(player->getName())));
        else
            appendHtmlServerMessage((tapped ? tr("%1 taps %2.") : tr("%1 untaps %2."))
                                        .arg(sanitizeHtml(player->getName()))
                                        .arg(cardLink(card->getName())));
    }
}

void MessageLogWidget::logShuffle(Player *player, CardZone *zone, int start, int end)
{
    soundEngine->playSound("shuffle");
    if (currentContext == MessageContext_Mulligan) {
        return;
    }

    if (currentContext == MessageContext_MoveCard && start == 0 && end == -1) {
        moveCardExtras.append("shuffle_partial");
        return;
    }

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

void MessageLogWidget::logSpectatorSay(QString spectatorName,
                                       UserLevelFlags spectatorUserLevel,
                                       QString userPrivLevel,
                                       QString message)
{
    appendMessage(std::move(message), nullptr, spectatorName, spectatorUserLevel, userPrivLevel, false);
}

void MessageLogWidget::logStopDumpZone(Player *player, CardZone *zone)
{
    appendHtmlServerMessage(tr("%1 stops looking at %2.")
                                .arg(sanitizeHtml(player->getName()))
                                .arg(zone->getTranslatedName(zone->getPlayer() == player, CaseLookAtZone)));
}

void MessageLogWidget::logUnattachCard(Player *player, QString cardName)
{
    appendHtmlServerMessage(
        tr("%1 unattaches %2.").arg(sanitizeHtml(player->getName())).arg(cardLink(std::move(cardName))));
}

void MessageLogWidget::logUndoDraw(Player *player, QString cardName)
{
    if (cardName.isEmpty())
        appendHtmlServerMessage(tr("%1 undoes their last draw.").arg(sanitizeHtml(player->getName())));
    else
        appendHtmlServerMessage(
            tr("%1 undoes their last draw (%2).")
                .arg(sanitizeHtml(player->getName()))
                .arg(QString("<a href=\"card://%1\">%2</a>").arg(sanitizeHtml(cardName)).arg(sanitizeHtml(cardName))));
}

void MessageLogWidget::connectToPlayer(Player *player)
{
    connect(player, SIGNAL(logSay(Player *, QString)), this, SLOT(logSay(Player *, QString)));
    connect(player, &Player::logShuffle, this, &MessageLogWidget::logShuffle);
    connect(player, SIGNAL(logRollDie(Player *, int, int)), this, SLOT(logRollDie(Player *, int, int)));
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
    connect(player, SIGNAL(logDumpZone(Player *, CardZone *, int)), this, SLOT(logDumpZone(Player *, CardZone *, int)));
    connect(player, SIGNAL(logStopDumpZone(Player *, CardZone *)), this, SLOT(logStopDumpZone(Player *, CardZone *)));
    connect(player, SIGNAL(logDrawCards(Player *, int)), this, SLOT(logDrawCards(Player *, int)));
    connect(player, SIGNAL(logUndoDraw(Player *, QString)), this, SLOT(logUndoDraw(Player *, QString)));
    connect(player, SIGNAL(logRevealCards(Player *, CardZone *, int, QString, Player *, bool, int)), this,
            SLOT(logRevealCards(Player *, CardZone *, int, QString, Player *, bool, int)));
    connect(player, SIGNAL(logAlwaysRevealTopCard(Player *, CardZone *, bool)), this,
            SLOT(logAlwaysRevealTopCard(Player *, CardZone *, bool)));
}

MessageLogWidget::MessageLogWidget(const TabSupervisor *_tabSupervisor,
                                   const UserlistProxy *_userlistProxy,
                                   TabGame *_game,
                                   QWidget *parent)
    : ChatView(_tabSupervisor, _userlistProxy, _game, true, parent), mulliganNumber(0),
      currentContext(MessageContext_None)
{
}
