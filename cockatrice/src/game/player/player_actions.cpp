#include "player_actions.h"

#include "../../interface/widgets/tabs/tab_game.h"
#include "../../interface/widgets/utility/get_text_with_max.h"
#include "../board/card_item.h"
#include "../dialogs/dlg_move_top_cards_until.h"
#include "../dialogs/dlg_roll_dice.h"
#include "../zones/hand_zone.h"
#include "../zones/logic/view_zone_logic.h"
#include "../zones/table_zone.h"
#include "card_menu_action_type.h"

#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/card/relation/card_relation.h>
#include <libcockatrice/protocol/pb/command_attach_card.pb.h>
#include <libcockatrice/protocol/pb/command_change_zone_properties.pb.h>
#include <libcockatrice/protocol/pb/command_create_token.pb.h>
#include <libcockatrice/protocol/pb/command_draw_cards.pb.h>
#include <libcockatrice/protocol/pb/command_flip_card.pb.h>
#include <libcockatrice/protocol/pb/command_game_say.pb.h>
#include <libcockatrice/protocol/pb/command_move_card.pb.h>
#include <libcockatrice/protocol/pb/command_mulligan.pb.h>
#include <libcockatrice/protocol/pb/command_reveal_cards.pb.h>
#include <libcockatrice/protocol/pb/command_roll_die.pb.h>
#include <libcockatrice/protocol/pb/command_set_card_attr.pb.h>
#include <libcockatrice/protocol/pb/command_set_card_counter.pb.h>
#include <libcockatrice/protocol/pb/command_shuffle.pb.h>
#include <libcockatrice/protocol/pb/command_undo_draw.pb.h>
#include <libcockatrice/protocol/pb/context_move_card.pb.h>
#include <libcockatrice/utility/trice_limits.h>

// milliseconds in between triggers of the move top cards until action
static constexpr int MOVE_TOP_CARD_UNTIL_INTERVAL = 100;

PlayerActions::PlayerActions(Player *_player) : player(_player), lastTokenTableRow(0), movingCardsUntil(false)
{
    moveTopCardTimer = new QTimer(this);
    moveTopCardTimer->setInterval(MOVE_TOP_CARD_UNTIL_INTERVAL);
    moveTopCardTimer->setSingleShot(true);
    connect(moveTopCardTimer, &QTimer::timeout, [this]() { actMoveTopCardToPlay(); });
}

void PlayerActions::playCard(CardItem *card, bool faceDown)
{
    if (card == nullptr) {
        return;
    }

    Command_MoveCard cmd;
    cmd.set_start_player_id(card->getZone()->getPlayer()->getPlayerInfo()->getId());
    cmd.set_start_zone(card->getZone()->getName().toStdString());
    cmd.set_target_player_id(player->getPlayerInfo()->getId());
    CardToMove *cardToMove = cmd.mutable_cards_to_move()->add_card();
    cardToMove->set_card_id(card->getId());

    ExactCard exactCard = card->getCard();
    if (!exactCard) {
        return;
    }

    const CardInfo &info = exactCard.getInfo();

    int tableRow = info.getUiAttributes().tableRow;
    bool playToStack = SettingsCache::instance().getPlayToStack();
    QString currentZone = card->getZone()->getName();
    if (currentZone == "stack" && tableRow == 3) {
        cmd.set_target_zone("grave");
        cmd.set_x(0);
        cmd.set_y(0);
    } else if (!faceDown &&
               ((!playToStack && tableRow == 3) || ((playToStack && tableRow != 0) && currentZone != "stack"))) {
        cmd.set_target_zone("stack");
        cmd.set_x(-1);
        cmd.set_y(0);
    } else {
        tableRow = faceDown ? 2 : info.getUiAttributes().tableRow;
        QPoint gridPoint = QPoint(-1, TableZone::clampValidTableRow(2 - tableRow));
        cardToMove->set_face_down(faceDown);
        if (!faceDown) {
            cardToMove->set_pt(info.getPowTough().toStdString());
        }
        cardToMove->set_tapped(!faceDown && info.getUiAttributes().cipt);
        if (tableRow != 3)
            cmd.set_target_zone("table");
        cmd.set_x(gridPoint.x());
        cmd.set_y(gridPoint.y());
    }
    sendGameCommand(cmd);
}

/**
 * Like {@link PlayerActions::playCard}, but forces the card to be played to the table zone.
 * Cards with tablerow 3 (the stack) will be played to tablerow 1 (the noncreatures row).
 */
void PlayerActions::playCardToTable(const CardItem *card, bool faceDown)
{
    if (card == nullptr) {
        return;
    }

    Command_MoveCard cmd;
    cmd.set_start_player_id(card->getZone()->getPlayer()->getPlayerInfo()->getId());
    cmd.set_start_zone(card->getZone()->getName().toStdString());
    cmd.set_target_player_id(player->getPlayerInfo()->getId());
    CardToMove *cardToMove = cmd.mutable_cards_to_move()->add_card();
    cardToMove->set_card_id(card->getId());

    ExactCard exactCard = card->getCard();
    if (!exactCard) {
        return;
    }

    const CardInfo &info = exactCard.getInfo();

    int tableRow = faceDown ? 2 : info.getUiAttributes().tableRow;
    // default instant/sorcery cards to the noncreatures row
    if (tableRow > 2) {
        tableRow = 1;
    }

    QPoint gridPoint = QPoint(-1, TableZone::clampValidTableRow(2 - tableRow));
    cardToMove->set_face_down(faceDown);
    if (!faceDown) {
        cardToMove->set_pt(info.getPowTough().toStdString());
    }
    cardToMove->set_tapped(!faceDown && info.getUiAttributes().cipt);
    cmd.set_target_zone("table");
    cmd.set_x(gridPoint.x());
    cmd.set_y(gridPoint.y());
    sendGameCommand(cmd);
}

void PlayerActions::actViewLibrary()
{
    player->getGameScene()->toggleZoneView(player, "deck", -1);
}

void PlayerActions::actViewHand()
{
    player->getGameScene()->toggleZoneView(player, "hand", -1);
}

/**
 * @brief The sortHand actions only pass along a single SortOption in its data.
 * This method fills out the rest of the sort priority list given that option.
 * @param option The single sort option
 * @return The sort priority list
 */
static QList<CardList::SortOption> expandSortOption(CardList::SortOption option)
{
    switch (option) {
        case CardList::SortByName:
            return {};
        case CardList::SortByMainType:
            return {CardList::SortByMainType, CardList::SortByManaValue};
        case CardList::SortByManaValue:
            return {CardList::SortByManaValue, CardList::SortByColors};
        default:
            return {option};
    }
}

void PlayerActions::actSortHand()
{
    auto *action = qobject_cast<QAction *>(sender());
    CardList::SortOption option = static_cast<CardList::SortOption>(action->data().toInt());

    QList<CardList::SortOption> sortOptions = expandSortOption(option);

    static QList defaultOptions = {CardList::SortByName, CardList::SortByPrinting};

    player->getGraphicsItem()->getHandZoneGraphicsItem()->sortHand(sortOptions + defaultOptions);
}

void PlayerActions::actViewTopCards()
{
    int deckSize = player->getDeckZone()->getCards().size();
    bool ok;
    int number = QInputDialog::getInt(player->getGame()->getTab(), tr("View top cards of library"),
                                      tr("Number of cards: (max. %1)").arg(deckSize), defaultNumberTopCards, 1,
                                      deckSize, 1, &ok);
    if (ok) {
        defaultNumberTopCards = number;
        player->getGameScene()->toggleZoneView(player, "deck", number);
    }
}

void PlayerActions::actViewBottomCards()
{
    int deckSize = player->getDeckZone()->getCards().size();
    bool ok;
    int number = QInputDialog::getInt(player->getGame()->getTab(), tr("View bottom cards of library"),
                                      tr("Number of cards: (max. %1)").arg(deckSize), defaultNumberBottomCards, 1,
                                      deckSize, 1, &ok);
    if (ok) {
        defaultNumberBottomCards = number;
        player->getGameScene()->toggleZoneView(player, "deck", number, true);
    }
}

void PlayerActions::actAlwaysRevealTopCard()
{
    Command_ChangeZoneProperties cmd;
    cmd.set_zone_name("deck");
    cmd.set_always_reveal_top_card(player->getPlayerMenu()->getLibraryMenu()->isAlwaysRevealTopCardChecked());

    sendGameCommand(cmd);
}

