#include "player_menu.h"

#include "../../../common/pb/command_reveal_cards.pb.h"
#include "../../client/tabs/tab_game.h"
#include "../../settings/card_counter_settings.h"
#include "../board/card_item.h"
#include "../cards/card_database_manager.h"
#include "../zones/hand_zone.h"
#include "../zones/logic/view_zone_logic.h"
#include "card_menu_action_type.h"
#include "player_actions.h"

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
    if (player->getPlayerInfo()->local || player->getPlayerInfo()->judge) {
        aMoveHandToTopLibrary = new QAction(this);
        aMoveHandToTopLibrary->setData(QList<QVariant>() << "deck" << 0);
        aMoveHandToBottomLibrary = new QAction(this);
        aMoveHandToBottomLibrary->setData(QList<QVariant>() << "deck" << -1);
        aMoveHandToGrave = new QAction(this);
        aMoveHandToGrave->setData(QList<QVariant>() << "grave" << 0);
        aMoveHandToRfg = new QAction(this);
        aMoveHandToRfg->setData(QList<QVariant>() << "rfg" << 0);

        auto hand = player->getHandZone();
        auto grave = player->getGraveZone();
        auto rfg = player->getRfgZone();

        connect(aMoveHandToTopLibrary, &QAction::triggered, hand, &HandZoneLogic::moveAllToZone);
        connect(aMoveHandToBottomLibrary, &QAction::triggered, hand, &HandZoneLogic::moveAllToZone);
        connect(aMoveHandToGrave, &QAction::triggered, hand, &HandZoneLogic::moveAllToZone);
        connect(aMoveHandToRfg, &QAction::triggered, hand, &HandZoneLogic::moveAllToZone);

        aMoveGraveToTopLibrary = new QAction(this);
        aMoveGraveToTopLibrary->setData(QList<QVariant>() << "deck" << 0);
        aMoveGraveToBottomLibrary = new QAction(this);
        aMoveGraveToBottomLibrary->setData(QList<QVariant>() << "deck" << -1);
        aMoveGraveToHand = new QAction(this);
        aMoveGraveToHand->setData(QList<QVariant>() << "hand" << 0);
        aMoveGraveToRfg = new QAction(this);
        aMoveGraveToRfg->setData(QList<QVariant>() << "rfg" << 0);

        connect(aMoveGraveToTopLibrary, &QAction::triggered, grave, &PileZoneLogic::moveAllToZone);
        connect(aMoveGraveToBottomLibrary, &QAction::triggered, grave, &PileZoneLogic::moveAllToZone);
        connect(aMoveGraveToHand, &QAction::triggered, grave, &PileZoneLogic::moveAllToZone);
        connect(aMoveGraveToRfg, &QAction::triggered, grave, &PileZoneLogic::moveAllToZone);

        aMoveRfgToTopLibrary = new QAction(this);
        aMoveRfgToTopLibrary->setData(QList<QVariant>() << "deck" << 0);
        aMoveRfgToBottomLibrary = new QAction(this);
        aMoveRfgToBottomLibrary->setData(QList<QVariant>() << "deck" << -1);
        aMoveRfgToHand = new QAction(this);
        aMoveRfgToHand->setData(QList<QVariant>() << "hand" << 0);
        aMoveRfgToGrave = new QAction(this);
        aMoveRfgToGrave->setData(QList<QVariant>() << "grave" << 0);

        connect(aMoveRfgToTopLibrary, &QAction::triggered, rfg, &PileZoneLogic::moveAllToZone);
        connect(aMoveRfgToBottomLibrary, &QAction::triggered, rfg, &PileZoneLogic::moveAllToZone);
        connect(aMoveRfgToHand, &QAction::triggered, rfg, &PileZoneLogic::moveAllToZone);
        connect(aMoveRfgToGrave, &QAction::triggered, rfg, &PileZoneLogic::moveAllToZone);

        aViewLibrary = new QAction(this);
        connect(aViewLibrary, &QAction::triggered, playerActions, &PlayerActions::actViewLibrary);
        aViewHand = new QAction(this);
        connect(aViewHand, &QAction::triggered, playerActions, &PlayerActions::actViewHand);
        aSortHand = new QAction(this);
        connect(aSortHand, &QAction::triggered, playerActions, &PlayerActions::actSortHand);

        aViewTopCards = new QAction(this);
        connect(aViewTopCards, &QAction::triggered, playerActions, &PlayerActions::actViewTopCards);
        aViewBottomCards = new QAction(this);
        connect(aViewBottomCards, &QAction::triggered, playerActions, &PlayerActions::actViewBottomCards);
        aAlwaysRevealTopCard = new QAction(this);
        aAlwaysRevealTopCard->setCheckable(true);
        connect(aAlwaysRevealTopCard, &QAction::triggered, playerActions, &PlayerActions::actAlwaysRevealTopCard);
        aAlwaysLookAtTopCard = new QAction(this);
        aAlwaysLookAtTopCard->setCheckable(true);
        connect(aAlwaysLookAtTopCard, &QAction::triggered, playerActions, &PlayerActions::actAlwaysLookAtTopCard);
        aOpenDeckInDeckEditor = new QAction(this);
        aOpenDeckInDeckEditor->setEnabled(false);
        connect(aOpenDeckInDeckEditor, &QAction::triggered, playerActions, &PlayerActions::actOpenDeckInDeckEditor);
    }

    aViewGraveyard = new QAction(this);
    connect(aViewGraveyard, &QAction::triggered, playerActions, &PlayerActions::actViewGraveyard);

    aViewRfg = new QAction(this);
    connect(aViewRfg, &QAction::triggered, playerActions, &PlayerActions::actViewRfg);

    if (player->getPlayerInfo()->local || player->getPlayerInfo()->judge) {
        aViewSideboard = new QAction(this);
        connect(aViewSideboard, &QAction::triggered, playerActions, &PlayerActions::actViewSideboard);

        aDrawCard = new QAction(this);
        connect(aDrawCard, &QAction::triggered, playerActions, &PlayerActions::actDrawCard);
        aDrawCards = new QAction(this);
        connect(aDrawCards, &QAction::triggered, playerActions, &PlayerActions::actDrawCards);
        aUndoDraw = new QAction(this);
        connect(aUndoDraw, &QAction::triggered, playerActions, &PlayerActions::actUndoDraw);

        aShuffle = new QAction(this);
        connect(aShuffle, &QAction::triggered, playerActions, &PlayerActions::actShuffle);

        aMulligan = new QAction(this);
        connect(aMulligan, &QAction::triggered, playerActions, &PlayerActions::actMulligan);

        aMoveTopToPlay = new QAction(this);
        connect(aMoveTopToPlay, &QAction::triggered, playerActions, &PlayerActions::actMoveTopCardToPlay);
        aMoveTopToPlayFaceDown = new QAction(this);
        connect(aMoveTopToPlayFaceDown, &QAction::triggered, playerActions,
                &PlayerActions::actMoveTopCardToPlayFaceDown);
        aMoveTopCardToGraveyard = new QAction(this);
        connect(aMoveTopCardToGraveyard, &QAction::triggered, playerActions, &PlayerActions::actMoveTopCardToGrave);
        aMoveTopCardToExile = new QAction(this);
        connect(aMoveTopCardToExile, &QAction::triggered, playerActions, &PlayerActions::actMoveTopCardToExile);
        aMoveTopCardsToGraveyard = new QAction(this);
        connect(aMoveTopCardsToGraveyard, &QAction::triggered, playerActions, &PlayerActions::actMoveTopCardsToGrave);
        aMoveTopCardsToExile = new QAction(this);
        connect(aMoveTopCardsToExile, &QAction::triggered, playerActions, &PlayerActions::actMoveTopCardsToExile);
        aMoveTopCardsUntil = new QAction(this);
        connect(aMoveTopCardsUntil, &QAction::triggered, playerActions, &PlayerActions::actMoveTopCardsUntil);
        aMoveTopCardToBottom = new QAction(this);
        connect(aMoveTopCardToBottom, &QAction::triggered, playerActions, &PlayerActions::actMoveTopCardToBottom);

        aShuffleTopCards = new QAction(this);
        connect(aShuffleTopCards, &QAction::triggered, playerActions, &PlayerActions::actShuffleTop);

        aDrawBottomCard = new QAction(this);
        connect(aDrawBottomCard, &QAction::triggered, playerActions, &PlayerActions::actDrawBottomCard);
        aDrawBottomCards = new QAction(this);
        connect(aDrawBottomCards, &QAction::triggered, playerActions, &PlayerActions::actDrawBottomCards);
        aMoveBottomToPlay = new QAction(this);
        connect(aMoveBottomToPlay, &QAction::triggered, playerActions, &PlayerActions::actMoveBottomCardToPlay);
        aMoveBottomToPlayFaceDown = new QAction(this);
        connect(aMoveBottomToPlayFaceDown, &QAction::triggered, playerActions,
                &PlayerActions::actMoveBottomCardToPlayFaceDown);
        aMoveBottomCardToGraveyard = new QAction(this);
        connect(aMoveBottomCardToGraveyard, &QAction::triggered, playerActions,
                &PlayerActions::actMoveBottomCardToGrave);
        aMoveBottomCardToExile = new QAction(this);
        connect(aMoveBottomCardToExile, &QAction::triggered, playerActions, &PlayerActions::actMoveBottomCardToExile);
        aMoveBottomCardsToGraveyard = new QAction(this);
        connect(aMoveBottomCardsToGraveyard, &QAction::triggered, playerActions,
                &PlayerActions::actMoveBottomCardsToGrave);
        aMoveBottomCardsToExile = new QAction(this);
        connect(aMoveBottomCardsToExile, &QAction::triggered, playerActions, &PlayerActions::actMoveBottomCardsToExile);
        aMoveBottomCardToTop = new QAction(this);
        connect(aMoveBottomCardToTop, &QAction::triggered, playerActions, &PlayerActions::actMoveBottomCardToTop);

        aShuffleBottomCards = new QAction(this);
        connect(aShuffleBottomCards, &QAction::triggered, playerActions, &PlayerActions::actShuffleBottom);
    }

    playerMenu = new TearOffMenu();
    player->getGraphicsItem()->getTableZoneGraphicsItem()->setMenu(playerMenu);

    if (player->getPlayerInfo()->local || player->getPlayerInfo()->judge) {
        handMenu = playerMenu->addTearOffMenu(QString());
        handMenu->addAction(aViewHand);
        handMenu->addAction(aSortHand);
        playerLists.append(mRevealHand = handMenu->addMenu(QString()));
        playerLists.append(mRevealRandomHandCard = handMenu->addMenu(QString()));
        handMenu->addSeparator();
        handMenu->addAction(aMulligan);
        handMenu->addSeparator();
        moveHandMenu = handMenu->addTearOffMenu(QString());
        moveHandMenu->addAction(aMoveHandToTopLibrary);
        moveHandMenu->addAction(aMoveHandToBottomLibrary);
        moveHandMenu->addSeparator();
        moveHandMenu->addAction(aMoveHandToGrave);
        moveHandMenu->addSeparator();
        moveHandMenu->addAction(aMoveHandToRfg);
        player->getGraphicsItem()->getHandZoneGraphicsItem()->setMenu(handMenu);

        libraryMenu = playerMenu->addTearOffMenu(QString());
        libraryMenu->addAction(aDrawCard);
        libraryMenu->addAction(aDrawCards);
        libraryMenu->addAction(aUndoDraw);
        libraryMenu->addSeparator();
        libraryMenu->addAction(aShuffle);
        libraryMenu->addSeparator();
        libraryMenu->addAction(aViewLibrary);
        libraryMenu->addAction(aViewTopCards);
        libraryMenu->addAction(aViewBottomCards);
        libraryMenu->addSeparator();
        playerLists.append(mRevealLibrary = libraryMenu->addMenu(QString()));
        singlePlayerLists.append(mLendLibrary = libraryMenu->addMenu(QString()));
        playerLists.append(mRevealTopCard = libraryMenu->addMenu(QString()));
        libraryMenu->addAction(aAlwaysRevealTopCard);
        libraryMenu->addAction(aAlwaysLookAtTopCard);
        libraryMenu->addSeparator();
        topLibraryMenu = libraryMenu->addTearOffMenu(QString());
        bottomLibraryMenu = libraryMenu->addTearOffMenu(QString());
        libraryMenu->addSeparator();
        libraryMenu->addAction(aOpenDeckInDeckEditor);
        player->getGraphicsItem()->getDeckZoneGraphicsItem()->setMenu(libraryMenu, aDrawCard);
        topLibraryMenu->addAction(aMoveTopToPlay);
        topLibraryMenu->addAction(aMoveTopToPlayFaceDown);
        topLibraryMenu->addAction(aMoveTopCardToBottom);
        topLibraryMenu->addSeparator();
        topLibraryMenu->addAction(aMoveTopCardToGraveyard);
        topLibraryMenu->addAction(aMoveTopCardsToGraveyard);
        topLibraryMenu->addAction(aMoveTopCardToExile);
        topLibraryMenu->addAction(aMoveTopCardsToExile);
        topLibraryMenu->addAction(aMoveTopCardsUntil);
        topLibraryMenu->addSeparator();
        topLibraryMenu->addAction(aShuffleTopCards);

        bottomLibraryMenu->addAction(aDrawBottomCard);
        bottomLibraryMenu->addAction(aDrawBottomCards);
        bottomLibraryMenu->addSeparator();
        bottomLibraryMenu->addAction(aMoveBottomToPlay);
        bottomLibraryMenu->addAction(aMoveBottomToPlayFaceDown);
        bottomLibraryMenu->addAction(aMoveBottomCardToTop);
        bottomLibraryMenu->addSeparator();
        bottomLibraryMenu->addAction(aMoveBottomCardToGraveyard);
        bottomLibraryMenu->addAction(aMoveBottomCardsToGraveyard);
        bottomLibraryMenu->addAction(aMoveBottomCardToExile);
        bottomLibraryMenu->addAction(aMoveBottomCardsToExile);
        bottomLibraryMenu->addSeparator();
        bottomLibraryMenu->addAction(aShuffleBottomCards);
    }

    graveMenu = playerMenu->addTearOffMenu(QString());
    graveMenu->addAction(aViewGraveyard);

    if (player->getPlayerInfo()->local || player->getPlayerInfo()->judge) {
        mRevealRandomGraveyardCard = graveMenu->addMenu(QString());
        QAction *newAction = mRevealRandomGraveyardCard->addAction(QString());
        newAction->setData(-1);
        connect(newAction, &QAction::triggered, playerActions, &PlayerActions::actRevealRandomGraveyardCard);
        allPlayersActions.append(newAction);
        mRevealRandomGraveyardCard->addSeparator();
    }
    player->getGraphicsItem()->getGraveyardZoneGraphicsItem()->setMenu(graveMenu, aViewGraveyard);

    rfgMenu = playerMenu->addTearOffMenu(QString());
    rfgMenu->addAction(aViewRfg);
    player->getGraphicsItem()->getRfgZoneGraphicsItem()->setMenu(rfgMenu, aViewRfg);

    if (player->getPlayerInfo()->local || player->getPlayerInfo()->judge) {
        graveMenu->addSeparator();
        moveGraveMenu = graveMenu->addTearOffMenu(QString());
        moveGraveMenu->addAction(aMoveGraveToTopLibrary);
        moveGraveMenu->addAction(aMoveGraveToBottomLibrary);
        moveGraveMenu->addSeparator();
        moveGraveMenu->addAction(aMoveGraveToHand);
        moveGraveMenu->addSeparator();
        moveGraveMenu->addAction(aMoveGraveToRfg);

        rfgMenu->addSeparator();
        moveRfgMenu = rfgMenu->addTearOffMenu(QString());
        moveRfgMenu->addAction(aMoveRfgToTopLibrary);
        moveRfgMenu->addAction(aMoveRfgToBottomLibrary);
        moveRfgMenu->addSeparator();
        moveRfgMenu->addAction(aMoveRfgToHand);
        moveRfgMenu->addSeparator();
        moveRfgMenu->addAction(aMoveRfgToGrave);

        sbMenu = playerMenu->addMenu(QString());
        sbMenu->addAction(aViewSideboard);
        // sb->setMenu(sbMenu, aViewSideboard);

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

        mCardCounters = new QMenu;

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

    if (!player->getPlayerInfo()->local && !player->getPlayerInfo()->judge) {
        countersMenu = nullptr;
        sbMenu = nullptr;
        mCustomZones = nullptr;
        aCreateAnotherToken = nullptr;
        createPredefinedTokenMenu = nullptr;
        mCardCounters = nullptr;
    }

    aTap = new QAction(this);
    aTap->setData(cmTap);
    connect(aTap, &QAction::triggered, playerActions, &PlayerActions::cardMenuAction);
    aDoesntUntap = new QAction(this);
    aDoesntUntap->setData(cmDoesntUntap);
    connect(aDoesntUntap, &QAction::triggered, playerActions, &PlayerActions::cardMenuAction);
    aAttach = new QAction(this);
    connect(aAttach, &QAction::triggered, playerActions, &PlayerActions::actAttach);
    aUnattach = new QAction(this);
    connect(aUnattach, &QAction::triggered, playerActions, &PlayerActions::actUnattach);
    aDrawArrow = new QAction(this);
    connect(aDrawArrow, &QAction::triggered, playerActions, &PlayerActions::actDrawArrow);
    aIncP = new QAction(this);
    connect(aIncP, &QAction::triggered, playerActions, &PlayerActions::actIncP);
    aDecP = new QAction(this);
    connect(aDecP, &QAction::triggered, playerActions, &PlayerActions::actDecP);
    aIncT = new QAction(this);
    connect(aIncT, &QAction::triggered, playerActions, &PlayerActions::actIncT);
    aDecT = new QAction(this);
    connect(aDecT, &QAction::triggered, playerActions, &PlayerActions::actDecT);
    aIncPT = new QAction(this);
    // connect(aIncPT, &QAction::triggered, playerActions, [this] { actIncPT(); });
    aDecPT = new QAction(this);
    connect(aDecPT, &QAction::triggered, playerActions, &PlayerActions::actDecPT);
    aFlowP = new QAction(this);
    connect(aFlowP, &QAction::triggered, playerActions, &PlayerActions::actFlowP);
    aFlowT = new QAction(this);
    connect(aFlowT, &QAction::triggered, playerActions, &PlayerActions::actFlowT);
    aSetPT = new QAction(this);
    connect(aSetPT, &QAction::triggered, playerActions, &PlayerActions::actSetPT);
    aResetPT = new QAction(this);
    connect(aResetPT, &QAction::triggered, playerActions, &PlayerActions::actResetPT);
    aSetAnnotation = new QAction(this);
    connect(aSetAnnotation, &QAction::triggered, playerActions, &PlayerActions::actSetAnnotation);
    aFlip = new QAction(this);
    aFlip->setData(cmFlip);
    connect(aFlip, &QAction::triggered, player->getPlayerActions(), &PlayerActions::cardMenuAction);
    aPeek = new QAction(this);
    aPeek->setData(cmPeek);
    connect(aPeek, &QAction::triggered, player->getPlayerActions(), &PlayerActions::cardMenuAction);
    aClone = new QAction(this);
    aClone->setData(cmClone);
    connect(aClone, &QAction::triggered, player->getPlayerActions(), &PlayerActions::cardMenuAction);
    aMoveToTopLibrary = new QAction(this);
    aMoveToTopLibrary->setData(cmMoveToTopLibrary);
    aMoveToBottomLibrary = new QAction(this);
    aMoveToBottomLibrary->setData(cmMoveToBottomLibrary);
    aMoveToXfromTopOfLibrary = new QAction(this);
    aMoveToGraveyard = new QAction(this);
    aMoveToHand = new QAction(this);
    aMoveToHand->setData(cmMoveToHand);
    aMoveToGraveyard->setData(cmMoveToGraveyard);
    aMoveToExile = new QAction(this);
    aMoveToExile->setData(cmMoveToExile);
    connect(aMoveToTopLibrary, &QAction::triggered, player->getPlayerActions(), &PlayerActions::cardMenuAction);
    connect(aMoveToBottomLibrary, &QAction::triggered, player->getPlayerActions(), &PlayerActions::cardMenuAction);
    connect(aMoveToXfromTopOfLibrary, &QAction::triggered, playerActions, &PlayerActions::actMoveCardXCardsFromTop);
    connect(aMoveToHand, &QAction::triggered, player->getPlayerActions(), &PlayerActions::cardMenuAction);
    connect(aMoveToGraveyard, &QAction::triggered, player->getPlayerActions(), &PlayerActions::cardMenuAction);
    connect(aMoveToExile, &QAction::triggered, player->getPlayerActions(), &PlayerActions::cardMenuAction);

    aSelectAll = new QAction(this);
    connect(aSelectAll, &QAction::triggered, playerActions, &PlayerActions::actSelectAll);
    aSelectRow = new QAction(this);
    connect(aSelectRow, &QAction::triggered, playerActions, &PlayerActions::actSelectRow);
    aSelectColumn = new QAction(this);
    connect(aSelectColumn, &QAction::triggered, playerActions, &PlayerActions::actSelectColumn);

    aPlay = new QAction(this);
    connect(aPlay, &QAction::triggered, playerActions, &PlayerActions::actPlay);
    aHide = new QAction(this);
    connect(aHide, &QAction::triggered, playerActions, &PlayerActions::actHide);
    aPlayFacedown = new QAction(this);
    connect(aPlayFacedown, &QAction::triggered, playerActions, &PlayerActions::actPlayFacedown);

    for (int i = 0; i < 6; ++i) {
        auto *tempAddCounter = new QAction(this);
        tempAddCounter->setData(9 + i * 1000);
        auto *tempRemoveCounter = new QAction(this);
        tempRemoveCounter->setData(10 + i * 1000);
        auto *tempSetCounter = new QAction(this);
        tempSetCounter->setData(11 + i * 1000);
        aAddCounter.append(tempAddCounter);
        aRemoveCounter.append(tempRemoveCounter);
        aSetCounter.append(tempSetCounter);
        connect(tempAddCounter, &QAction::triggered, playerActions, &PlayerActions::actCardCounterTrigger);
        connect(tempRemoveCounter, &QAction::triggered, playerActions, &PlayerActions::actCardCounterTrigger);
        connect(tempSetCounter, &QAction::triggered, playerActions, &PlayerActions::actCardCounterTrigger);
    }

    connect(&SettingsCache::instance().shortcuts(), &ShortcutsSettings::shortCutChanged, this,
            &PlayerMenu::refreshShortcuts);
    refreshShortcuts();

    connect(player, &Player::clearCustomZonesMenu, this, &PlayerMenu::clearCustomZonesMenu);
    connect(player, &Player::addViewCustomZoneActionToCustomZoneMenu, this,
            &PlayerMenu::addViewCustomZoneActionToCustomZoneMenu);
    connect(player, &Player::resetTopCardMenuActions, this, &PlayerMenu::resetTopCardMenuActions);

    retranslateUi();
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

    if (menu == mRevealLibrary || menu == mLendLibrary) {
        cmd.set_zone_name("deck");
        cmd.set_grant_write_access(menu == mLendLibrary);
    } else if (menu == mRevealTopCard) {
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
    } else if (menu == mRevealHand) {
        cmd.set_zone_name("hand");
    } else if (menu == mRevealRandomHandCard) {
        cmd.set_zone_name("hand");
        cmd.add_card_id(PlayerActions::RANDOM_CARD_FROM_ZONE);
    } else {
        return;
    }

    player->getPlayerActions()->sendGameCommand(cmd);
}

