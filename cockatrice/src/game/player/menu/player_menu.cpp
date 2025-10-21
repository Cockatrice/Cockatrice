#include "player_menu.h"

#include "../../../interface/widgets/tabs/tab_game.h"
#include "../../board/card_item.h"
#include "../../zones/hand_zone.h"
#include "../card_menu_action_type.h"
#include "../player_actions.h"
#include "card_menu.h"
#include "hand_menu.h"

#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/protocol/pb/command_reveal_cards.pb.h>

PlayerMenu::PlayerMenu(Player *_player) : player(_player)
{
    playerMenu = new TearOffMenu();

    if (player->getPlayerInfo()->getLocalOrJudge()) {
        handMenu = new HandMenu(player, player->getPlayerActions(), playerMenu);
        playerMenu->addMenu(handMenu);

        libraryMenu = new LibraryMenu(player, playerMenu);
        playerMenu->addMenu(libraryMenu);
    } else {
        handMenu = nullptr;
        libraryMenu = nullptr;
    }

    graveMenu = new GraveyardMenu(player, playerMenu);
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