#include "player_menu.h"

#include "../../../client/tabs/tab_game.h"
#include "../../../common/pb/command_reveal_cards.pb.h"
#include "../../board/card_item.h"
#include "../../cards/card_database_manager.h"
#include "../../zones/hand_zone.h"
#include "../card_menu_action_type.h"
#include "../player_actions.h"
#include "card_menu.h"
#include "hand_menu.h"

PlayerMenu::PlayerMenu(Player *_player) : player(_player)
{

    if (player->getPlayerInfo()->local || player->getPlayerInfo()->judge) {
        connect(player->getGame()->getPlayerManager(), &PlayerManager::playerAdded, this, &PlayerMenu::addPlayer);
        connect(player->getGame()->getPlayerManager(), &PlayerManager::playerRemoved, this, &PlayerMenu::removePlayer);
    }

    const QList<Player *> &players = player->getGame()->getPlayerManager()->getPlayers().values();
    for (const auto playerToAdd : players) {
        addPlayer(playerToAdd);
    }

    PlayerActions *playerActions = player->getPlayerActions();

    createViewActions();

    playerMenu = new TearOffMenu();

    if (player->getPlayerInfo()->local || player->getPlayerInfo()->judge) {
        handMenu = new HandMenu(player, player->getPlayerActions(), playerMenu);
        playerMenu->addMenu(handMenu);
        /*playerLists.append(mRevealHand = handMenu->addMenu(QString()));
        playerLists.append(mRevealRandomHandCard = handMenu->addMenu(QString()));*/

        libraryMenu = new LibraryMenu(player, playerMenu);
        playerMenu->addMenu(libraryMenu);
    }

    graveMenu = new GraveyardMenu(player, playerMenu);
    playerMenu->addMenu(graveMenu);

    rfgMenu = new RfgMenu(player, playerMenu);
    playerMenu->addMenu(rfgMenu);

    if (player->getPlayerInfo()->local || player->getPlayerInfo()->judge) {
        sbMenu = playerMenu->addMenu(QString());
        sbMenu->addAction(aViewSideboard);

        mCustomZones = playerMenu->addMenu(QString());
        mCustomZones->menuAction()->setVisible(false);

        aUntapAll = new QAction(this);
        connect(aUntapAll, &QAction::triggered, playerActions, &PlayerActions::actUntapAll);

        aRollDie = new QAction(this);
        connect(aRollDie, &QAction::triggered, playerActions, &PlayerActions::actRollDie);

        aCreateToken = new QAction(this);
        connect(aCreateToken, &QAction::triggered, playerActions, &PlayerActions::actCreateToken);

        aCreateAnotherToken = new QAction(this);
        connect(aCreateAnotherToken, &QAction::triggered, playerActions, &PlayerActions::actCreateAnotherToken);
        aCreateAnotherToken->setEnabled(false);

        aIncrementAllCardCounters = new QAction(this);
        connect(aIncrementAllCardCounters, &QAction::triggered, player, &Player::incrementAllCardCounters);

        createPredefinedTokenMenu = new QMenu(QString());
        createPredefinedTokenMenu->setEnabled(false);

        playerMenu->addSeparator();
        countersMenu = playerMenu->addMenu(QString());
        playerMenu->addAction(aIncrementAllCardCounters);
        playerMenu->addSeparator();
        playerMenu->addAction(aUntapAll);
        playerMenu->addSeparator();
        playerMenu->addAction(aRollDie);
        playerMenu->addSeparator();
        playerMenu->addAction(aCreateToken);
        playerMenu->addAction(aCreateAnotherToken);
        playerMenu->addMenu(createPredefinedTokenMenu);
        playerMenu->addSeparator();
    }

    if (player->getPlayerInfo()->local) {
        sayMenu = playerMenu->addMenu(QString());
        connect(&SettingsCache::instance().messages(), &MessageSettings::messageMacrosChanged, this,
                &PlayerMenu::initSayMenu);
        initSayMenu();
    }

    if (player->getPlayerInfo()->local || player->getPlayerInfo()->judge) {

        for (auto &playerList : playerLists) {
            QAction *newAction = playerList->addAction(QString());
            newAction->setData(-1);
            connect(newAction, &QAction::triggered, this, &PlayerMenu::playerListActionTriggered);
            allPlayersActions.append(newAction);
            playerList->addSeparator();
        }
    }

    // We have to explicitly not-instantiate a bunch of things if we are not local or a judge or else we have to
    // consider it everywhere instead of just null-checking
    if (!player->getPlayerInfo()->local && !player->getPlayerInfo()->judge) {
        countersMenu = nullptr;
        sbMenu = nullptr;
        mCustomZones = nullptr;
        aCreateAnotherToken = nullptr;
        createPredefinedTokenMenu = nullptr;
        aIncrementAllCardCounters = nullptr;

        aViewSideboard = nullptr;
        handMenu = nullptr;
        sbMenu = nullptr;
        libraryMenu = nullptr;

        aUntapAll = nullptr;
        aRollDie = nullptr;
    }

    connect(&SettingsCache::instance().shortcuts(), &ShortcutsSettings::shortCutChanged, this,
            &PlayerMenu::refreshShortcuts);
    refreshShortcuts();

    connect(player, &Player::clearCustomZonesMenu, this, &PlayerMenu::clearCustomZonesMenu);
    connect(player, &Player::addViewCustomZoneActionToCustomZoneMenu, this,
            &PlayerMenu::addViewCustomZoneActionToCustomZoneMenu);

    retranslateUi();
}