void PlayerMenu::initContextualPlayersMenu(QMenu *menu)
{
    menu->addAction(tr("&All players"))->setData(-1);
    menu->addSeparator();

    for (const auto &playerInfo : playersInfo) {
        menu->addAction(playerInfo.first)->setData(playerInfo.second);
    }
}

QMenu *PlayerMenu::createCardMenu(const CardItem *card)
{
    if (card == nullptr) {
        return nullptr;
    }

    bool revealedCard = false;
    bool writeableCard = player->getPlayerInfo()->getLocalOrJudge();
    if (auto *view = qobject_cast<ZoneViewZoneLogic *>(card->getZone())) {
        if (view->getRevealZone()) {
            if (view->getWriteableRevealZone()) {
                writeableCard = true;
            } else {
                revealedCard = true;
            }
        }
    }

    QMenu *cardMenu = new QMenu;

    if (revealedCard) {
        cardMenu->addAction(aHide);
        cardMenu->addAction(aClone);
        cardMenu->addSeparator();
        cardMenu->addAction(aSelectAll);
        cardMenu->addAction(aSelectColumn);
        addRelatedCardView(card, cardMenu);
    } else if (writeableCard) {
        bool canModifyCard = player->getPlayerInfo()->judge || card->getOwner() == player;

        if (card->getZone()) {
            if (card->getZone()->getName() == "table") {
                // Card is on the battlefield

                if (!canModifyCard) {
                    addRelatedCardView(card, cardMenu);
                    addRelatedCardActions(card, cardMenu);

                    cardMenu->addSeparator();
                    cardMenu->addAction(aDrawArrow);
                    cardMenu->addSeparator();
                    cardMenu->addAction(aClone);
                    cardMenu->addSeparator();
                    cardMenu->addAction(aSelectAll);
                    cardMenu->addAction(aSelectRow);
                    return cardMenu;
                }

                cardMenu->addAction(aTap);
                cardMenu->addAction(aDoesntUntap);
                cardMenu->addAction(aFlip);
                if (card->getFaceDown()) {
                    cardMenu->addAction(aPeek);
                }

                addRelatedCardView(card, cardMenu);
                addRelatedCardActions(card, cardMenu);

                cardMenu->addSeparator();
                cardMenu->addAction(aAttach);
                if (card->getAttachedTo()) {
                    cardMenu->addAction(aUnattach);
                }
                cardMenu->addAction(aDrawArrow);
                cardMenu->addSeparator();
                cardMenu->addMenu(createPtMenu());
                cardMenu->addAction(aSetAnnotation);
                cardMenu->addSeparator();
                cardMenu->addAction(aClone);
                cardMenu->addMenu(createMoveMenu());
                cardMenu->addSeparator();
                cardMenu->addAction(aSelectAll);
                cardMenu->addAction(aSelectRow);

                cardMenu->addSeparator();
                mCardCounters->clear();
                for (int i = 0; i < aAddCounter.size(); ++i) {
                    mCardCounters->addSeparator();
                    mCardCounters->addAction(aAddCounter[i]);
                    if (card->getCounters().contains(i)) {
                        mCardCounters->addAction(aRemoveCounter[i]);
                    }
                    mCardCounters->addAction(aSetCounter[i]);
                }
                cardMenu->addSeparator();
                cardMenu->addMenu(mCardCounters);
            } else if (card->getZone()->getName() == "stack") {
                // Card is on the stack
                if (canModifyCard) {
                    cardMenu->addAction(aAttach);
                    cardMenu->addAction(aDrawArrow);
                    cardMenu->addSeparator();
                    cardMenu->addAction(aClone);
                    cardMenu->addMenu(createMoveMenu());
                    cardMenu->addSeparator();
                    cardMenu->addAction(aSelectAll);
                } else {
                    cardMenu->addAction(aDrawArrow);
                    cardMenu->addSeparator();
                    cardMenu->addAction(aClone);
                    cardMenu->addSeparator();
                    cardMenu->addAction(aSelectAll);
                }

                addRelatedCardView(card, cardMenu);
                addRelatedCardActions(card, cardMenu);
            } else if (card->getZone()->getName() == "rfg" || card->getZone()->getName() == "grave") {
                // Card is in the graveyard or exile
                if (canModifyCard) {
                    cardMenu->addAction(aPlay);
                    cardMenu->addAction(aPlayFacedown);

                    cardMenu->addSeparator();
                    cardMenu->addAction(aClone);
                    cardMenu->addMenu(createMoveMenu());
                    cardMenu->addSeparator();
                    cardMenu->addAction(aSelectAll);
                    cardMenu->addAction(aSelectColumn);

                    cardMenu->addSeparator();
                    cardMenu->addAction(aAttach);
                    cardMenu->addAction(aDrawArrow);
                } else {
                    cardMenu->addAction(aClone);
                    cardMenu->addSeparator();
                    cardMenu->addAction(aSelectAll);
                    cardMenu->addAction(aSelectColumn);
                    cardMenu->addSeparator();
                    cardMenu->addAction(aDrawArrow);
                }

                addRelatedCardView(card, cardMenu);
                addRelatedCardActions(card, cardMenu);
            } else {
                // Card is in hand or a custom zone specified by server
                cardMenu->addAction(aPlay);
                cardMenu->addAction(aPlayFacedown);

                QMenu *revealMenu = cardMenu->addMenu(tr("Re&veal to..."));
                initContextualPlayersMenu(revealMenu);

                connect(revealMenu, &QMenu::triggered, player->getPlayerActions(), &PlayerActions::actReveal);

                cardMenu->addSeparator();
                cardMenu->addAction(aClone);
                cardMenu->addMenu(createMoveMenu());

                // actions that are really wonky when done from deck or sideboard
                if (card->getZone()->getName() == "hand") {
                    cardMenu->addSeparator();
                    cardMenu->addAction(aAttach);
                    cardMenu->addAction(aDrawArrow);
                }

                cardMenu->addSeparator();
                cardMenu->addAction(aSelectAll);
                if (qobject_cast<ZoneViewZoneLogic *>(card->getZone())) {
                    cardMenu->addAction(aSelectColumn);
                }

                addRelatedCardView(card, cardMenu);
                if (card->getZone()->getName() == "hand") {
                    addRelatedCardActions(card, cardMenu);
                }
            }
        } else {
            cardMenu->addMenu(createMoveMenu());
        }
    } else {
        if (card->getZone() && card->getZone()->getName() != "hand") {
            cardMenu->addAction(aDrawArrow);
            cardMenu->addSeparator();
            addRelatedCardView(card, cardMenu);
            addRelatedCardActions(card, cardMenu);
            cardMenu->addSeparator();
            cardMenu->addAction(aClone);
            cardMenu->addSeparator();
            cardMenu->addAction(aSelectAll);
        }
    }

    return cardMenu;
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

    QMenu *menu = createCardMenu(card);
    emit cardMenuUpdated(menu);

    return menu;
}

