#include "player_menu.h"

#include "../../../interface/widgets/tabs/tab_game.h"
#include "../../board/card_item.h"
#include "../../game_meta_info.h"
#include "../../zones/hand_zone.h"
#include "../../zones/pile_zone.h"
#include "../../zones/table_zone.h"
#include "card_menu.h"
#include "hand_menu.h"

#include <libcockatrice/protocol/pb/command_reveal_cards.pb.h>

PlayerMenu::PlayerMenu(Player *_player) : player(_player)
{
    playerMenu = new TearOffMenu();

    if (player->getPlayerInfo()->getLocalOrJudge()) {
        handMenu = addManagedMenu<HandMenu>(player, player->getPlayerActions(), playerMenu);
        libraryMenu = addManagedMenu<LibraryMenu>(player, playerMenu);
    } else {
        handMenu = nullptr;
        libraryMenu = nullptr;
    }

    graveMenu = addManagedMenu<GraveyardMenu>(player, playerMenu);
    rfgMenu = addManagedMenu<RfgMenu>(player, playerMenu);

    if (player->getPlayerInfo()->getLocalOrJudge()) {
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

    if (player->getPlayerInfo()->getLocal()) {
        sayMenu = addManagedMenu<SayMenu>(player);
    } else {
        sayMenu = nullptr;
    }

    connect(&SettingsCache::instance().shortcuts(), &ShortcutsSettings::shortCutChanged, this,
            &PlayerMenu::refreshShortcuts);

    // Monitor game state to re-evaluate shortcuts when game starts/stops
    if (player->getGame() && player->getGame()->getGameMetaInfo()) {
        connect(player->getGame()->getGameMetaInfo(), &GameMetaInfo::startedChanged, this,
                &PlayerMenu::onGameStartedChanged);
    }

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

    for (auto *component : managedComponents) {
        component->retranslateUi();
    }

    if (countersMenu) {
        countersMenu->setTitle(tr("&Counters"));
    }

    QMapIterator<int, AbstractCounter *> counterIterator(player->getCounters());
    while (counterIterator.hasNext()) {
        counterIterator.next().value()->retranslateUi();
    }
}

void PlayerMenu::refreshShortcuts()
{
    if (shortcutsActive) {
        // Judges get access to every player's menus but only want shortcuts to be set for their own.
        if (player->getPlayerInfo()->getLocalOrJudge() && !player->getPlayerInfo()->getLocal()) {
            setShortcutsInactive();
        } else {
            setShortcutsActive();
        }
    } else {
        setShortcutsInactive();
    }
}

void PlayerMenu::onGameStartedChanged(bool started)
{
    Q_UNUSED(started);
    // Re-evaluate shortcuts when game state transitions
    if (shortcutsActive) {
        setShortcutsActive();
    }
}

void PlayerMenu::setShortcutsActive()
{
    shortcutsActive = true;

    // Null-safety checks
    if (!player->getGame() || !player->getGame()->getGameMetaInfo()) {
        return;
    }

    if (!player->getGame()->getGameMetaInfo()->started()) {
        for (auto *component : managedComponents) {
            component->setShortcutsInactive();
        }

        QMapIterator<int, AbstractCounter *> counterIterator(player->getCounters());
        while (counterIterator.hasNext()) {
            counterIterator.next().value()->setShortcutsInactive();
        }

        if (utilityMenu) {
            utilityMenu->setLobbyShortcutsActive();
        }
        return;
    }

    for (auto *component : managedComponents) {
        component->setShortcutsActive();
    }

    // Counters implement AbstractPlayerComponent but are iterated via Player::counters
    // (the authoritative source) rather than managedComponents to avoid a redundant
    // list that must stay in sync with the map.
    QMapIterator<int, AbstractCounter *> counterIterator(player->getCounters());
    while (counterIterator.hasNext()) {
        counterIterator.next().value()->setShortcutsActive();
    }
}

void PlayerMenu::setShortcutsInactive()
{
    shortcutsActive = false;

    for (auto *component : managedComponents) {
        component->setShortcutsInactive();
    }

    QMapIterator<int, AbstractCounter *> counterIterator(player->getCounters());
    while (counterIterator.hasNext()) {
        counterIterator.next().value()->setShortcutsInactive();
    }
}