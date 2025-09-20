#include "player_menu.h"

#include "../../../common/pb/command_reveal_cards.pb.h"
#include "../../../database/card_database_manager.h"
#include "../../../tabs/tab_game.h"
#include "../../board/card_item.h"
#include "../../zones/hand_zone.h"
#include "../card_menu_action_type.h"
#include "../player_actions.h"
#include "card_menu.h"
#include "hand_menu.h"

PlayerMenu::PlayerMenu(Player *_player) : player(_player)
{

    if (player->getPlayerInfo()->getLocalOrJudge()) {
        connect(player->getGame()->getPlayerManager(), &PlayerManager::playerAdded, this, &PlayerMenu::addPlayer);
        connect(player->getGame()->getPlayerManager(), &PlayerManager::playerRemoved, this, &PlayerMenu::removePlayer);
    }

    const QList<Player *> &players = player->getGame()->getPlayerManager()->getPlayers().values();
    for (const auto playerToAdd : players) {
        addPlayer(playerToAdd);
    }

    playerMenu = new TearOffMenu();

    if (player->getPlayerInfo()->getLocalOrJudge()) {
        handMenu = new HandMenu(player, player->getPlayerActions(), playerMenu);
        playerMenu->addMenu(handMenu);
        playerLists.append(handMenu->revealHandMenu());
        playerLists.append(handMenu->revealRandomHandCardMenu());

        libraryMenu = new LibraryMenu(player, playerMenu);
        playerLists.append(libraryMenu->revealLibrary());
        playerLists.append(libraryMenu->lendLibraryMenu());
        playerLists.append(libraryMenu->revealTopCardMenu());
        playerMenu->addMenu(libraryMenu);
    } else {
        handMenu = nullptr;
        libraryMenu = nullptr;
    }

    graveMenu = new GraveyardMenu(player, playerMenu);
    connect(graveMenu, &GraveyardMenu::newPlayerActionCreated, this, &PlayerMenu::onNewPlayerListActionCreated);
    playerMenu->addMenu(graveMenu);

    rfgMenu = new RfgMenu(player, playerMenu);
    playerMenu->addMenu(rfgMenu);

    if (player->getPlayerInfo()->getLocalOrJudge()) {
        sideboardMenu = new SideboardMenu(player, playerMenu);
        playerMenu->addMenu(sideboardMenu);

        customZonesMenu = new CustomZoneMenu(player);
        playerMenu->addMenu(customZonesMenu);
        playerMenu->addSeparator();

        countersMenu = playerMenu->addMenu(QString());

        utilityMenu = new UtilityMenu(player, playerMenu);
    } else {
        sideboardMenu = nullptr;
        customZonesMenu = nullptr;
        countersMenu = nullptr;
        utilityMenu = nullptr;
    }

    if (player->getPlayerInfo()->getLocal()) {
        sayMenu = new SayMenu(player);
        playerMenu->addMenu(sayMenu);
    } else {
        sayMenu = nullptr;
    }

    if (player->getPlayerInfo()->getLocalOrJudge()) {

        for (auto &playerList : playerLists) {
            QAction *newAction = playerList->addAction(QString());
            newAction->setData(-1);
            connect(newAction, &QAction::triggered, this, &PlayerMenu::playerListActionTriggered);
            allPlayersActions.append(newAction);
            playerList->addSeparator();
        }
    }

    connect(&SettingsCache::instance().shortcuts(), &ShortcutsSettings::shortCutChanged, this,
            &PlayerMenu::refreshShortcuts);
    refreshShortcuts();

    retranslateUi();
}

void PlayerMenu::setMenusForGraphicItems()
{
    player->getGraphicsItem()->getTableZoneGraphicsItem()->setMenu(playerMenu);
    player->getGraphicsItem()->getGraveyardZoneGraphicsItem()->setMenu(graveMenu, graveMenu->aViewGraveyard);
    player->getGraphicsItem()->getRfgZoneGraphicsItem()->setMenu(rfgMenu, rfgMenu->aViewRfg);
    if (player->getPlayerInfo()->getLocalOrJudge()) {
        player->getGraphicsItem()->getHandZoneGraphicsItem()->setMenu(handMenu);
        player->getGraphicsItem()->getDeckZoneGraphicsItem()->setMenu(libraryMenu, libraryMenu->aDrawCard);
        player->getGraphicsItem()->getSideboardZoneGraphicsItem()->setMenu(sideboardMenu);
    }
}

void PlayerMenu::addPlayer(Player *playerToAdd)
{
    if (playerToAdd == nullptr || playerToAdd == player) {
        return;
    }

    for (auto &playerList : playerLists) {
        addPlayerToList(playerList, playerToAdd);
    }
}

void PlayerMenu::addPlayerToList(QMenu *playerList, Player *playerToAdd)
{
    QAction *newAction = playerList->addAction(playerToAdd->getPlayerInfo()->getName());
    newAction->setData(playerToAdd->getPlayerInfo()->getId());
    connect(newAction, &QAction::triggered, this, &PlayerMenu::playerListActionTriggered);
}

void PlayerMenu::removePlayer(Player *playerToRemove)
{
    if (playerToRemove == nullptr) {
        return;
    }

    for (auto &playerList : playerLists) {
        removePlayerFromList(playerList, playerToRemove);
    }
}