void PlayerActions::actAlwaysLookAtTopCard()
{
    Command_ChangeZoneProperties cmd;
    cmd.set_zone_name("deck");
    cmd.set_always_look_at_top_card(player->getPlayerMenu()->getLibraryMenu()->isAlwaysLookAtTopCardChecked());

    sendGameCommand(cmd);
}

void PlayerActions::actOpenDeckInDeckEditor()
{
    emit player->openDeckEditor({.deckList = player->getDeck()});
}

void PlayerActions::actViewGraveyard()
{
    player->getGameScene()->toggleZoneView(player, "grave", -1);
}

void PlayerActions::actViewRfg()
{
    player->getGameScene()->toggleZoneView(player, "rfg", -1);
}

void PlayerActions::actViewSideboard()
{
    player->getGameScene()->toggleZoneView(player, "sb", -1);
}

void PlayerActions::actShuffle()
{
    sendGameCommand(Command_Shuffle());
}

void PlayerActions::actShuffleTop()
{
    const int maxCards = player->getDeckZone()->getCards().size();
    if (maxCards == 0) {
        return;
    }

    bool ok;
    int number = QInputDialog::getInt(player->getGame()->getTab(), tr("Shuffle top cards of library"),
                                      tr("Number of cards: (max. %1)").arg(maxCards), defaultNumberTopCards, 1,
                                      maxCards, 1, &ok);
    if (!ok) {
        return;
    }

    if (number > maxCards) {
        number = maxCards;
    }

    defaultNumberTopCards = number;

    Command_Shuffle cmd;
    cmd.set_zone_name("deck");
    cmd.set_start(0);
    cmd.set_end(number - 1); // inclusive, the indexed card at end will be shuffled

    sendGameCommand(cmd);
}

void PlayerActions::actShuffleBottom()
{
    const int maxCards = player->getDeckZone()->getCards().size();
    if (maxCards == 0) {
        return;
    }

    bool ok;
    int number = QInputDialog::getInt(player->getGame()->getTab(), tr("Shuffle bottom cards of library"),
                                      tr("Number of cards: (max. %1)").arg(maxCards), defaultNumberBottomCards, 1,
                                      maxCards, 1, &ok);
    if (!ok) {
        return;
    }

    if (number > maxCards) {
        number = maxCards;
    }

    defaultNumberBottomCards = number;

    Command_Shuffle cmd;
    cmd.set_zone_name("deck");
    cmd.set_start(-number);
    cmd.set_end(-1);

    sendGameCommand(cmd);
}

void PlayerActions::actDrawCard()
{
    Command_DrawCards cmd;
    cmd.set_number(1);
    sendGameCommand(cmd);
}

void PlayerActions::actMulligan()
{
    int startSize = SettingsCache::instance().getStartingHandSize();
    int handSize = player->getHandZone()->getCards().size();
    int deckSize = player->getDeckZone()->getCards().size() + handSize;

    bool ok;
    int number = QInputDialog::getInt(player->getGame()->getTab(), tr("Draw hand"),
                                      tr("Number of cards: (max. %1)").arg(deckSize) + '\n' +
                                          tr("0 and lower are in comparison to current hand size"),
                                      startSize, -handSize, deckSize, 1, &ok);

    if (!ok) {
        return;
    }

    if (number < 1) {
        number = handSize + number;
    }

    doMulligan(number);
    SettingsCache::instance().setStartingHandSize(number);
}

void PlayerActions::actMulliganSameSize()
{
    int handSize = player->getHandZone()->getCards().size();
    doMulligan(handSize);
}

void PlayerActions::actMulliganMinusOne()
{
    int handSize = player->getHandZone()->getCards().size();
    int targetSize = qMax(1, handSize - 1);
    doMulligan(targetSize);
}

void PlayerActions::doMulligan(int number)
{
    if (number < 1) {
        return;
    }

    Command_Mulligan cmd;
    cmd.set_number(number);
    sendGameCommand(cmd);
}

void PlayerActions::actDrawCards()
{
    int deckSize = player->getDeckZone()->getCards().size();
    bool ok;
    int number = QInputDialog::getInt(player->getGame()->getTab(), tr("Draw cards"),
                                      tr("Number of cards: (max. %1)").arg(deckSize), defaultNumberTopCards, 1,
                                      deckSize, 1, &ok);
    if (ok) {
        defaultNumberTopCards = number;
        Command_DrawCards cmd;
        cmd.set_number(static_cast<google::protobuf::uint32>(number));
        sendGameCommand(cmd);
    }
}

void PlayerActions::actUndoDraw()
{
    sendGameCommand(Command_UndoDraw());
}

void PlayerActions::cmdSetTopCard(Command_MoveCard &cmd)
{
    cmd.set_start_zone("deck");
    auto *cardToMove = cmd.mutable_cards_to_move()->add_card();
    cardToMove->set_card_id(0);
    cmd.set_target_player_id(player->getPlayerInfo()->getId());
}

void PlayerActions::cmdSetBottomCard(Command_MoveCard &cmd)
{
    CardZoneLogic *zone = player->getDeckZone();
    int lastCard = zone->getCards().size() - 1;
    cmd.set_start_zone("deck");
    auto *cardToMove = cmd.mutable_cards_to_move()->add_card();
    cardToMove->set_card_id(lastCard);
    cmd.set_target_player_id(player->getPlayerInfo()->getId());
}

void PlayerActions::actMoveTopCardToGrave()
{
    if (player->getDeckZone()->getCards().empty()) {
        return;
    }

    Command_MoveCard cmd;
    cmdSetTopCard(cmd);
    cmd.set_target_zone("grave");
    cmd.set_x(0);
    cmd.set_y(0);

    sendGameCommand(cmd);
}

void PlayerActions::actMoveTopCardToExile()
{
    if (player->getDeckZone()->getCards().empty()) {
        return;
    }

    Command_MoveCard cmd;
    cmdSetTopCard(cmd);
    cmd.set_target_zone("rfg");
    cmd.set_x(0);
    cmd.set_y(0);

    sendGameCommand(cmd);
}

void PlayerActions::actMoveTopCardsToGrave()
{
    const int maxCards = player->getDeckZone()->getCards().size();
    if (maxCards == 0) {
        return;
    }

    bool ok;
    int number = QInputDialog::getInt(player->getGame()->getTab(), tr("Move top cards to grave"),
                                      tr("Number of cards: (max. %1)").arg(maxCards), defaultNumberTopCards, 1,
                                      maxCards, 1, &ok);
    if (!ok) {
        return;
    } else if (number > maxCards) {
        number = maxCards;
    }
    defaultNumberTopCards = number;

    Command_MoveCard cmd;
    cmd.set_start_zone("deck");
    cmd.set_target_player_id(player->getPlayerInfo()->getId());
    cmd.set_target_zone("grave");
    cmd.set_x(0);
    cmd.set_y(0);

    for (int i = number - 1; i >= 0; --i) {
        cmd.mutable_cards_to_move()->add_card()->set_card_id(i);
    }

    sendGameCommand(cmd);
}

void PlayerActions::actMoveTopCardsToExile()
{
    const int maxCards = player->getDeckZone()->getCards().size();
    if (maxCards == 0) {
        return;
    }

    bool ok;
    int number = QInputDialog::getInt(player->getGame()->getTab(), tr("Move top cards to exile"),
                                      tr("Number of cards: (max. %1)").arg(maxCards), defaultNumberTopCards, 1,
                                      maxCards, 1, &ok);
    if (!ok) {
        return;
    } else if (number > maxCards) {
        number = maxCards;
    }
    defaultNumberTopCards = number;

    Command_MoveCard cmd;
    cmd.set_start_zone("deck");
    cmd.set_target_player_id(player->getPlayerInfo()->getId());
    cmd.set_target_zone("rfg");
    cmd.set_x(0);
    cmd.set_y(0);

    for (int i = number - 1; i >= 0; --i) {
        cmd.mutable_cards_to_move()->add_card()->set_card_id(i);
    }

    sendGameCommand(cmd);
}

