#include "player_menu.h"

#include "../../../client/settings/cache_settings.h"
#include "../../../client/settings/shortcuts_settings.h"
#include "../../../game/abstract_game.h"
#include "../../../game/player/player_info.h"
#include "../../../game/player/player_logic.h"
#include "../../../game/player/player_manager.h"
#include "../../../game_graphics/zones/hand_zone.h"
#include "../../../game_graphics/zones/pile_zone.h"
#include "../../../game_graphics/zones/table_zone.h"
#include "../../../interface/widgets/menus/tearoff_menu.h"
#include "../../board/card_item.h"
#include "../player_graphics_item.h"
#include "abstract_player_component.h"
#include "card_menu.h"
#include "custom_zone_menu.h"
#include "grave_menu.h"
#include "hand_menu.h"
#include "library_menu.h"
#include "rfg_menu.h"
#include "say_menu.h"
#include "sideboard_menu.h"
#include "tally_menu.h"
#include "utility_menu.h"

#include <QMenu>
#include <QString>

PlayerMenu::PlayerMenu(PlayerGraphicsItem *_player) : QObject(_player), player(_player)
{
    connect(player->getLogic(), &PlayerLogic::requestCardMenuUpdate, this, &PlayerMenu::updateCardMenu);
    connect(this, &PlayerMenu::cardInfoRequested, player, &PlayerGraphicsItem::cardInfoRequested);

    playerMenu = new TearOffMenu();

    if (player->getLogic()->getPlayerInfo()->getLocalOrJudge()) {
        handMenu = addManagedMenu<HandMenu>(player, playerMenu);
        libraryMenu = addManagedMenu<LibraryMenu>(player, playerMenu);
    } else {
        handMenu = nullptr;
        libraryMenu = nullptr;
    }

    graveMenu = addManagedMenu<GraveyardMenu>(player, playerMenu);
    rfgMenu = addManagedMenu<RfgMenu>(player, playerMenu);

    if (player->getLogic()->getPlayerInfo()->getLocalOrJudge()) {
        sideboardMenu = addManagedMenu<SideboardMenu>(player, playerMenu);
        customZonesMenu = addManagedMenu<CustomZoneMenu>(player);
        playerMenu->addSeparator();

        countersMenu = playerMenu->addMenu(QString());

        utilityMenu = createManagedComponent<UtilityMenu>(player, playerMenu);
    } else {
        sideboardMenu = nullptr;
        customZonesMenu = nullptr;
        countersMenu = nullptr;
        utilityMenu = nullptr;
    }

    tallyMenu = addManagedMenu<TallyMenu>();

    if (player->getLogic()->getPlayerInfo()->getLocal()) {
        sayMenu = addManagedMenu<SayMenu>(player);
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
    player->getTableZoneGraphicsItem()->setMenu(playerMenu);
    player->getGraveyardZoneGraphicsItem()->setMenu(graveMenu, graveMenu->aViewGraveyard);
    player->getRfgZoneGraphicsItem()->setMenu(rfgMenu, rfgMenu->aViewRfg);
    if (player->getLogic()->getPlayerInfo()->getLocalOrJudge()) {
        player->getHandZoneGraphicsItem()->setMenu(handMenu);
        player->getDeckZoneGraphicsItem()->setMenu(libraryMenu, libraryMenu->aDrawCard);
        player->getSideboardZoneGraphicsItem()->setMenu(sideboardMenu);
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
    if ((player->getLogic()->getGame()->getPlayerManager()->isSpectator() &&
         !player->getLogic()->getGame()->getPlayerManager()->isJudge()) ||
        player->getLogic()->getGame()->getActiveCard() != card) {
        return nullptr;
    }

    CardMenu *menu = new CardMenu(player, card, shortcutsActive);
    connect(menu, &CardMenu::cardInfoRequested, this, &PlayerMenu::cardInfoRequested);
    emit cardMenuUpdated(menu);

    return menu;
}

void PlayerMenu::retranslateUi()
{
    playerMenu->setTitle(tr("Player \"%1\"").arg(player->getLogic()->getPlayerInfo()->getName()));

    for (auto *component : managedComponents) {
        component->retranslateUi();
    }

    if (countersMenu) {
        countersMenu->setTitle(tr("&Counters"));
    }

    emit retranslateRequested();
}

void PlayerMenu::refreshShortcuts()
{
    if (shortcutsActive) {
        // Judges get access to every player's menus but only want shortcuts to be set for their own.
        if (player->getLogic()->getPlayerInfo()->getLocalOrJudge() &&
            !player->getLogic()->getPlayerInfo()->getLocal()) {
            setShortcutsInactive();
        } else {
            setShortcutsActive();
        }
    } else {
        setShortcutsInactive();
    }
}

void PlayerMenu::setShortcutsActive()
{
    shortcutsActive = true;
    for (auto *c : managedComponents) {
        c->setShortcutsActive();
    }
    emit shortcutsActivated();
}

void PlayerMenu::setShortcutsInactive()
{
    shortcutsActive = false;
    for (auto *c : managedComponents) {
        c->setShortcutsInactive();
    }
    emit shortcutsDeactivated();
}