void PlayerMenu::removePlayerFromList(QMenu *playerList, Player *player)
{
    QList<QAction *> actionList = playerList->actions();
    for (auto &j : actionList)
        if (j->data().toInt() == player->getPlayerInfo()->getId()) {
            playerList->removeAction(j);
            j->deleteLater();
        }
}

void PlayerMenu::playerListActionTriggered()
{
    auto *action = static_cast<QAction *>(sender());
    auto *menu = static_cast<QMenu *>(action->parent());

    Command_RevealCards cmd;
    const int otherPlayerId = action->data().toInt();
    if (otherPlayerId != -1) {
        cmd.set_player_id(otherPlayerId);
    }

    if (menu == libraryMenu->revealLibrary() || menu == libraryMenu->lendLibraryMenu()) {
        cmd.set_zone_name("deck");
        cmd.set_grant_write_access(menu == libraryMenu->lendLibraryMenu());
    } else if (menu == libraryMenu->revealTopCardMenu()) {
        int deckSize = player->getDeckZone()->getCards().size();
        bool ok;
        int number = QInputDialog::getInt(player->getGame()->getTab(), tr("Reveal top cards of library"),
                                          tr("Number of cards: (max. %1)").arg(deckSize), /* defaultNumberTopCards */ 1,
                                          1, deckSize, 1, &ok);
        if (ok) {
            cmd.set_zone_name("deck");
            cmd.set_top_cards(number);
            // backward compatibility: servers before #1051 only permits to reveal the first card
            cmd.add_card_id(0);
            // defaultNumberTopCards = number;
        }
    } else if (menu == handMenu->revealHandMenu()) {
        cmd.set_zone_name("hand");
    } else if (menu == handMenu->revealRandomHandCardMenu()) {
        cmd.set_zone_name("hand");
        cmd.add_card_id(PlayerActions::RANDOM_CARD_FROM_ZONE);
    } else {
        return;
    }

    player->getPlayerActions()->sendGameCommand(cmd);
}

void PlayerMenu::onNewPlayerListActionCreated(QAction *action)
{
    allPlayersActions.append(action);
}

QMenu *PlayerMenu::updateCardMenu(const CardItem *card)
{
    if (!card) {
        emit cardMenuUpdated(nullptr);
        return nullptr;
    }

    // If is spectator (as spectators don't need card menus), return
    // only update the menu if the card is actually selected
    if ((player->getGame()->getPlayerManager()->isSpectator() && !player->getGame()->getPlayerManager()->isJudge()) ||
        player->getGame()->getActiveCard() != card) {
        return nullptr;
    }

    QMenu *menu = new CardMenu(player, card, shortcutsActive);
    emit cardMenuUpdated(menu);

    return menu;
}

void PlayerMenu::retranslateUi()
{
    playerMenu->setTitle(tr("Player \"%1\"").arg(player->getPlayerInfo()->getName()));

    if (handMenu) {
        handMenu->retranslateUi();
    }
    if (libraryMenu) {
        libraryMenu->retranslateUi();
    }

    graveMenu->retranslateUi();
    rfgMenu->retranslateUi();

    if (sideboardMenu) {
        sideboardMenu->retranslateUi();
    }

    if (countersMenu) {
        countersMenu->setTitle(tr("&Counters"));
    }

    if (customZonesMenu) {
        customZonesMenu->retranslateUi();
    }

    QMapIterator<int, AbstractCounter *> counterIterator(player->getCounters());
    while (counterIterator.hasNext()) {
        counterIterator.next().value()->retranslateUi();
    }

    if (utilityMenu) {
        utilityMenu->retranslateUi();
    }

    for (auto &allPlayersAction : allPlayersActions) {
        allPlayersAction->setText(tr("&All players"));
    }

    if (sayMenu) {
        sayMenu->setTitle(tr("S&ay"));
    }
}

void PlayerMenu::refreshShortcuts()
{
    if (shortcutsActive) {
        setShortcutsActive();
    }
}

void PlayerMenu::setShortcutsActive()
{
    shortcutsActive = true;

    if (handMenu) {
        handMenu->setShortcutsActive();
    }
    if (libraryMenu) {
        libraryMenu->setShortcutsActive();
    }
    graveMenu->setShortcutsActive();
    // No shortcuts for RfgMenu yet

    if (sideboardMenu) {
        sideboardMenu->setShortcutsActive();
    }

    QMapIterator<int, AbstractCounter *> counterIterator(player->getCounters());
    while (counterIterator.hasNext()) {
        counterIterator.next().value()->setShortcutsActive();
    }

    if (utilityMenu) {
        utilityMenu->setShortcutsActive();
    }
}

void PlayerMenu::setShortcutsInactive()
{
    shortcutsActive = false;

    if (handMenu) {
        handMenu->setShortcutsInactive();
    }
    if (libraryMenu) {
        libraryMenu->setShortcutsInactive();
    }
    graveMenu->setShortcutsInactive();
    // No shortcuts for RfgMenu yet

    if (sideboardMenu) {
        sideboardMenu->setShortcutsInactive();
    }

    QMapIterator<int, AbstractCounter *> counterIterator(player->getCounters());
    while (counterIterator.hasNext()) {
        counterIterator.next().value()->setShortcutsInactive();
    }

    if (utilityMenu) {
        utilityMenu->setShortcutsInactive();
    }
}