void PlayerActions::actMoveTopCardsUntil()
{
    stopMoveTopCardsUntil();

    DlgMoveTopCardsUntil dlg(player->getGame()->getTab(), movingCardsUntilExprs, movingCardsUntilNumberOfHits,
                             movingCardsUntilAutoPlay);
    if (!dlg.exec()) {
        return;
    }

    auto expr = dlg.getExpr();
    movingCardsUntilExprs = dlg.getExprs();
    movingCardsUntilNumberOfHits = dlg.getNumberOfHits();
    movingCardsUntilAutoPlay = dlg.isAutoPlay();

    if (player->getDeckZone()->getCards().empty()) {
        stopMoveTopCardsUntil();
    } else {
        movingCardsUntilFilter = FilterString(expr);
        movingCardsUntilCounter = movingCardsUntilNumberOfHits;
        movingCardsUntil = true;
        actMoveTopCardToPlay();
    }
}

void PlayerActions::moveOneCardUntil(CardItem *card)
{
    moveTopCardTimer->stop();

    const bool isMatch = card && movingCardsUntilFilter.check(card->getCard().getCardPtr());

    if (isMatch && movingCardsUntilAutoPlay) {
        // Directly calling playCard will deadlock, since we are already in the middle of processing an event.
        // Use QTimer::singleShot to queue up the playCard on the event loop.
        QTimer::singleShot(0, this, [card, this] { playCard(card, false); });
    }

    if (player->getDeckZone()->getCards().empty() || !card) {
        stopMoveTopCardsUntil();
    } else if (isMatch) {
        --movingCardsUntilCounter;
        if (movingCardsUntilCounter > 0) {
            moveTopCardTimer->start();
        } else {
            stopMoveTopCardsUntil();
        }
    } else {
        moveTopCardTimer->start();
    }
}

/**
 * @brief Immediately stops any ongoing `play top card to stack until...` process, resetting all variables involved.
 */
void PlayerActions::stopMoveTopCardsUntil()
{
    moveTopCardTimer->stop();
    movingCardsUntilCounter = 0;
    movingCardsUntil = false;
}

void PlayerActions::actMoveTopCardToBottom()
{
    if (player->getDeckZone()->getCards().empty()) {
        return;
    }

    Command_MoveCard cmd;
    cmdSetTopCard(cmd);
    cmd.set_target_zone("deck");
    cmd.set_x(-1); // bottom of deck
    cmd.set_y(0);

    sendGameCommand(cmd);
}

void PlayerActions::actMoveTopCardToPlay()
{
    if (player->getDeckZone()->getCards().empty()) {
        return;
    }

    Command_MoveCard cmd;
    cmdSetTopCard(cmd);
    cmd.set_target_zone("stack");
    cmd.set_x(-1);
    cmd.set_y(0);

    sendGameCommand(cmd);
}

void PlayerActions::actMoveTopCardToPlayFaceDown()
{
    if (player->getDeckZone()->getCards().empty()) {
        return;
    }

    Command_MoveCard cmd;
    cmd.set_start_zone("deck");
    CardToMove *cardToMove = cmd.mutable_cards_to_move()->add_card();
    cardToMove->set_card_id(0);
    cardToMove->set_face_down(true);
    cmd.set_target_player_id(player->getPlayerInfo()->getId());
    cmd.set_target_zone("table");
    cmd.set_x(-1);
    cmd.set_y(0);

    sendGameCommand(cmd);
}

void PlayerActions::actMoveBottomCardToGrave()
{
    if (player->getDeckZone()->getCards().empty()) {
        return;
    }

    Command_MoveCard cmd;
    cmdSetBottomCard(cmd);
    cmd.set_target_zone("grave");
    cmd.set_x(0);
    cmd.set_y(0);

    sendGameCommand(cmd);
}

void PlayerActions::actMoveBottomCardToExile()
{
    if (player->getDeckZone()->getCards().empty()) {
        return;
    }

    Command_MoveCard cmd;
    cmdSetBottomCard(cmd);
    cmd.set_target_zone("rfg");
    cmd.set_x(0);
    cmd.set_y(0);

    sendGameCommand(cmd);
}

void PlayerActions::actMoveBottomCardsToGrave()
{
    const int maxCards = player->getDeckZone()->getCards().size();
    if (maxCards == 0) {
        return;
    }

    bool ok;
    int number = QInputDialog::getInt(player->getGame()->getTab(), tr("Move bottom cards to grave"),
                                      tr("Number of cards: (max. %1)").arg(maxCards), defaultNumberBottomCards, 1,
                                      maxCards, 1, &ok);
    if (!ok) {
        return;
    } else if (number > maxCards) {
        number = maxCards;
    }
    defaultNumberBottomCards = number;

    Command_MoveCard cmd;
    cmd.set_start_zone("deck");
    cmd.set_target_player_id(player->getPlayerInfo()->getId());
    cmd.set_target_zone("grave");
    cmd.set_x(0);
    cmd.set_y(0);

    for (int i = maxCards - number; i < maxCards; ++i) {
        cmd.mutable_cards_to_move()->add_card()->set_card_id(i);
    }

    sendGameCommand(cmd);
}

void PlayerActions::actMoveBottomCardsToExile()
{
    const int maxCards = player->getDeckZone()->getCards().size();
    if (maxCards == 0) {
        return;
    }

    bool ok;
    int number = QInputDialog::getInt(player->getGame()->getTab(), tr("Move bottom cards to exile"),
                                      tr("Number of cards: (max. %1)").arg(maxCards), defaultNumberBottomCards, 1,
                                      maxCards, 1, &ok);
    if (!ok) {
        return;
    } else if (number > maxCards) {
        number = maxCards;
    }
    defaultNumberBottomCards = number;

    Command_MoveCard cmd;
    cmd.set_start_zone("deck");
    cmd.set_target_player_id(player->getPlayerInfo()->getId());
    cmd.set_target_zone("rfg");
    cmd.set_x(0);
    cmd.set_y(0);

    for (int i = maxCards - number; i < maxCards; ++i) {
        cmd.mutable_cards_to_move()->add_card()->set_card_id(i);
    }

    sendGameCommand(cmd);
}

void PlayerActions::actMoveBottomCardToTop()
{
    if (player->getDeckZone()->getCards().empty()) {
        return;
    }

    Command_MoveCard cmd;
    cmdSetBottomCard(cmd);
    cmd.set_target_zone("deck");
    cmd.set_x(0); // top of deck
    cmd.set_y(0);

    sendGameCommand(cmd);
}

/**
 * Selects all cards in the given zone.
 *
 * @param zone The zone to select from
 * @param filter A predicate to filter which cards are selected. Defaults to always returning true.
 */
static void selectCardsInZone(
    const CardZoneLogic *zone,
    std::function<bool(const CardItem *)> filter = [](const CardItem *) { return true; })
{
    if (!zone) {
        return;
    }

    for (auto &cardItem : zone->getCards()) {
        if (cardItem && filter(cardItem)) {
            cardItem->setSelected(true);
        }
    }
}

void PlayerActions::actSelectAll()
{
    const CardItem *card = player->getGame()->getActiveCard();
    if (!card) {
        return;
    }

    selectCardsInZone(card->getZone());
}

void PlayerActions::actSelectRow()
{
    const CardItem *card = player->getGame()->getActiveCard();
    if (!card) {
        return;
    }

    auto isSameRow = [card](const CardItem *cardItem) {
        return qAbs(card->scenePos().y() - cardItem->scenePos().y()) < 50;
    };
    selectCardsInZone(card->getZone(), isSameRow);
}

void PlayerActions::actSelectColumn()
{
    const CardItem *card = player->getGame()->getActiveCard();
    if (!card) {
        return;
    }

    auto isSameColumn = [card](const CardItem *cardItem) { return cardItem->x() == card->x(); };
    selectCardsInZone(card->getZone(), isSameColumn);
}

void PlayerActions::actDrawBottomCard()
{
    if (player->getDeckZone()->getCards().empty()) {
        return;
    }

    Command_MoveCard cmd;
    cmdSetBottomCard(cmd);
    cmd.set_target_zone("hand");
    cmd.set_x(0);
    cmd.set_y(0);

    sendGameCommand(cmd);
}

void PlayerActions::actDrawBottomCards()
{
    const int maxCards = player->getDeckZone()->getCards().size();
    if (maxCards == 0) {
        return;
    }

    bool ok;
    int number = QInputDialog::getInt(player->getGame()->getTab(), tr("Draw bottom cards"),
                                      tr("Number of cards: (max. %1)").arg(maxCards), defaultNumberBottomCards, 1,
                                      maxCards, 1, &ok);
    if (!ok) {
        return;
    } else if (number > maxCards) {
        number = maxCards;
    }
    defaultNumberBottomCards = number;

    Command_MoveCard cmd;
    cmd.set_start_zone("deck");
    cmd.set_target_player_id(player->getPlayerInfo()->getId());
    cmd.set_target_zone("hand");
    cmd.set_x(0);
    cmd.set_y(0);

    for (int i = maxCards - number; i < maxCards; ++i) {
        cmd.mutable_cards_to_move()->add_card()->set_card_id(i);
    }

    sendGameCommand(cmd);
}