void PlayerMenu::setMenusForGraphicItems()
{
    player->getGraphicsItem()->getTableZoneGraphicsItem()->setMenu(playerMenu);
    // player->getGraphicsItem()->getGraveyardZoneGraphicsItem()->setMenu(graveMenu, aViewGraveyard);
    // player->getGraphicsItem()->getRfgZoneGraphicsItem()->setMenu(rfgMenu, aViewRfg);
    if (player->getPlayerInfo()->local || player->getPlayerInfo()->judge) {
        player->getGraphicsItem()->getHandZoneGraphicsItem()->setMenu(handMenu);
        // player->getGraphicsItem()->getDeckZoneGraphicsItem()->setMenu(libraryMenu, aDrawCard);
        player->getGraphicsItem()->getSideboardZoneGraphicsItem()->setMenu(sbMenu);
    }
}

void PlayerMenu::createViewActions()
{
    PlayerActions *playerActions = player->getPlayerActions();

    if (player->getPlayerInfo()->local || player->getPlayerInfo()->judge) {
        aViewSideboard = new QAction(this);
        connect(aViewSideboard, &QAction::triggered, playerActions, &PlayerActions::actViewSideboard);
    }
}

void PlayerMenu::refreshShortcuts()
{
    if (shortcutsActive) {
        setShortcutsActive();
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
    for (auto &playerList : singlePlayerLists) {
        addPlayerToList(playerList, playerToAdd);
    }

    playersInfo.append(qMakePair(playerToAdd->getPlayerInfo()->getName(), playerToAdd->getPlayerInfo()->getId()));
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
    for (auto &playerList : singlePlayerLists) {
        removePlayerFromList(playerList, playerToRemove);
    }

    for (auto it = playersInfo.begin(); it != playersInfo.end();) {
        if (it->second == playerToRemove->getPlayerInfo()->getId()) {
            it = playersInfo.erase(it);
        } else {
            ++it;
        }
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

    QMenu *menu = new CardMenu(player, card, playersInfo, shortcutsActive);
    emit cardMenuUpdated(menu);

    return menu;
}

void PlayerMenu::clearCustomZonesMenu()
{
    // Can be null if we are not the local player!
    if (mCustomZones) {
        mCustomZones->clear();
        mCustomZones->menuAction()->setVisible(false);
    }
}

void PlayerMenu::addViewCustomZoneActionToCustomZoneMenu(QString zoneName)
{
    if (mCustomZones) {
        mCustomZones->menuAction()->setVisible(true);
        QAction *aViewZone = mCustomZones->addAction(tr("View custom zone '%1'").arg(zoneName));
        aViewZone->setData(zoneName);
        connect(aViewZone, &QAction::triggered, this,
                [zoneName, this]() { player->getGameScene()->toggleZoneView(player, zoneName, -1); });
    }
}

void PlayerMenu::populatePredefinedTokensMenu()
{
    DeckLoader *_deck = player->getDeck();
    createPredefinedTokenMenu->clear();
    createPredefinedTokenMenu->setEnabled(false);
    predefinedTokens.clear();
    InnerDecklistNode *tokenZone = dynamic_cast<InnerDecklistNode *>(_deck->getRoot()->findChild(DECK_ZONE_TOKENS));

    if (tokenZone) {
        if (!tokenZone->empty())
            createPredefinedTokenMenu->setEnabled(true);

        for (int i = 0; i < tokenZone->size(); ++i) {
            const QString tokenName = tokenZone->at(i)->getName();
            predefinedTokens.append(tokenName);
            QAction *a = createPredefinedTokenMenu->addAction(tokenName);
            if (i < 10) {
                a->setShortcut(QKeySequence("Alt+" + QString::number((i + 1) % 10)));
            }
            connect(a, &QAction::triggered, player->getPlayerActions(), &PlayerActions::actCreatePredefinedToken);
        }
    }
}

void PlayerMenu::retranslateUi()
{
    playerMenu->setTitle(tr("Player \"%1\"").arg(player->getPlayerInfo()->getName()));
    graveMenu->retranslateUi();
    rfgMenu->retranslateUi();

    if (player->getPlayerInfo()->getLocalOrJudge()) {
        aViewSideboard->setText(tr("&View sideboard"));

        handMenu->retranslateUi();
        sbMenu->setTitle(tr("&Sideboard"));
        libraryMenu->retranslateUi();
        countersMenu->setTitle(tr("&Counters"));
        mCustomZones->setTitle(tr("C&ustom Zones"));

        for (auto aViewZone : mCustomZones->actions()) {
            aViewZone->setText(tr("View custom zone '%1'").arg(aViewZone->data().toString()));
        }

        aIncrementAllCardCounters->setText(tr("Increment all card counters"));
        aUntapAll->setText(tr("&Untap all permanents"));
        aRollDie->setText(tr("R&oll die..."));
        aCreateToken->setText(tr("&Create token..."));
        aCreateAnotherToken->setText(tr("C&reate another token"));
        createPredefinedTokenMenu->setTitle(tr("Cr&eate predefined token"));

        for (auto &allPlayersAction : allPlayersActions) {
            allPlayersAction->setText(tr("&All players"));
        }
    }

    if (player->getPlayerInfo()->getLocal()) {
        sayMenu->setTitle(tr("S&ay"));
    }
}

void PlayerMenu::setShortcutIfItExists(QAction *action, ShortcutKey shortcut)
{
    if (action) {
        action->setShortcuts(shortcut);
    }
}

void PlayerMenu::clearShortcutIfItExists(QAction *action)
{
    if (action) {
        action->setShortcut(QKeySequence());
    }
}

void PlayerMenu::setShortcutsActive()
{
    shortcutsActive = true;
    ShortcutsSettings &shortcuts = SettingsCache::instance().shortcuts();

    QMapIterator<int, AbstractCounter *> counterIterator(player->getCounters());
    while (counterIterator.hasNext()) {
        counterIterator.next().value()->setShortcutsActive();
    }

    setShortcutIfItExists(aIncrementAllCardCounters, shortcuts.getShortcut("Player/aIncrementAllCardCounters"));
    setShortcutIfItExists(aViewSideboard, shortcuts.getShortcut("Player/aViewSideboard"));
    setShortcutIfItExists(aUntapAll, shortcuts.getShortcut("Player/aUntapAll"));
    setShortcutIfItExists(aRollDie, shortcuts.getShortcut("Player/aRollDie"));
    setShortcutIfItExists(aCreateToken, shortcuts.getShortcut("Player/aCreateToken"));
    setShortcutIfItExists(aCreateAnotherToken, shortcuts.getShortcut("Player/aCreateAnotherToken"));
}

void PlayerMenu::setShortcutsInactive()
{
    shortcutsActive = false;

    clearShortcutIfItExists(aViewSideboard);
    clearShortcutIfItExists(aUntapAll);
    clearShortcutIfItExists(aRollDie);
    clearShortcutIfItExists(aCreateToken);
    clearShortcutIfItExists(aCreateAnotherToken);
    clearShortcutIfItExists(aIncrementAllCardCounters);

    QMapIterator<int, AbstractCounter *> counterIterator(player->getCounters());
    while (counterIterator.hasNext()) {
        counterIterator.next().value()->setShortcutsInactive();
    }
}

void PlayerMenu::initSayMenu()
{
    sayMenu->clear();

    int count = SettingsCache::instance().messages().getCount();
    sayMenu->setEnabled(count > 0);

    for (int i = 0; i < count; ++i) {
        auto *newAction = new QAction(SettingsCache::instance().messages().getMessageAt(i), sayMenu);
        if (i < 10) {
            newAction->setShortcut(QKeySequence("Ctrl+" + QString::number((i + 1) % 10)));
        }
        connect(newAction, &QAction::triggered, player->getPlayerActions(), &PlayerActions::actSayMessage);
        sayMenu->addAction(newAction);
    }
}
