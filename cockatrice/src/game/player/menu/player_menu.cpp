#include "player_menu.h"

#include "../../../interface/widgets/tabs/tab_game.h"
#include "../../board/card_item.h"
#include "../../zones/command_zone.h"
#include "../../zones/hand_zone.h"
#include "../../zones/pile_zone.h"
#include "../../zones/table_zone.h"
#include "../../zones/zone_names.h"
#include "../player_graphics_item.h"
#include "card_menu.h"
#include "hand_menu.h"

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

        commandZoneMenu = new CommandZoneMenu(player, ZoneNames::COMMAND, playerMenu);
        playerMenu->addMenu(commandZoneMenu);
        partnerZoneMenu = new CommandZoneMenu(player, ZoneNames::PARTNER, playerMenu);
        companionZoneMenu = new CommandZoneMenu(player, ZoneNames::COMPANION, playerMenu);
        playerMenu->addMenu(companionZoneMenu);
        backgroundZoneMenu = new CommandZoneMenu(player, ZoneNames::BACKGROUND, playerMenu);
        playerMenu->addMenu(backgroundZoneMenu);

        auto updateCommandZoneMenuVisibility = [this](bool has) {
            if (commandZoneMenu)
                commandZoneMenu->menuAction()->setVisible(has);
        };
        auto updateCompanionZoneMenuVisibility = [this](bool has) {
            if (companionZoneMenu)
                companionZoneMenu->menuAction()->setVisible(has);
        };
        auto updateBackgroundZoneMenuVisibility = [this](bool has) {
            if (backgroundZoneMenu)
                backgroundZoneMenu->menuAction()->setVisible(has);
        };

        connect(player, &Player::commandZoneSupportChanged, this, updateCommandZoneMenuVisibility);
        connect(player, &Player::companionZoneSupportChanged, this, updateCompanionZoneMenuVisibility);
        connect(player, &Player::backgroundZoneSupportChanged, this, updateBackgroundZoneMenuVisibility);

        updateCommandZoneMenuVisibility(player->hasServerCommandZone());
        updateCompanionZoneMenuVisibility(player->hasServerCompanionZone());
        updateBackgroundZoneMenuVisibility(player->hasServerBackgroundZone());

        customZonesMenu = new CustomZoneMenu(player);
        playerMenu->addMenu(customZonesMenu);
        playerMenu->addSeparator();

        countersMenu = playerMenu->addMenu(QString());

        utilityMenu = new UtilityMenu(player, playerMenu);

        // Build zone menu collection for batch shortcut operations
        allZoneMenus = {handMenu,        libraryMenu,       graveMenu,          sideboardMenu, commandZoneMenu,
                        partnerZoneMenu, companionZoneMenu, backgroundZoneMenu, utilityMenu};
    } else {
        sideboardMenu = nullptr;
        commandZoneMenu = nullptr;
        partnerZoneMenu = nullptr;
        companionZoneMenu = nullptr;
        backgroundZoneMenu = nullptr;
        customZonesMenu = nullptr;
        countersMenu = nullptr;
        utilityMenu = nullptr;

        // Non-local players only have graveMenu
        allZoneMenus = {graveMenu};
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
    auto *graphicsItem = player->getGraphicsItem();
    if (!graphicsItem) {
        return;
    }

    graphicsItem->getTableZoneGraphicsItem()->setMenu(playerMenu);
    graphicsItem->getGraveyardZoneGraphicsItem()->setMenu(graveMenu, graveMenu->aViewGraveyard);
    graphicsItem->getRfgZoneGraphicsItem()->setMenu(rfgMenu, rfgMenu->aViewRfg);
    if (player->getPlayerInfo()->getLocalOrJudge()) {
        graphicsItem->getHandZoneGraphicsItem()->setMenu(handMenu);
        graphicsItem->getDeckZoneGraphicsItem()->setMenu(libraryMenu, libraryMenu->aDrawCard);
        graphicsItem->getSideboardZoneGraphicsItem()->setMenu(sideboardMenu);

        // Command zone
        if (auto *commandZone = graphicsItem->getCommandZoneGraphicsItem()) {
            commandZone->setMenu(commandZoneMenu, commandZoneMenu->aViewZone);
        }

        // Partner zone
        if (auto *partnerZone = graphicsItem->getPartnerZoneGraphicsItem()) {
            partnerZone->setMenu(partnerZoneMenu, partnerZoneMenu->aViewZone);
        }

        // Companion zone
        if (auto *companionZone = graphicsItem->getCompanionZoneGraphicsItem()) {
            companionZone->setMenu(companionZoneMenu, companionZoneMenu->aViewZone);
        }

        // Background zone
        if (auto *backgroundZone = graphicsItem->getBackgroundZoneGraphicsItem()) {
            backgroundZone->setMenu(backgroundZoneMenu, backgroundZoneMenu->aViewZone);
        }
    }
}

QMenu *PlayerMenu::updateCardMenu(const CardItem *card)
{
    if (!card) {
        emit cardMenuUpdated(nullptr);
        return nullptr;
    }

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

    for (auto *menu : allZoneMenus) {
        if (menu) {
            menu->retranslateUi();
        }
    }

    rfgMenu->retranslateUi();

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

    if (sayMenu) {
        sayMenu->setTitle(tr("S&ay"));
    }
}

void PlayerMenu::refreshShortcuts()
{
    if (shortcutsActive) {
        if (player->getPlayerInfo()->getLocalOrJudge() && !player->getPlayerInfo()->getLocal()) {
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

    for (auto *menu : allZoneMenus) {
        if (menu) {
            menu->setShortcutsActive();
        }
    }

    QMapIterator<int, AbstractCounter *> counterIterator(player->getCounters());
    while (counterIterator.hasNext()) {
        counterIterator.next().value()->setShortcutsActive();
    }
}

void PlayerMenu::setShortcutsInactive()
{
    shortcutsActive = false;

    for (auto *menu : allZoneMenus) {
        if (menu) {
            menu->setShortcutsInactive();
        }
    }

    QMapIterator<int, AbstractCounter *> counterIterator(player->getCounters());
    while (counterIterator.hasNext()) {
        counterIterator.next().value()->setShortcutsInactive();
    }
}