void PlayerActions::actMoveBottomCardToPlay()
{
    if (player->getDeckZone()->getCards().empty()) {
        return;
    }

    Command_MoveCard cmd;
    cmdSetBottomCard(cmd);
    cmd.set_target_zone("stack");
    cmd.set_x(-1);
    cmd.set_y(0);

    sendGameCommand(cmd);
}

void PlayerActions::actMoveBottomCardToPlayFaceDown()
{
    if (player->getDeckZone()->getCards().empty()) {
        return;
    }

    CardZoneLogic *zone = player->getDeckZone();
    int lastCard = zone->getCards().size() - 1;

    Command_MoveCard cmd;
    cmd.set_start_zone("deck");
    auto *cardToMove = cmd.mutable_cards_to_move()->add_card();
    cardToMove->set_card_id(lastCard);
    cardToMove->set_face_down(true);

    cmd.set_target_player_id(player->getPlayerInfo()->getId());
    cmd.set_target_zone("table");
    cmd.set_x(-1);
    cmd.set_y(0);

    sendGameCommand(cmd);
}

void PlayerActions::actUntapAll()
{
    Command_SetCardAttr cmd;
    cmd.set_zone("table");
    cmd.set_attribute(AttrTapped);
    cmd.set_attr_value("0");

    sendGameCommand(cmd);
}

void PlayerActions::actRollDie()
{
    DlgRollDice dlg(player->getGame()->getTab());
    if (!dlg.exec()) {
        return;
    }

    Command_RollDie cmd;
    cmd.set_sides(dlg.getDieSideCount());
    cmd.set_count(dlg.getDiceToRollCount());
    sendGameCommand(cmd);
}

void PlayerActions::actCreateToken()
{
    DlgCreateToken dlg(player->getPlayerMenu()->getUtilityMenu()->getPredefinedTokens(), player->getGame()->getTab());
    if (!dlg.exec()) {
        return;
    }

    lastTokenInfo = dlg.getTokenInfo();

    ExactCard correctedCard = CardDatabaseManager::query()->guessCard({lastTokenInfo.name, lastTokenInfo.providerId});
    if (correctedCard) {
        lastTokenInfo.name = correctedCard.getName();
        lastTokenTableRow = TableZone::clampValidTableRow(2 - correctedCard.getInfo().getUiAttributes().tableRow);
        if (lastTokenInfo.pt.isEmpty()) {
            lastTokenInfo.pt = correctedCard.getInfo().getPowTough();
        }
    }

    player->getPlayerMenu()->getUtilityMenu()->setAndEnableCreateAnotherTokenAction(
        tr("C&reate another %1 token").arg(lastTokenInfo.name));
    actCreateAnotherToken();
}

void PlayerActions::actCreateAnotherToken()
{
    if (lastTokenInfo.name.isEmpty()) {
        return;
    }

    Command_CreateToken cmd;
    cmd.set_zone("table");
    cmd.set_card_name(lastTokenInfo.name.toStdString());
    cmd.set_card_provider_id(lastTokenInfo.providerId.toStdString());
    cmd.set_color(lastTokenInfo.color.toStdString());
    cmd.set_pt(lastTokenInfo.pt.toStdString());
    cmd.set_annotation(lastTokenInfo.annotation.toStdString());
    cmd.set_destroy_on_zone_change(lastTokenInfo.destroy);
    cmd.set_face_down(lastTokenInfo.faceDown);
    cmd.set_x(-1);
    cmd.set_y(lastTokenTableRow);

    sendGameCommand(cmd);
}

void PlayerActions::setLastToken(CardInfoPtr cardInfo)
{
    if (cardInfo == nullptr) {
        return;
    }

    UtilityMenu *utilityMenu = player->getPlayerMenu()->getUtilityMenu();
    if (utilityMenu == nullptr || !utilityMenu->createAnotherTokenActionExists()) {
        return;
    }

    lastTokenInfo = {.name = cardInfo->getName(),
                     .color = cardInfo->getColors().isEmpty() ? QString() : cardInfo->getColors().left(1).toLower(),
                     .pt = cardInfo->getPowTough(),
                     .annotation = SettingsCache::instance().getAnnotateTokens() ? cardInfo->getText() : "",
                     .destroy = true,
                     .providerId =
                         SettingsCache::instance().cardOverrides().getCardPreferenceOverride(cardInfo->getName())};

    lastTokenTableRow = TableZone::clampValidTableRow(2 - cardInfo->getUiAttributes().tableRow);

    utilityMenu->setAndEnableCreateAnotherTokenAction(tr("C&reate another %1 token").arg(lastTokenInfo.name));
}

void PlayerActions::actCreatePredefinedToken()
{
    auto *action = static_cast<QAction *>(sender());
    CardInfoPtr cardInfo = CardDatabaseManager::query()->getCardInfo(action->text());
    if (!cardInfo) {
        return;
    }

    setLastToken(cardInfo);

    actCreateAnotherToken();
}

void PlayerActions::actCreateRelatedCard()
{
    const CardItem *sourceCard = player->getGame()->getActiveCard();
    if (!sourceCard) {
        return;
    }
    auto *action = static_cast<QAction *>(sender());
    // If there is a better way of passing a CardRelation through a QAction, please add it here.
    auto relatedCards = sourceCard->getCardInfo().getAllRelatedCards();
    CardRelation *cardRelation = relatedCards.at(action->data().toInt());

    /*
     * If we make a token via "Token: TokenName"
     * then let's allow it to be created via "create another token"
     */
    if (createRelatedFromRelation(sourceCard, cardRelation) && cardRelation->getCanCreateAnother()) {
        ExactCard relatedCard = CardDatabaseManager::query()->getCardFromSameSet(cardRelation->getName(),
                                                                                 sourceCard->getCard().getPrinting());
        setLastToken(relatedCard.getCardPtr());
    }
}

void PlayerActions::actCreateAllRelatedCards()
{
    const CardItem *sourceCard = player->getGame()->getActiveCard();
    if (!sourceCard) {
        return;
    }

    auto relatedCards = sourceCard->getCardInfo().getAllRelatedCards();
    if (relatedCards.isEmpty()) {
        return;
    }

    CardRelation *cardRelation = nullptr;
    int tokensTypesCreated = 0;

    if (relatedCards.length() == 1) {
        cardRelation = relatedCards.at(0);
        if (createRelatedFromRelation(sourceCard, cardRelation)) {
            ++tokensTypesCreated;
        }
    } else {
        QList<CardRelation *> nonExcludedRelatedCards;
        QString dbName;
        for (CardRelation *cardRelationTemp : relatedCards) {
            if (!cardRelationTemp->getIsCreateAllExclusion() && !cardRelationTemp->getDoesAttach()) {
                nonExcludedRelatedCards.append(cardRelationTemp);
            }
        }
        switch (nonExcludedRelatedCards.length()) {
            case 1: // if nonExcludedRelatedCards == 1
                cardRelation = nonExcludedRelatedCards.at(0);
                if (createRelatedFromRelation(sourceCard, cardRelation)) {
                    ++tokensTypesCreated;
                }
                break;
            // If all are marked "Exclude", then treat the situation as if none of them are.
            // We won't accept "garbage in, garbage out", here.
            case 0: // else if nonExcludedRelatedCards == 0
                for (CardRelation *cardRelationAll : relatedCards) {
                    if (!cardRelationAll->getDoesAttach() && !cardRelationAll->getIsVariable()) {
                        dbName = cardRelationAll->getName();
                        bool persistent = cardRelationAll->getIsPersistent();
                        for (int i = 0; i < cardRelationAll->getDefaultCount(); ++i) {
                            createCard(sourceCard, dbName, CardRelationType::DoesNotAttach, persistent);
                        }
                        ++tokensTypesCreated;
                        if (tokensTypesCreated == 1) {
                            cardRelation = cardRelationAll;
                        }
                    }
                }
                break;
            default: // else
                for (CardRelation *cardRelationNotExcluded : nonExcludedRelatedCards) {
                    if (!cardRelationNotExcluded->getDoesAttach() && !cardRelationNotExcluded->getIsVariable()) {
                        dbName = cardRelationNotExcluded->getName();
                        bool persistent = cardRelationNotExcluded->getIsPersistent();
                        for (int i = 0; i < cardRelationNotExcluded->getDefaultCount(); ++i) {
                            createCard(sourceCard, dbName, CardRelationType::DoesNotAttach, persistent);
                        }
                        ++tokensTypesCreated;
                        if (tokensTypesCreated == 1) {
                            cardRelation = cardRelationNotExcluded;
                        }
                    }
                }
                break;
        }
    }

    /*
     * If we made at least one token via "Create All Tokens"
     * then assign the first to the "Create another" shortcut.
     */
    if (cardRelation != nullptr && cardRelation->getCanCreateAnother()) {
        CardInfoPtr cardInfo = CardDatabaseManager::query()->getCardInfo(cardRelation->getName());
        setLastToken(cardInfo);
    }
}