QMenu *PlayerMenu::createPtMenu() const
{
    QMenu *ptMenu = new QMenu("Power / toughness");
    ptMenu->addAction(aIncP);
    ptMenu->addAction(aDecP);
    ptMenu->addAction(aFlowP);
    ptMenu->addSeparator();
    ptMenu->addAction(aIncT);
    ptMenu->addAction(aDecT);
    ptMenu->addAction(aFlowT);
    ptMenu->addSeparator();
    ptMenu->addAction(aIncPT);
    ptMenu->addAction(aDecPT);
    ptMenu->addSeparator();
    ptMenu->addAction(aSetPT);
    ptMenu->addAction(aResetPT);
    return ptMenu;
}

QMenu *PlayerMenu::createMoveMenu() const
{
    QMenu *moveMenu = new QMenu("Move to");
    moveMenu->addAction(aMoveToTopLibrary);
    moveMenu->addAction(aMoveToXfromTopOfLibrary);
    moveMenu->addAction(aMoveToBottomLibrary);
    moveMenu->addSeparator();
    moveMenu->addAction(aMoveToHand);
    moveMenu->addSeparator();
    moveMenu->addAction(aMoveToGraveyard);
    moveMenu->addSeparator();
    moveMenu->addAction(aMoveToExile);
    return moveMenu;
}

