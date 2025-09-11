#include "player_menu.h"

#include "../../../client/tabs/tab_game.h"
#include "../../../common/pb/command_reveal_cards.pb.h"
#include "../../../settings/card_counter_settings.h"
#include "../../board/card_item.h"
#include "../../cards/card_database_manager.h"
#include "../../zones/hand_zone.h"
#include "../../zones/logic/view_zone_logic.h"
#include "../card_menu_action_type.h"
#include "../player_actions.h"
#include "hand_menu.h"
#include "pt_menu.h"

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

    // We have to explicitly not-instantiate a bunch of things if we are not local or a judge or else we have to
    // consider it everywhere instead of just null-checking
    if (!player->getPlayerInfo()->local && !player->getPlayerInfo()->judge) {
        countersMenu = nullptr;
        sbMenu = nullptr;
        mCustomZones = nullptr;
        aCreateAnotherToken = nullptr;
        createPredefinedTokenMenu = nullptr;
        aIncrementAllCardCounters = nullptr;
        mCardCounters = nullptr;

        aViewSideboard = nullptr;
        handMenu = nullptr;
        sbMenu = nullptr;
        libraryMenu = nullptr;

        aUntapAll = nullptr;
        aRollDie = nullptr;
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

    retranslateUi();
}

void PlayerMenu::setMenusForGraphicItems()
{
    player->getGraphicsItem()->getTableZoneGraphicsItem()->setMenu(playerMenu);
    //player->getGraphicsItem()->getGraveyardZoneGraphicsItem()->setMenu(graveMenu, aViewGraveyard);
    //player->getGraphicsItem()->getRfgZoneGraphicsItem()->setMenu(rfgMenu, aViewRfg);
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
                cardMenu->addMenu(new PtMenu(player));
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

/**
 * Creates a card menu from the given card and sets it as the currently active card menu.
 * Will first check if the card should have a card menu, and no-ops if not.
 *
 * @param card The card to create the menu for. Pass nullptr to disable the card menu.
 * @return The new card menu, or nullptr if failed.
 */
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

QMenu *PlayerMenu::createMoveMenu() const
{
    QMenu *moveMenu = new QMenu(tr("Move to"));
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



void PlayerMenu::retranslateUi()
{
    playerMenu->setTitle(tr("Player \"%1\"").arg(player->getPlayerInfo()->getName()));
    graveMenu->setTitle(tr("&Graveyard"));
    rfgMenu->setTitle(tr("&Exile"));

    if (player->getPlayerInfo()->getLocalOrJudge()) {
        aViewSideboard->setText(tr("&View sideboard"));

        handMenu->setTitle(tr("&Hand"));
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

    setShortcutIfItExists(aHide, shortcuts.getShortcut("Player/aHide"));
    setShortcutIfItExists(aPlay, shortcuts.getShortcut("Player/aPlay"));
    setShortcutIfItExists(aTap, shortcuts.getShortcut("Player/aTap"));
    setShortcutIfItExists(aDoesntUntap, shortcuts.getShortcut("Player/aDoesntUntap"));
    setShortcutIfItExists(aFlip, shortcuts.getShortcut("Player/aFlip"));
    setShortcutIfItExists(aPeek, shortcuts.getShortcut("Player/aPeek"));
    setShortcutIfItExists(aClone, shortcuts.getShortcut("Player/aClone"));
    setShortcutIfItExists(aAttach, shortcuts.getShortcut("Player/aAttach"));
    setShortcutIfItExists(aUnattach, shortcuts.getShortcut("Player/aUnattach"));
    setShortcutIfItExists(aDrawArrow, shortcuts.getShortcut("Player/aDrawArrow"));
    setShortcutIfItExists(aSetAnnotation, shortcuts.getShortcut("Player/aSetAnnotation"));
    setShortcutIfItExists(aMoveToTopLibrary, shortcuts.getShortcut("Player/aMoveToTopLibrary"));
    setShortcutIfItExists(aMoveToBottomLibrary, shortcuts.getShortcut("Player/aMoveToBottomLibrary"));
    setShortcutIfItExists(aMoveToHand, shortcuts.getShortcut("Player/aMoveToHand"));
    setShortcutIfItExists(aMoveToGraveyard, shortcuts.getShortcut("Player/aMoveToGraveyard"));
    setShortcutIfItExists(aMoveToExile, shortcuts.getShortcut("Player/aMoveToExile"));

    setShortcutIfItExists(aSelectAll, shortcuts.getShortcut("Player/aSelectAll"));
    setShortcutIfItExists(aSelectRow, shortcuts.getShortcut("Player/aSelectRow"));
    setShortcutIfItExists(aSelectColumn, shortcuts.getShortcut("Player/aSelectColumn"));

    static const QStringList colorWords = {"Red", "Yellow", "Green", "Cyan", "Purple", "Magenta"};
    for (int i = 0; i < aAddCounter.size(); i++) {
        setShortcutIfItExists(aAddCounter[i], shortcuts.getShortcut("Player/aCC" + colorWords[i]));
        setShortcutIfItExists(aRemoveCounter[i], shortcuts.getShortcut("Player/aRC" + colorWords[i]));
        setShortcutIfItExists(aSetCounter[i], shortcuts.getShortcut("Player/aSC" + colorWords[i]));
    }

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
    setShortcutIfItExists(aPlayFacedown, shortcuts.getShortcut("Player/aPlayFacedown"));
    setShortcutIfItExists(aPlay, shortcuts.getShortcut("Player/aPlay"));

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