bool PlayerActions::createRelatedFromRelation(const CardItem *sourceCard, const CardRelation *cardRelation)
{
    if (sourceCard == nullptr || cardRelation == nullptr) {
        return false;
    }
    QString dbName = cardRelation->getName();
    bool persistent = cardRelation->getIsPersistent();
    if (cardRelation->getIsVariable()) {
        bool ok;
        player->setDialogSemaphore(true);
        int count = QInputDialog::getInt(player->getGame()->getTab(), tr("Create tokens"), tr("Number:"),
                                         cardRelation->getDefaultCount(), 1, MAX_TOKENS_PER_DIALOG, 1, &ok);
        player->setDialogSemaphore(false);
        if (!ok) {
            return false;
        }
        for (int i = 0; i < count; ++i) {
            createCard(sourceCard, dbName, CardRelationType::DoesNotAttach, persistent);
        }
    } else if (cardRelation->getDefaultCount() > 1) {
        for (int i = 0; i < cardRelation->getDefaultCount(); ++i) {
            createCard(sourceCard, dbName, CardRelationType::DoesNotAttach, persistent);
        }
    } else {
        auto attachType = cardRelation->getAttachType();

        // move card onto table first if attaching from some other zone
        // we only do this for AttachTo because cross-zone TransformInto is already handled server-side
        if (attachType == CardRelationType::AttachTo && sourceCard->getZone()->getName() != "table") {
            playCardToTable(sourceCard, false);
        }

        createCard(sourceCard, dbName, attachType, persistent);
    }
    return true;
}

void PlayerActions::createCard(const CardItem *sourceCard,
                               const QString &dbCardName,
                               CardRelationType attachType,
                               bool persistent)
{
    CardInfoPtr cardInfo = CardDatabaseManager::query()->getCardInfo(dbCardName);

    if (cardInfo == nullptr || sourceCard == nullptr) {
        return;
    }

    // get the target token's location
    // TODO: Define this QPoint into its own function along with the one below
    QPoint gridPoint = QPoint(-1, TableZone::clampValidTableRow(2 - cardInfo->getUiAttributes().tableRow));

    // create the token for the related card
    Command_CreateToken cmd;
    cmd.set_zone("table");
    cmd.set_card_name(cardInfo->getName().toStdString());
    switch (cardInfo->getColors().size()) {
        case 0:
            cmd.set_color("");
            break;
        case 1:
            cmd.set_color("m");
            break;
        default:
            cmd.set_color(cardInfo->getColors().left(1).toLower().toStdString());
            break;
    }

    cmd.set_pt(cardInfo->getPowTough().toStdString());
    if (SettingsCache::instance().getAnnotateTokens()) {
        cmd.set_annotation(cardInfo->getText().toStdString());
    } else {
        cmd.set_annotation("");
    }
    cmd.set_destroy_on_zone_change(!persistent);
    cmd.set_x(gridPoint.x());
    cmd.set_y(gridPoint.y());

    ExactCard relatedCard =
        CardDatabaseManager::query()->getCardFromSameSet(cardInfo->getName(), sourceCard->getCard().getPrinting());

    switch (attachType) {
        case CardRelationType::DoesNotAttach:
            cmd.set_target_zone("table");
            cmd.set_card_provider_id(relatedCard.getPrinting().getUuid().toStdString());
            break;

        case CardRelationType::AttachTo:
            cmd.set_target_zone("table"); // We currently only support creating tokens on the table
            cmd.set_card_provider_id(relatedCard.getPrinting().getUuid().toStdString());
            cmd.set_target_card_id(sourceCard->getId());
            cmd.set_target_mode(Command_CreateToken::ATTACH_TO);
            break;

        case CardRelationType::TransformInto:
            // allow cards to directly transform on stack
            cmd.set_zone(sourceCard->getZone()->getName() == "stack" ? "stack" : "table");
            // Transform card zone changes are handled server-side
            cmd.set_target_zone(sourceCard->getZone()->getName().toStdString());
            cmd.set_target_card_id(sourceCard->getId());
            cmd.set_target_mode(Command_CreateToken::TRANSFORM_INTO);
            cmd.set_card_provider_id(sourceCard->getProviderId().toStdString());
            break;
    }

    sendGameCommand(cmd);
}

void PlayerActions::actSayMessage()
{
    auto *a = qobject_cast<QAction *>(sender());
    Command_GameSay cmd;
    cmd.set_message(a->text().toStdString());
    sendGameCommand(cmd);
}

void PlayerActions::setCardAttrHelper(const GameEventContext &context,
                                      CardItem *card,
                                      CardAttribute attribute,
                                      const QString &avalue,
                                      bool allCards,
                                      EventProcessingOptions options)
{
    if (card == nullptr) {
        return;
    }

    bool moveCardContext = context.HasExtension(Context_MoveCard::ext);
    switch (attribute) {
        case AttrTapped: {
            bool tapped = avalue == "1";
            if (!(!tapped && card->getDoesntUntap() && allCards)) {
                if (!allCards) {
                    emit logSetTapped(player, card, tapped);
                }
                bool canAnimate = !options.testFlag(SKIP_TAP_ANIMATION) && !moveCardContext;
                card->setTapped(tapped, canAnimate);
            }
            break;
        }
        case AttrAttacking: {
            card->setAttacking(avalue == "1");
            break;
        }
        case AttrFaceDown: {
            card->setFaceDown(avalue == "1");
            break;
        }
        case AttrColor: {
            card->setColor(avalue);
            break;
        }
        case AttrAnnotation: {
            emit logSetAnnotation(player, card, avalue);
            card->setAnnotation(avalue);
            break;
        }
        case AttrDoesntUntap: {
            bool value = (avalue == "1");
            emit logSetDoesntUntap(player, card, value);
            card->setDoesntUntap(value);
            break;
        }
        case AttrPT: {
            emit logSetPT(player, card, avalue);
            card->setPT(avalue);
            break;
        }
    }
}

void PlayerActions::actMoveCardXCardsFromTop()
{
    int deckSize = player->getDeckZone()->getCards().size() + 1; // add the card to move to the deck
    bool ok;
    int number =
        QInputDialog::getInt(player->getGame()->getTab(), tr("Place card X cards from top of library"),
                             tr("Which position should this card be placed:") + "\n" + tr("(max. %1)").arg(deckSize),
                             defaultNumberTopCardsToPlaceBelow, 1, deckSize, 1, &ok);
    number -= 1; // indexes start at 0

    if (!ok) {
        return;
    }

    defaultNumberTopCardsToPlaceBelow = number;

    QList<QGraphicsItem *> sel = player->getGameScene()->selectedItems();
    if (sel.isEmpty()) {
        return;
    }

    QList<CardItem *> cardList;
    while (!sel.isEmpty()) {
        cardList.append(qgraphicsitem_cast<CardItem *>(sel.takeFirst()));
    }

    QList<const ::google::protobuf::Message *> commandList;
    ListOfCardsToMove idList;
    for (const auto &i : cardList) {
        idList.add_card()->set_card_id(i->getId());
    }

    int startPlayerId = cardList[0]->getZone()->getPlayer()->getPlayerInfo()->getId();
    QString startZone = cardList[0]->getZone()->getName();

    auto *cmd = new Command_MoveCard;
    cmd->set_start_player_id(startPlayerId);
    cmd->set_start_zone(startZone.toStdString());
    cmd->mutable_cards_to_move()->CopyFrom(idList);
    cmd->set_target_player_id(player->getPlayerInfo()->getId());
    cmd->set_target_zone("deck");
    cmd->set_x(number);
    cmd->set_y(0);
    commandList.append(cmd);

    if (player->getPlayerInfo()->local) {
        sendGameCommand(prepareGameCommand(commandList));
    } else {
        player->getGame()->getGameEventHandler()->sendGameCommand(prepareGameCommand(commandList));
    }
}