void PlayerMenu::addRelatedCardView(const CardItem *card, QMenu *cardMenu)
{
    if (!card || !cardMenu) {
        return;
    }
    auto exactCard = card->getCard();
    if (!exactCard) {
        return;
    }

    bool atLeastOneGoodRelationFound = false;
    QList<CardRelation *> relatedCards = exactCard.getInfo().getAllRelatedCards();
    for (const CardRelation *cardRelation : relatedCards) {
        CardInfoPtr relatedCard = CardDatabaseManager::getInstance()->getCardInfo(cardRelation->getName());
        if (relatedCard != nullptr) {
            atLeastOneGoodRelationFound = true;
            break;
        }
    }

    if (!atLeastOneGoodRelationFound) {
        return;
    }

    cardMenu->addSeparator();
    auto viewRelatedCards = new QMenu(tr("View related cards"));
    cardMenu->addMenu(viewRelatedCards);
    for (const CardRelation *relatedCard : relatedCards) {
        QString relatedCardName = relatedCard->getName();
        CardRef cardRef = {relatedCardName, exactCard.getPrinting().getUuid()};
        QAction *viewCard = viewRelatedCards->addAction(relatedCardName);
        Q_UNUSED(viewCard);

        connect(viewCard, &QAction::triggered, player->getGame(),
                [this, cardRef] { player->getGame()->getTab()->viewCardInfo(cardRef); });
    }
}