void PlayerActions::actIncPT(int deltaP, int deltaT)
{
    int playerid = player->getPlayerInfo()->getId();

    QList<const ::google::protobuf::Message *> commandList;
    for (const auto &item : player->getGameScene()->selectedItems()) {
        auto *card = static_cast<CardItem *>(item);
        QString pt = card->getPT();
        const auto ptList = parsePT(pt);
        QString newpt;
        if (ptList.isEmpty()) {
            newpt = QString::number(deltaP) + (deltaT ? "/" + QString::number(deltaT) : "");
        } else if (ptList.size() == 1) {
            newpt = QString::number(ptList.at(0).toInt() + deltaP) + (deltaT ? "/" + QString::number(deltaT) : "");
        } else {
            newpt =
                QString::number(ptList.at(0).toInt() + deltaP) + "/" + QString::number(ptList.at(1).toInt() + deltaT);
        }

        auto *cmd = new Command_SetCardAttr;
        cmd->set_zone(card->getZone()->getName().toStdString());
        cmd->set_card_id(card->getId());
        cmd->set_attribute(AttrPT);
        cmd->set_attr_value(newpt.toStdString());
        commandList.append(cmd);

        if (player->getPlayerInfo()->getLocal()) {
            playerid = card->getZone()->getPlayer()->getPlayerInfo()->getId();
        }
    }

    player->getGame()->getGameEventHandler()->sendGameCommand(prepareGameCommand(commandList), playerid);
}

void PlayerActions::actResetPT()
{
    int playerid = player->getPlayerInfo()->getId();
    QList<const ::google::protobuf::Message *> commandList;
    for (const auto &item : player->getGameScene()->selectedItems()) {
        auto *card = static_cast<CardItem *>(item);
        QString ptString;
        if (!card->getFaceDown()) { // leave the pt empty if the card is face down
            ExactCard ec = card->getCard();
            if (ec) {
                ptString = ec.getInfo().getPowTough();
            }
        }
        if (ptString == card->getPT()) {
            continue;
        }
        QString zoneName = card->getZone()->getName();
        auto *cmd = new Command_SetCardAttr;
        cmd->set_zone(zoneName.toStdString());
        cmd->set_card_id(card->getId());
        cmd->set_attribute(AttrPT);
        cmd->set_attr_value(ptString.toStdString());
        commandList.append(cmd);

        if (player->getPlayerInfo()->getLocal()) {
            playerid = card->getZone()->getPlayer()->getPlayerInfo()->getId();
        }
    }

    if (!commandList.empty()) {
        player->getGame()->getGameEventHandler()->sendGameCommand(prepareGameCommand(commandList), playerid);
    }
}

QVariantList PlayerActions::parsePT(const QString &pt)
{
    QVariantList ptList = QVariantList();
    if (!pt.isEmpty()) {
        int sep = pt.indexOf('/');
        if (sep == 0) {
            ptList.append(QVariant(pt.mid(1))); // cut off starting '/' and take full string
        } else {
            int start = 0;
            for (;;) {
                QString item = pt.mid(start, sep - start);
                if (item.isEmpty()) {
                    ptList.append(QVariant(QString()));
                } else if (item[0] == '+') {
                    ptList.append(QVariant(item.mid(1).toInt())); // add as int
                } else if (item[0] == '-') {
                    ptList.append(QVariant(item.toInt())); // add as int
                } else {
                    ptList.append(QVariant(item)); // add as qstring
                }
                if (sep == -1) {
                    break;
                }
                start = sep + 1;
                sep = pt.indexOf('/', start);
            }
        }
    }
    return ptList;
}

void PlayerActions::actSetPT()
{
    QString oldPT;
    int playerid = player->getPlayerInfo()->getId();

    auto sel = player->getGameScene()->selectedItems();
    for (const auto &item : sel) {
        auto *card = static_cast<CardItem *>(item);
        if (!card->getPT().isEmpty()) {
            oldPT = card->getPT();
        }
    }
    bool ok;
    player->setDialogSemaphore(true);
    QString pt = getTextWithMax(player->getGame()->getTab(), tr("Change power/toughness"), tr("Change stats to:"),
                                QLineEdit::Normal, oldPT, &ok);
    player->setDialogSemaphore(false);
    if (player->clearCardsToDelete() || !ok) {
        return;
    }

    const auto ptList = parsePT(pt);
    bool empty = ptList.isEmpty();

    QList<const ::google::protobuf::Message *> commandList;
    for (const auto &item : sel) {
        auto *card = static_cast<CardItem *>(item);
        auto *cmd = new Command_SetCardAttr;
        QString newpt = QString();
        if (!empty) {
            const auto oldpt = parsePT(card->getPT());
            int ptIter = 0;
            for (const auto &_item : ptList) {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
                if (_item.typeId() == QMetaType::Type::Int) {
#else
                if (_item.type() == QVariant::Int) {
#endif
                    int oldItem = ptIter < oldpt.size() ? oldpt.at(ptIter).toInt() : 0;
                    newpt += '/' + QString::number(oldItem + _item.toInt());
                } else {
                    newpt += '/' + _item.toString();
                }
                ++ptIter;
            }
            newpt = newpt.mid(1);
        }

        cmd->set_zone(card->getZone()->getName().toStdString());
        cmd->set_card_id(card->getId());
        cmd->set_attribute(AttrPT);
        cmd->set_attr_value(newpt.toStdString());
        commandList.append(cmd);

        if (player->getPlayerInfo()->local) {
            playerid = card->getZone()->getPlayer()->getPlayerInfo()->getId();
        }
    }

    player->getGame()->getGameEventHandler()->sendGameCommand(prepareGameCommand(commandList), playerid);
}

void PlayerActions::actDrawArrow()
{
    auto *card = player->getGame()->getActiveCard();
    if (card) {
        card->drawArrow(Qt::red);
    }
}

void PlayerActions::actIncP()
{
    actIncPT(1, 0);
}

void PlayerActions::actDecP()
{
    actIncPT(-1, 0);
}

void PlayerActions::actIncT()
{
    actIncPT(0, 1);
}

void PlayerActions::actDecT()
{
    actIncPT(0, -1);
}

void PlayerActions::actIncPT()
{
    actIncPT(1, 1);
}

void PlayerActions::actDecPT()
{
    actIncPT(-1, -1);
}

void PlayerActions::actFlowP()
{
    actIncPT(1, -1);
}

void PlayerActions::actFlowT()
{
    actIncPT(-1, 1);
}

void AnnotationDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return && event->modifiers() & Qt::ControlModifier) {
        event->accept();
        accept();
        return;
    }
    QInputDialog::keyPressEvent(event);
}

void PlayerActions::actSetAnnotation()
{
    QString oldAnnotation;
    auto sel = player->getGameScene()->selectedItems();
    for (const auto &item : sel) {
        auto *card = static_cast<CardItem *>(item);
        if (!card->getAnnotation().isEmpty()) {
            oldAnnotation = card->getAnnotation();
        }
    }

    player->setDialogSemaphore(true);
    AnnotationDialog *dialog = new AnnotationDialog(player->getGame()->getTab());
    dialog->setOptions(QInputDialog::UsePlainTextEditForTextInput);
    dialog->setWindowTitle(tr("Set annotation"));
    dialog->setLabelText(tr("Please enter the new annotation:"));
    dialog->setTextValue(oldAnnotation);
    bool ok = dialog->exec();
    player->setDialogSemaphore(false);
    if (player->clearCardsToDelete() || !ok) {
        return;
    }
    QString annotation = dialog->textValue().left(MAX_NAME_LENGTH);

    QList<const ::google::protobuf::Message *> commandList;
    for (const auto &item : sel) {
        auto *card = static_cast<CardItem *>(item);
        auto *cmd = new Command_SetCardAttr;
        cmd->set_zone(card->getZone()->getName().toStdString());
        cmd->set_card_id(card->getId());
        cmd->set_attribute(AttrAnnotation);
        cmd->set_attr_value(annotation.toStdString());
        commandList.append(cmd);
    }
    sendGameCommand(prepareGameCommand(commandList));
}