void PlayerMenu::addRelatedCardActions(const CardItem *card, QMenu *cardMenu)
{
    if (!card || !cardMenu) {
        return;
    }
    auto exactCard = card->getCard();
    if (!exactCard) {
        return;
    }

    QList<CardRelation *> relatedCards = exactCard.getInfo().getAllRelatedCards();
    if (relatedCards.isEmpty()) {
        return;
    }

    cardMenu->addSeparator();
    int index = 0;
    QAction *createRelatedCards = nullptr;
    for (const CardRelation *cardRelation : relatedCards) {
        ExactCard relatedCard = CardDatabaseManager::getInstance()->getCardFromSameSet(cardRelation->getName(),
                                                                                       card->getCard().getPrinting());
        if (!relatedCard) {
            relatedCard = CardDatabaseManager::getInstance()->getCard({cardRelation->getName()});
        }
        if (!relatedCard) {
            continue;
        }

        QString relatedCardName;
        if (relatedCard.getInfo().getPowTough().size() > 0) {
            relatedCardName = relatedCard.getInfo().getPowTough() + " " + relatedCard.getName(); // "n/n name"
        } else {
            relatedCardName = relatedCard.getName(); // "name"
        }

        QString text = tr("Token: ");
        if (cardRelation->getDoesAttach()) {
            text +=
                tr(cardRelation->getDoesTransform() ? "Transform into " : "Attach to ") + "\"" + relatedCardName + "\"";
        } else if (cardRelation->getIsVariable()) {
            text += "X " + relatedCardName;
        } else if (cardRelation->getDefaultCount() != 1) {
            text += QString::number(cardRelation->getDefaultCount()) + "x " + relatedCardName;
        } else {
            text += relatedCardName;
        }

        if (createRelatedCards == nullptr) {
            if (relatedCards.length() == 1) {
                createRelatedCards = new QAction(text, this); // set actCreateAllRelatedCards with this text
                break; // do not set an individual entry as there is only one entry
            } else {
                createRelatedCards = new QAction(tr("All tokens"), this);
            }
        }

        auto *createRelated = new QAction(text, this);
        createRelated->setData(QVariant(index++));
        connect(createRelated, &QAction::triggered, player->getPlayerActions(), &PlayerActions::actCreateRelatedCard);
        cardMenu->addAction(createRelated);
    }

    if (createRelatedCards) {
        if (shortcutsActive) {
            createRelatedCards->setShortcuts(
                SettingsCache::instance().shortcuts().getShortcut("Player/aCreateRelatedTokens"));
        }
        connect(createRelatedCards, &QAction::triggered, player->getPlayerActions(),
                &PlayerActions::actCreateAllRelatedCards);
        cardMenu->addAction(createRelatedCards);
    }
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

void PlayerMenu::enableOpenInDeckEditorAction() const
{
    aOpenDeckInDeckEditor->setEnabled(true);
}

void PlayerMenu::resetTopCardMenuActions()
{
    aAlwaysRevealTopCard->setChecked(false);
    aAlwaysLookAtTopCard->setChecked(false);
}

void PlayerMenu::retranslateUi()
{
    aViewGraveyard->setText(tr("&View graveyard"));
    aViewRfg->setText(tr("&View exile"));

    playerMenu->setTitle(tr("Player \"%1\"").arg(player->getPlayerInfo()->getName()));
    graveMenu->setTitle(tr("&Graveyard"));
    rfgMenu->setTitle(tr("&Exile"));

    if (player->getPlayerInfo()->getLocalOrJudge()) {
        moveHandMenu->setTitle(tr("&Move hand to..."));
        aMoveHandToTopLibrary->setText(tr("&Top of library"));
        aMoveHandToBottomLibrary->setText(tr("&Bottom of library"));
        aMoveHandToGrave->setText(tr("&Graveyard"));
        aMoveHandToRfg->setText(tr("&Exile"));

        moveGraveMenu->setTitle(tr("&Move graveyard to..."));
        aMoveGraveToTopLibrary->setText(tr("&Top of library"));
        aMoveGraveToBottomLibrary->setText(tr("&Bottom of library"));
        aMoveGraveToHand->setText(tr("&Hand"));
        aMoveGraveToRfg->setText(tr("&Exile"));

        moveRfgMenu->setTitle(tr("&Move exile to..."));
        aMoveRfgToTopLibrary->setText(tr("&Top of library"));
        aMoveRfgToBottomLibrary->setText(tr("&Bottom of library"));
        aMoveRfgToHand->setText(tr("&Hand"));
        aMoveRfgToGrave->setText(tr("&Graveyard"));

        aViewLibrary->setText(tr("&View library"));
        aViewHand->setText(tr("&View hand"));
        aSortHand->setText(tr("&Sort hand"));
        aViewTopCards->setText(tr("View &top cards of library..."));
        aViewBottomCards->setText(tr("View bottom cards of library..."));
        mRevealLibrary->setTitle(tr("Reveal &library to..."));
        mLendLibrary->setTitle(tr("Lend library to..."));
        mRevealTopCard->setTitle(tr("Reveal &top cards to..."));
        topLibraryMenu->setTitle(tr("&Top of library..."));
        bottomLibraryMenu->setTitle(tr("&Bottom of library..."));
        aAlwaysRevealTopCard->setText(tr("&Always reveal top card"));
        aAlwaysLookAtTopCard->setText(tr("&Always look at top card"));
        aOpenDeckInDeckEditor->setText(tr("&Open deck in deck editor"));
        aViewSideboard->setText(tr("&View sideboard"));
        aDrawCard->setText(tr("&Draw card"));
        aDrawCards->setText(tr("D&raw cards..."));
        aUndoDraw->setText(tr("&Undo last draw"));
        aMulligan->setText(tr("Take &mulligan"));

        aShuffle->setText(tr("Shuffle"));

        aMoveTopToPlay->setText(tr("&Play top card"));
        aMoveTopToPlayFaceDown->setText(tr("Play top card &face down"));
        aMoveTopCardToBottom->setText(tr("Put top card on &bottom"));
        aMoveTopCardToGraveyard->setText(tr("Move top card to grave&yard"));
        aMoveTopCardToExile->setText(tr("Move top card to e&xile"));
        aMoveTopCardsToGraveyard->setText(tr("Move top cards to &graveyard..."));
        aMoveTopCardsToExile->setText(tr("Move top cards to &exile..."));
        aMoveTopCardsUntil->setText(tr("Put top cards on stack &until..."));
        aShuffleTopCards->setText(tr("Shuffle top cards..."));

        aDrawBottomCard->setText(tr("&Draw bottom card"));
        aDrawBottomCards->setText(tr("D&raw bottom cards..."));
        aMoveBottomToPlay->setText(tr("&Play bottom card"));
        aMoveBottomToPlayFaceDown->setText(tr("Play bottom card &face down"));
        aMoveBottomCardToGraveyard->setText(tr("Move bottom card to grave&yard"));
        aMoveBottomCardToExile->setText(tr("Move bottom card to e&xile"));
        aMoveBottomCardsToGraveyard->setText(tr("Move bottom cards to &graveyard..."));
        aMoveBottomCardsToExile->setText(tr("Move bottom cards to &exile..."));
        aMoveBottomCardToTop->setText(tr("Put bottom card on &top"));
        aShuffleBottomCards->setText(tr("Shuffle bottom cards..."));

        handMenu->setTitle(tr("&Hand"));
        mRevealHand->setTitle(tr("&Reveal hand to..."));
        mRevealRandomHandCard->setTitle(tr("Reveal r&andom card to..."));
        mRevealRandomGraveyardCard->setTitle(tr("Reveal random card to..."));
        sbMenu->setTitle(tr("&Sideboard"));
        libraryMenu->setTitle(tr("&Library"));
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

        mCardCounters->setTitle(tr("Ca&rd counters"));

        for (auto &allPlayersAction : allPlayersActions) {
            allPlayersAction->setText(tr("&All players"));
        }
    }

    if (player->getPlayerInfo()->getLocal()) {
        sayMenu->setTitle(tr("S&ay"));
    }

    aSelectAll->setText(tr("&Select All"));
    aSelectRow->setText(tr("S&elect Row"));
    aSelectColumn->setText(tr("S&elect Column"));

    aPlay->setText(tr("&Play"));
    aHide->setText(tr("&Hide"));
    aPlayFacedown->setText(tr("Play &Face Down"));
    //: Turn sideways or back again
    aTap->setText(tr("&Tap / Untap"));
    aDoesntUntap->setText(tr("Toggle &normal untapping"));
    //: Turn face up/face down
    aFlip->setText(tr("T&urn Over")); // Only the user facing names in client got renamed to "turn over"
                                      // All code and proto bits are still unchanged (flip) for compatibility reasons
                                      // A protocol rewrite with v3 could incorporate that, see #3100
    aPeek->setText(tr("&Peek at card face"));
    aClone->setText(tr("&Clone"));
    aAttach->setText(tr("Attac&h to card..."));
    aUnattach->setText(tr("Unattac&h"));
    aDrawArrow->setText(tr("&Draw arrow..."));
    aIncP->setText(tr("&Increase power"));
    aDecP->setText(tr("&Decrease power"));
    aIncT->setText(tr("I&ncrease toughness"));
    aDecT->setText(tr("D&ecrease toughness"));
    aIncPT->setText(tr("In&crease power and toughness"));
    aDecPT->setText(tr("Dec&rease power and toughness"));
    aFlowP->setText(tr("Increase power and decrease toughness"));
    aFlowT->setText(tr("Decrease power and increase toughness"));
    aSetPT->setText(tr("Set &power and toughness..."));
    aResetPT->setText(tr("Reset p&ower and toughness"));
    aSetAnnotation->setText(tr("&Set annotation..."));

    auto &cardCounterSettings = SettingsCache::instance().cardCounters();

    for (int i = 0; i < aAddCounter.size(); ++i) {
        aAddCounter[i]->setText(tr("&Add counter (%1)").arg(cardCounterSettings.displayName(i)));
    }
    for (int i = 0; i < aRemoveCounter.size(); ++i) {
        aRemoveCounter[i]->setText(tr("&Remove counter (%1)").arg(cardCounterSettings.displayName(i)));
    }
    for (int i = 0; i < aSetCounter.size(); ++i) {
        aSetCounter[i]->setText(tr("&Set counters (%1)...").arg(cardCounterSettings.displayName(i)));
    }

    aMoveToTopLibrary->setText(tr("&Top of library in random order"));
    aMoveToXfromTopOfLibrary->setText(tr("X cards from the top of library..."));
    aMoveToBottomLibrary->setText(tr("&Bottom of library in random order"));
    aMoveToHand->setText(tr("&Hand"));
    aMoveToGraveyard->setText(tr("&Graveyard"));
    aMoveToExile->setText(tr("&Exile"));
}

void PlayerMenu::setShortcutsActive()
{
    shortcutsActive = true;
    ShortcutsSettings &shortcuts = SettingsCache::instance().shortcuts();

    aHide->setShortcuts(shortcuts.getShortcut("Player/aHide"));
    aPlay->setShortcuts(shortcuts.getShortcut("Player/aPlay"));
    aTap->setShortcuts(shortcuts.getShortcut("Player/aTap"));
    aDoesntUntap->setShortcuts(shortcuts.getShortcut("Player/aDoesntUntap"));
    aFlip->setShortcuts(shortcuts.getShortcut("Player/aFlip"));
    aPeek->setShortcuts(shortcuts.getShortcut("Player/aPeek"));
    aClone->setShortcuts(shortcuts.getShortcut("Player/aClone"));
    aAttach->setShortcuts(shortcuts.getShortcut("Player/aAttach"));
    aUnattach->setShortcuts(shortcuts.getShortcut("Player/aUnattach"));
    aDrawArrow->setShortcuts(shortcuts.getShortcut("Player/aDrawArrow"));
    aIncP->setShortcuts(shortcuts.getShortcut("Player/aIncP"));
    aDecP->setShortcuts(shortcuts.getShortcut("Player/aDecP"));
    aIncT->setShortcuts(shortcuts.getShortcut("Player/aIncT"));
    aDecT->setShortcuts(shortcuts.getShortcut("Player/aDecT"));
    aIncPT->setShortcuts(shortcuts.getShortcut("Player/aIncPT"));
    aDecPT->setShortcuts(shortcuts.getShortcut("Player/aDecPT"));
    aFlowP->setShortcuts(shortcuts.getShortcut("Player/aFlowP"));
    aFlowT->setShortcuts(shortcuts.getShortcut("Player/aFlowT"));
    aSetPT->setShortcuts(shortcuts.getShortcut("Player/aSetPT"));
    aResetPT->setShortcuts(shortcuts.getShortcut("Player/aResetPT"));
    aSetAnnotation->setShortcuts(shortcuts.getShortcut("Player/aSetAnnotation"));
    aMoveToTopLibrary->setShortcuts(shortcuts.getShortcut("Player/aMoveToTopLibrary"));
    aMoveToBottomLibrary->setShortcuts(shortcuts.getShortcut("Player/aMoveToBottomLibrary"));
    aMoveToHand->setShortcuts(shortcuts.getShortcut("Player/aMoveToHand"));
    aMoveToGraveyard->setShortcuts(shortcuts.getShortcut("Player/aMoveToGraveyard"));
    aMoveToExile->setShortcuts(shortcuts.getShortcut("Player/aMoveToExile"));
    aSortHand->setShortcuts(shortcuts.getShortcut("Player/aSortHand"));

    aSelectAll->setShortcuts(shortcuts.getShortcut("Player/aSelectAll"));
    aSelectRow->setShortcuts(shortcuts.getShortcut("Player/aSelectRow"));
    aSelectColumn->setShortcuts(shortcuts.getShortcut("Player/aSelectColumn"));

    static const QStringList colorWords = {"Red", "Yellow", "Green", "Cyan", "Purple", "Magenta"};
    for (int i = 0; i < aAddCounter.size(); i++) {
        aAddCounter[i]->setShortcuts(shortcuts.getShortcut("Player/aCC" + colorWords[i]));
        aRemoveCounter[i]->setShortcuts(shortcuts.getShortcut("Player/aRC" + colorWords[i]));
        aSetCounter[i]->setShortcuts(shortcuts.getShortcut("Player/aSC" + colorWords[i]));
    }

    QMapIterator<int, AbstractCounter *> counterIterator(player->getCounters());
    while (counterIterator.hasNext()) {
        counterIterator.next().value()->setShortcutsActive();
    }

    aIncrementAllCardCounters->setShortcuts(shortcuts.getShortcut("Player/aIncrementAllCardCounters"));
    aViewSideboard->setShortcuts(shortcuts.getShortcut("Player/aViewSideboard"));
    aViewLibrary->setShortcuts(shortcuts.getShortcut("Player/aViewLibrary"));
    aViewHand->setShortcuts(shortcuts.getShortcut("Player/aViewHand"));
    aViewTopCards->setShortcuts(shortcuts.getShortcut("Player/aViewTopCards"));
    aViewBottomCards->setShortcuts(shortcuts.getShortcut("Player/aViewBottomCards"));
    aViewGraveyard->setShortcuts(shortcuts.getShortcut("Player/aViewGraveyard"));
    aDrawCard->setShortcuts(shortcuts.getShortcut("Player/aDrawCard"));
    aDrawCards->setShortcuts(shortcuts.getShortcut("Player/aDrawCards"));
    aUndoDraw->setShortcuts(shortcuts.getShortcut("Player/aUndoDraw"));
    aMulligan->setShortcuts(shortcuts.getShortcut("Player/aMulligan"));
    aShuffle->setShortcuts(shortcuts.getShortcut("Player/aShuffle"));
    aShuffleTopCards->setShortcuts(shortcuts.getShortcut("Player/aShuffleTopCards"));
    aShuffleBottomCards->setShortcuts(shortcuts.getShortcut("Player/aShuffleBottomCards"));
    aUntapAll->setShortcuts(shortcuts.getShortcut("Player/aUntapAll"));
    aRollDie->setShortcuts(shortcuts.getShortcut("Player/aRollDie"));
    aCreateToken->setShortcuts(shortcuts.getShortcut("Player/aCreateToken"));
    aCreateAnotherToken->setShortcuts(shortcuts.getShortcut("Player/aCreateAnotherToken"));
    aAlwaysRevealTopCard->setShortcuts(shortcuts.getShortcut("Player/aAlwaysRevealTopCard"));
    aAlwaysLookAtTopCard->setShortcuts(shortcuts.getShortcut("Player/aAlwaysLookAtTopCard"));
    aMoveTopToPlay->setShortcuts(shortcuts.getShortcut("Player/aMoveTopToPlay"));
    aMoveTopToPlayFaceDown->setShortcuts(shortcuts.getShortcut("Player/aMoveTopToPlayFaceDown"));
    aMoveTopCardToGraveyard->setShortcuts(shortcuts.getShortcut("Player/aMoveTopCardToGraveyard"));
    aMoveTopCardsToGraveyard->setShortcuts(shortcuts.getShortcut("Player/aMoveTopCardsToGraveyard"));
    aMoveTopCardToExile->setShortcuts(shortcuts.getShortcut("Player/aMoveTopCardToExile"));
    aMoveTopCardsToExile->setShortcuts(shortcuts.getShortcut("Player/aMoveTopCardsToExile"));
    aMoveTopCardsUntil->setShortcuts(shortcuts.getShortcut("Player/aMoveTopCardsUntil"));
    aMoveTopCardToBottom->setShortcuts(shortcuts.getShortcut("Player/aMoveTopCardToBottom"));
    aDrawBottomCard->setShortcuts(shortcuts.getShortcut("Player/aDrawBottomCard"));
    aDrawBottomCards->setShortcuts(shortcuts.getShortcut("Player/aDrawBottomCards"));
    aMoveBottomToPlay->setShortcuts(shortcuts.getShortcut("Player/aMoveBottomToPlay"));
    aMoveBottomToPlayFaceDown->setShortcuts(shortcuts.getShortcut("Player/aMoveBottomToPlayFaceDown"));
    aMoveBottomCardToGraveyard->setShortcuts(shortcuts.getShortcut("Player/aMoveBottomCardToGrave"));
    aMoveBottomCardsToGraveyard->setShortcuts(shortcuts.getShortcut("Player/aMoveBottomCardsToGrave"));
    aMoveBottomCardToExile->setShortcuts(shortcuts.getShortcut("Player/aMoveBottomCardToExile"));
    aMoveBottomCardsToExile->setShortcuts(shortcuts.getShortcut("Player/aMoveBottomCardsToExile"));
    aMoveBottomCardToTop->setShortcuts(shortcuts.getShortcut("Player/aMoveBottomCardToTop"));
    aPlayFacedown->setShortcuts(shortcuts.getShortcut("Player/aPlayFacedown"));
    aPlay->setShortcuts(shortcuts.getShortcut("Player/aPlay"));

    // Don't enable always-active shortcuts in local games, since it causes keyboard shortcuts to work inconsistently
    // when there are more than 1 player.
    if (!player->getGame()->getGameState()->getIsLocalGame()) {
        // unattach action is only active in card menu if the active card is attached.
        // make unattach shortcut always active so that it consistently works when multiple cards are selected.
        player->getGame()->getTab()->addAction(aUnattach);
    }
}

void PlayerMenu::setShortcutsInactive()
{
    shortcutsActive = false;

    aViewSideboard->setShortcut(QKeySequence());
    aViewLibrary->setShortcut(QKeySequence());
    aViewHand->setShortcut(QKeySequence());
    aViewTopCards->setShortcut(QKeySequence());
    aViewBottomCards->setShortcut(QKeySequence());
    aViewGraveyard->setShortcut(QKeySequence());
    aDrawCard->setShortcut(QKeySequence());
    aDrawCards->setShortcut(QKeySequence());
    aUndoDraw->setShortcut(QKeySequence());
    aMulligan->setShortcut(QKeySequence());
    aShuffle->setShortcut(QKeySequence());
    aShuffleTopCards->setShortcut(QKeySequence());
    aShuffleBottomCards->setShortcut(QKeySequence());
    aUntapAll->setShortcut(QKeySequence());
    aRollDie->setShortcut(QKeySequence());
    aCreateToken->setShortcut(QKeySequence());
    aCreateAnotherToken->setShortcut(QKeySequence());
    aAlwaysRevealTopCard->setShortcut(QKeySequence());
    aAlwaysLookAtTopCard->setShortcut(QKeySequence());
    aMoveTopToPlay->setShortcut(QKeySequence());
    aMoveTopToPlayFaceDown->setShortcut(QKeySequence());
    aMoveTopCardToGraveyard->setShortcut(QKeySequence());
    aMoveTopCardsToGraveyard->setShortcut(QKeySequence());
    aMoveTopCardToExile->setShortcut(QKeySequence());
    aMoveTopCardsToExile->setShortcut(QKeySequence());
    aMoveTopCardsUntil->setShortcut(QKeySequence());
    aDrawBottomCard->setShortcut(QKeySequence());
    aDrawBottomCards->setShortcut(QKeySequence());
    aMoveBottomToPlay->setShortcut(QKeySequence());
    aMoveBottomToPlayFaceDown->setShortcut(QKeySequence());
    aMoveBottomCardToGraveyard->setShortcut(QKeySequence());
    aMoveBottomCardsToGraveyard->setShortcut(QKeySequence());
    aMoveBottomCardToExile->setShortcut(QKeySequence());
    aMoveBottomCardsToExile->setShortcut(QKeySequence());
    aIncrementAllCardCounters->setShortcut(QKeySequence());
    aSortHand->setShortcut(QKeySequence());

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