void PlayerActions::actAttach()
{
    auto *card = player->getGame()->getActiveCard();
    if (!card) {
        return;
    }

    card->drawAttachArrow();
}

void PlayerActions::actUnattach()
{
    QList<const ::google::protobuf::Message *> commandList;
    for (QGraphicsItem *item : player->getGameScene()->selectedItems()) {
        auto *card = static_cast<CardItem *>(item);

        if (!card->getAttachedTo()) {
            continue;
        }

        auto *cmd = new Command_AttachCard;
        cmd->set_start_zone(card->getZone()->getName().toStdString());
        cmd->set_card_id(card->getId());
        commandList.append(cmd);
    }
    sendGameCommand(prepareGameCommand(commandList));
}

void PlayerActions::actCardCounterTrigger()
{
    auto *action = static_cast<QAction *>(sender());
    int counterId = action->data().toInt() / 1000;
    QList<const ::google::protobuf::Message *> commandList;
    switch (action->data().toInt() % 1000) {
        case 9: { // increment counter
            for (const auto &item : player->getGameScene()->selectedItems()) {
                auto *card = static_cast<CardItem *>(item);
                if (card->getCounters().value(counterId, 0) < MAX_COUNTERS_ON_CARD) {
                    auto *cmd = new Command_SetCardCounter;
                    cmd->set_zone(card->getZone()->getName().toStdString());
                    cmd->set_card_id(card->getId());
                    cmd->set_counter_id(counterId);
                    cmd->set_counter_value(card->getCounters().value(counterId, 0) + 1);
                    commandList.append(cmd);
                }
            }
            break;
        }
        case 10: { // decrement counter
            for (const auto &item : player->getGameScene()->selectedItems()) {
                auto *card = static_cast<CardItem *>(item);
                if (card->getCounters().value(counterId, 0)) {
                    auto *cmd = new Command_SetCardCounter;
                    cmd->set_zone(card->getZone()->getName().toStdString());
                    cmd->set_card_id(card->getId());
                    cmd->set_counter_id(counterId);
                    cmd->set_counter_value(card->getCounters().value(counterId, 0) - 1);
                    commandList.append(cmd);
                }
            }
            break;
        }
        case 11: { // set counter with dialog
            bool ok;
            player->setDialogSemaphore(true);

            int oldValue = 0;
            if (player->getGameScene()->selectedItems().size() == 1) {
                auto *card = static_cast<CardItem *>(player->getGameScene()->selectedItems().first());
                oldValue = card->getCounters().value(counterId, 0);
            }
            int number = QInputDialog::getInt(player->getGame()->getTab(), tr("Set counters"), tr("Number:"), oldValue,
                                              0, MAX_COUNTERS_ON_CARD, 1, &ok);
            player->setDialogSemaphore(false);
            if (player->clearCardsToDelete() || !ok) {
                return;
            }

            for (const auto &item : player->getGameScene()->selectedItems()) {
                auto *card = static_cast<CardItem *>(item);
                auto *cmd = new Command_SetCardCounter;
                cmd->set_zone(card->getZone()->getName().toStdString());
                cmd->set_card_id(card->getId());
                cmd->set_counter_id(counterId);
                cmd->set_counter_value(number);
                commandList.append(cmd);
            }
            break;
        }
        default:;
    }
    sendGameCommand(prepareGameCommand(commandList));
}

/**
 * @brief returns true if the zone is a unwritable reveal zone view (eg a card reveal window). Will return false if zone
 * is nullptr.
 */
static bool isUnwritableRevealZone(CardZoneLogic *zone)
{
    if (auto *view = qobject_cast<ZoneViewZoneLogic *>(zone)) {
        return view->getRevealZone() && !view->getWriteableRevealZone();
    }
    return false;
}

void PlayerActions::playSelectedCards(const bool faceDown)
{
    QList<CardItem *> selectedCards;
    for (const auto &item : player->getGameScene()->selectedItems()) {
        auto *card = static_cast<CardItem *>(item);
        selectedCards.append(card);
    }

    // CardIds will get shuffled downwards when cards leave the deck.
    // We need to iterate through the cards in reverse order so cardIds don't get changed out from under us as we play
    // out the cards one-by-one.
    std::sort(selectedCards.begin(), selectedCards.end(),
              [](const auto &card1, const auto &card2) { return card1->getId() > card2->getId(); });

    for (auto &card : selectedCards) {
        if (card && !isUnwritableRevealZone(card->getZone()) && card->getZone()->getName() != "table") {
            playCard(card, faceDown);
        }
    }
}

void PlayerActions::actPlay()
{
    playSelectedCards(false);
}

void PlayerActions::actPlayFacedown()
{
    playSelectedCards(true);
}

void PlayerActions::actHide()
{
    for (const auto &item : player->getGameScene()->selectedItems()) {
        auto *card = static_cast<CardItem *>(item);
        if (card && isUnwritableRevealZone(card->getZone())) {
            card->getZone()->removeCard(card);
        }
    }
}

void PlayerActions::actReveal(QAction *action)
{
    const int otherPlayerId = action->data().toInt();

    Command_RevealCards cmd;
    if (otherPlayerId != -1) {
        cmd.set_player_id(otherPlayerId);
    }

    QList<QGraphicsItem *> sel = player->getGameScene()->selectedItems();
    while (!sel.isEmpty()) {
        const auto *card = qgraphicsitem_cast<CardItem *>(sel.takeFirst());
        if (!cmd.has_zone_name()) {
            cmd.set_zone_name(card->getZone()->getName().toStdString());
        }
        cmd.add_card_id(card->getId());
    }

    sendGameCommand(cmd);
}

void PlayerActions::actRevealHand(int revealToPlayerId)
{
    Command_RevealCards cmd;
    if (revealToPlayerId != -1) {
        cmd.set_player_id(revealToPlayerId);
    }
    cmd.set_zone_name("hand");

    sendGameCommand(cmd);
}

void PlayerActions::actRevealRandomHandCard(int revealToPlayerId)
{
    Command_RevealCards cmd;
    if (revealToPlayerId != -1) {
        cmd.set_player_id(revealToPlayerId);
    }
    cmd.set_zone_name("hand");
    cmd.add_card_id(RANDOM_CARD_FROM_ZONE);

    sendGameCommand(cmd);
}

void PlayerActions::actRevealLibrary(int revealToPlayerId)
{
    Command_RevealCards cmd;
    if (revealToPlayerId != -1) {
        cmd.set_player_id(revealToPlayerId);
    }
    cmd.set_zone_name("deck");

    sendGameCommand(cmd);
}

void PlayerActions::actLendLibrary(int lendToPlayerId)
{
    Command_RevealCards cmd;
    if (lendToPlayerId != -1) {
        cmd.set_player_id(lendToPlayerId);
    }
    cmd.set_zone_name("deck");
    cmd.set_grant_write_access(true);

    sendGameCommand(cmd);
}

void PlayerActions::actRevealTopCards(int revealToPlayerId, int amount)
{
    Command_RevealCards cmd;
    if (revealToPlayerId != -1) {
        cmd.set_player_id(revealToPlayerId);
    }

    cmd.set_zone_name("deck");
    cmd.set_top_cards(amount);
    // backward compatibility: servers before #1051 only permits to reveal the first card
    cmd.add_card_id(0);

    sendGameCommand(cmd);
}

void PlayerActions::actRevealRandomGraveyardCard(int revealToPlayerId)
{
    Command_RevealCards cmd;
    if (revealToPlayerId != -1) {
        cmd.set_player_id(revealToPlayerId);
    }
    cmd.set_zone_name("grave");
    cmd.add_card_id(RANDOM_CARD_FROM_ZONE);
    sendGameCommand(cmd);
}

void PlayerActions::cardMenuAction()
{
    auto *a = dynamic_cast<QAction *>(sender());
    QList<QGraphicsItem *> sel = player->getGameScene()->selectedItems();
    QList<CardItem *> cardList;
    while (!sel.isEmpty()) {
        cardList.append(qgraphicsitem_cast<CardItem *>(sel.takeFirst()));
    }

    QList<const ::google::protobuf::Message *> commandList;
    if (a->data().toInt() <= (int)cmClone) {
        for (const auto &card : cardList) {
            switch (static_cast<CardMenuActionType>(a->data().toInt())) {
                // Leaving both for compatibility with server
                case cmUntap:
                    // fallthrough
                case cmTap: {
                    auto *cmd = new Command_SetCardAttr;
                    cmd->set_zone(card->getZone()->getName().toStdString());
                    cmd->set_card_id(card->getId());
                    cmd->set_attribute(AttrTapped);
                    cmd->set_attr_value(std::to_string(1 - static_cast<int>(card->getTapped())));
                    commandList.append(cmd);
                    break;
                }
                case cmDoesntUntap: {
                    auto *cmd = new Command_SetCardAttr;
                    cmd->set_zone(card->getZone()->getName().toStdString());
                    cmd->set_card_id(card->getId());
                    cmd->set_attribute(AttrDoesntUntap);
                    cmd->set_attr_value(card->getDoesntUntap() ? "0" : "1");
                    commandList.append(cmd);
                    break;
                }
                case cmFlip: {
                    auto *cmd = new Command_FlipCard;
                    cmd->set_zone(card->getZone()->getName().toStdString());
                    cmd->set_card_id(card->getId());
                    cmd->set_face_down(!card->getFaceDown());
                    if (card->getFaceDown()) {
                        ExactCard ec = card->getCard();
                        if (ec) {
                            cmd->set_pt(ec.getInfo().getPowTough().toStdString());
                        }
                    }
                    commandList.append(cmd);
                    break;
                }
                case cmPeek: {
                    auto *cmd = new Command_RevealCards;
                    cmd->set_zone_name(card->getZone()->getName().toStdString());
                    cmd->add_card_id(card->getId());
                    cmd->set_player_id(player->getPlayerInfo()->getId());
                    commandList.append(cmd);
                    break;
                }
                case cmClone: {
                    auto *cmd = new Command_CreateToken;
                    cmd->set_zone("table");
                    cmd->set_card_name(card->getName().toStdString());
                    cmd->set_card_provider_id(card->getProviderId().toStdString());
                    cmd->set_color(card->getColor().toStdString());
                    cmd->set_pt(card->getPT().toStdString());
                    cmd->set_annotation(card->getAnnotation().toStdString());
                    cmd->set_destroy_on_zone_change(true);
                    cmd->set_x(-1);
                    cmd->set_y(card->getGridPoint().y());
                    commandList.append(cmd);
                    break;
                }
                default:
                    break;
            }
        }
    } else {
        CardZoneLogic *zone = cardList[0]->getZone();
        if (!zone) {
            return;
        }

        Player *startPlayer = zone->getPlayer();
        if (!startPlayer) {
            return;
        }

        int startPlayerId = startPlayer->getPlayerInfo()->getId();
        QString startZone = zone->getName();

        ListOfCardsToMove idList;
        for (const auto &i : cardList) {
            idList.add_card()->set_card_id(i->getId());
        }

        switch (static_cast<CardMenuActionType>(a->data().toInt())) {
            case cmMoveToTopLibrary: {
                auto *cmd = new Command_MoveCard;
                cmd->set_start_player_id(startPlayerId);
                cmd->set_start_zone(startZone.toStdString());
                cmd->mutable_cards_to_move()->CopyFrom(idList);
                cmd->set_target_player_id(player->getPlayerInfo()->getId());
                cmd->set_target_zone("deck");
                cmd->set_x(0);
                cmd->set_y(0);

                if (idList.card_size() > 1) {
                    auto *scmd = new Command_Shuffle;
                    scmd->set_zone_name("deck");
                    scmd->set_start(0);
                    scmd->set_end(idList.card_size() - 1); // inclusive, the indexed card at end will be shuffled
                    // Server process events backwards, so...
                    commandList.append(scmd);
                }

                commandList.append(cmd);
                break;
            }
            case cmMoveToBottomLibrary: {
                auto *cmd = new Command_MoveCard;
                cmd->set_start_player_id(startPlayerId);
                cmd->set_start_zone(startZone.toStdString());
                cmd->mutable_cards_to_move()->CopyFrom(idList);
                cmd->set_target_player_id(player->getPlayerInfo()->getId());
                cmd->set_target_zone("deck");
                cmd->set_x(-1);
                cmd->set_y(0);

                if (idList.card_size() > 1) {
                    auto *scmd = new Command_Shuffle;
                    scmd->set_zone_name("deck");
                    scmd->set_start(-idList.card_size());
                    scmd->set_end(-1);
                    // Server process events backwards, so...
                    commandList.append(scmd);
                }

                commandList.append(cmd);
                break;
            }
            case cmMoveToHand: {
                auto *cmd = new Command_MoveCard;
                cmd->set_start_player_id(startPlayerId);
                cmd->set_start_zone(startZone.toStdString());
                cmd->mutable_cards_to_move()->CopyFrom(idList);
                cmd->set_target_player_id(player->getPlayerInfo()->getId());
                cmd->set_target_zone("hand");
                cmd->set_x(0);
                cmd->set_y(0);
                commandList.append(cmd);
                break;
            }
            case cmMoveToGraveyard: {
                auto *cmd = new Command_MoveCard;
                cmd->set_start_player_id(startPlayerId);
                cmd->set_start_zone(startZone.toStdString());
                cmd->mutable_cards_to_move()->CopyFrom(idList);
                cmd->set_target_player_id(player->getPlayerInfo()->getId());
                cmd->set_target_zone("grave");
                cmd->set_x(0);
                cmd->set_y(0);
                commandList.append(cmd);
                break;
            }
            case cmMoveToExile: {
                auto *cmd = new Command_MoveCard;
                cmd->set_start_player_id(startPlayerId);
                cmd->set_start_zone(startZone.toStdString());
                cmd->mutable_cards_to_move()->CopyFrom(idList);
                cmd->set_target_player_id(player->getPlayerInfo()->getId());
                cmd->set_target_zone("rfg");
                cmd->set_x(0);
                cmd->set_y(0);
                commandList.append(cmd);
                break;
            }
            default:
                break;
        }
    }

    if (player->getPlayerInfo()->getLocal()) {
        sendGameCommand(prepareGameCommand(commandList));
    } else {
        player->getGame()->getGameEventHandler()->sendGameCommand(prepareGameCommand(commandList));
    }
}

PendingCommand *PlayerActions::prepareGameCommand(const google::protobuf::Message &cmd)
{

    if (player->getPlayerInfo()->getJudge() && !player->getPlayerInfo()->getLocal()) {
        Command_Judge base;
        GameCommand *c = base.add_game_command();
        base.set_target_id(player->getPlayerInfo()->getId());
        c->GetReflection()->MutableMessage(c, cmd.GetDescriptor()->FindExtensionByName("ext"))->CopyFrom(cmd);
        return player->getGame()->getGameEventHandler()->prepareGameCommand(base);
    } else {
        return player->getGame()->getGameEventHandler()->prepareGameCommand(cmd);
    }
}

PendingCommand *PlayerActions::prepareGameCommand(const QList<const ::google::protobuf::Message *> &cmdList)
{
    if (player->getPlayerInfo()->getJudge() && !player->getPlayerInfo()->getLocal()) {
        Command_Judge base;
        base.set_target_id(player->getPlayerInfo()->getId());
        for (int i = 0; i < cmdList.size(); ++i) {
            GameCommand *c = base.add_game_command();
            c->GetReflection()
                ->MutableMessage(c, cmdList[i]->GetDescriptor()->FindExtensionByName("ext"))
                ->CopyFrom(*cmdList[i]);
            delete cmdList[i];
        }
        return player->getGame()->getGameEventHandler()->prepareGameCommand(base);
    } else {
        return player->getGame()->getGameEventHandler()->prepareGameCommand(cmdList);
    }
}

void PlayerActions::sendGameCommand(const google::protobuf::Message &command)
{
    if (player->getPlayerInfo()->getJudge() && !player->getPlayerInfo()->getLocal()) {
        Command_Judge base;
        GameCommand *c = base.add_game_command();
        base.set_target_id(player->getPlayerInfo()->getId());
        c->GetReflection()->MutableMessage(c, command.GetDescriptor()->FindExtensionByName("ext"))->CopyFrom(command);
        player->getGame()->getGameEventHandler()->sendGameCommand(base, player->getPlayerInfo()->getId());
    } else {
        player->getGame()->getGameEventHandler()->sendGameCommand(command, player->getPlayerInfo()->getId());
    }
}

void PlayerActions::sendGameCommand(PendingCommand *pend)
{
    player->getGame()->getGameEventHandler()->sendGameCommand(pend, player->getPlayerInfo()->getId());
}
