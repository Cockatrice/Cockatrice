#include "card_menu.h"

#include "../../../interface/widgets/tabs/tab_game.h"
#include "../../board/card_item.h"
#include "../../zones/logic/view_zone_logic.h"
#include "../card_menu_action_type.h"
#include "../player.h"
#include "../player_actions.h"
#include "move_menu.h"
#include "pt_menu.h"

#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/card/relation/card_relation.h>
#include <libcockatrice/settings/card_counter_settings.h>

CardMenu::CardMenu(Player *_player, const CardItem *_card, bool _shortcutsActive)
    : player(_player), card(_card), shortcutsActive(_shortcutsActive)
{
    auto playerActions = player->getPlayerActions();

    const QList<Player *> &players = player->getGame()->getPlayerManager()->getPlayers().values();

    for (auto playerToAdd : players) {
        if (playerToAdd == player) {
            continue;
        }
        playersInfo.append(qMakePair(playerToAdd->getPlayerInfo()->getName(), playerToAdd->getPlayerInfo()->getId()));
    }

    connect(player->getGame()->getPlayerManager(), &PlayerManager::playerRemoved, this, &CardMenu::removePlayer);

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

    mCardCounters = new QMenu;

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

    setShortcutsActive();

    retranslateUi();

    if (card == nullptr) {
        return;
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

    if (revealedCard) {
        addAction(aHide);
        addAction(aClone);
        addSeparator();
        addAction(aSelectAll);
        addAction(aSelectColumn);
        addRelatedCardView();
    } else if (writeableCard) {

        if (card->getZone()) {
            if (card->getZone()->getName() == "table") {
                createTableMenu();
            } else if (card->getZone()->getName() == "stack") {
                createStackMenu();
            } else if (card->getZone()->getName() == "rfg" || card->getZone()->getName() == "grave") {
                createGraveyardOrExileMenu();
            } else {
                createHandOrCustomZoneMenu();
            }
        } else {
            addMenu(new MoveMenu(player));
        }
    } else {
        if (card->getZone() && card->getZone()->getName() != "hand") {
            addAction(aDrawArrow);
            addSeparator();
            addRelatedCardView();
            addRelatedCardActions();
            addSeparator();
            addAction(aClone);
            addSeparator();
            addAction(aSelectAll);
        }
    }
}

void CardMenu::removePlayer(Player *playerToRemove)
{
    for (auto it = playersInfo.begin(); it != playersInfo.end();) {
        if (it->second == playerToRemove->getPlayerInfo()->getId()) {
            it = playersInfo.erase(it);
        } else {
            ++it;
        }
    }
}

void CardMenu::createTableMenu()
{
    // Card is on the battlefield
    bool canModifyCard = player->getPlayerInfo()->judge || card->getOwner() == player;

    if (!canModifyCard) {
        addRelatedCardView();
        addRelatedCardActions();

        addSeparator();
        addAction(aDrawArrow);
        addSeparator();
        addAction(aClone);
        addSeparator();
        addAction(aSelectAll);
        addAction(aSelectRow);
        return;
    }

    addAction(aTap);
    addAction(aDoesntUntap);
    addAction(aFlip);
    if (card->getFaceDown()) {
        addAction(aPeek);
    }

    addRelatedCardView();
    addRelatedCardActions();

    addSeparator();
    addAction(aAttach);
    if (card->getAttachedTo()) {
        addAction(aUnattach);
    }
    addAction(aDrawArrow);
    addSeparator();
    addMenu(new PtMenu(player));
    addAction(aSetAnnotation);
    addSeparator();
    addAction(aClone);
    addMenu(new MoveMenu(player));
    addSeparator();
    addAction(aSelectAll);
    addAction(aSelectRow);

    addSeparator();
    mCardCounters->clear();
    for (int i = 0; i < aAddCounter.size(); ++i) {
        mCardCounters->addSeparator();
        mCardCounters->addAction(aAddCounter[i]);
        if (card->getCounters().contains(i)) {
            mCardCounters->addAction(aRemoveCounter[i]);
        }
        mCardCounters->addAction(aSetCounter[i]);
    }
    addSeparator();
    addMenu(mCardCounters);
}

void CardMenu::createStackMenu()
{
    bool canModifyCard = player->getPlayerInfo()->judge || card->getOwner() == player;

    // Card is on the stack
    if (canModifyCard) {
        addAction(aAttach);
        addAction(aDrawArrow);
        addSeparator();
        addAction(aClone);
        addMenu(new MoveMenu(player));
        addSeparator();
        addAction(aSelectAll);
    } else {
        addAction(aDrawArrow);
        addSeparator();
        addAction(aClone);
        addSeparator();
        addAction(aSelectAll);
    }

    addRelatedCardView();
    addRelatedCardActions();
}

void CardMenu::createGraveyardOrExileMenu()
{
    bool canModifyCard = player->getPlayerInfo()->judge || card->getOwner() == player;

    // Card is in the graveyard or exile
    if (canModifyCard) {
        addAction(aPlay);
        addAction(aPlayFacedown);

        addSeparator();
        addAction(aClone);
        addMenu(new MoveMenu(player));
        addSeparator();
        addAction(aSelectAll);
        addAction(aSelectColumn);

        addSeparator();
        addAction(aAttach);
        addAction(aDrawArrow);
    } else {
        addAction(aClone);
        addSeparator();
        addAction(aSelectAll);
        addAction(aSelectColumn);
        addSeparator();
        addAction(aDrawArrow);
    }

    addRelatedCardView();
    addRelatedCardActions();
}

void CardMenu::createHandOrCustomZoneMenu()
{
    // Card is in hand or a custom zone specified by server
    addAction(aPlay);
    addAction(aPlayFacedown);

    QMenu *revealMenu = addMenu(tr("Re&veal to..."));

    initContextualPlayersMenu(revealMenu);

    connect(revealMenu, &QMenu::triggered, player->getPlayerActions(), &PlayerActions::actReveal);

    addSeparator();
    addAction(aClone);
    addMenu(new MoveMenu(player));

    // actions that are really wonky when done from deck or sideboard
    if (card->getZone()->getName() == "hand") {
        addSeparator();
        addAction(aAttach);
        addAction(aDrawArrow);
    }

    addSeparator();
    addAction(aSelectAll);
    if (qobject_cast<ZoneViewZoneLogic *>(card->getZone())) {
        addAction(aSelectColumn);
    }

    addRelatedCardView();
    if (card->getZone()->getName() == "hand") {
        addRelatedCardActions();
    }
}

void CardMenu::initContextualPlayersMenu(QMenu *menu)
{
    menu->addAction(tr("&All players"))->setData(-1);
    menu->addSeparator();

    for (const auto &playerInfo : playersInfo) {
        menu->addAction(playerInfo.first)->setData(playerInfo.second);
    }
}

void CardMenu::addRelatedCardView()
{
    if (!card) {
        return;
    }
    auto exactCard = card->getCard();
    if (!exactCard) {
        return;
    }

    bool atLeastOneGoodRelationFound = false;
    QList<CardRelation *> relatedCards = exactCard.getInfo().getAllRelatedCards();
    for (const CardRelation *cardRelation : relatedCards) {
        CardInfoPtr relatedCard = CardDatabaseManager::query()->getCardInfo(cardRelation->getName());
        if (relatedCard != nullptr) {
            atLeastOneGoodRelationFound = true;
            break;
        }
    }

    if (!atLeastOneGoodRelationFound) {
        return;
    }

    addSeparator();
    auto viewRelatedCards = new QMenu(tr("View related cards"));
    addMenu(viewRelatedCards);
    for (const CardRelation *relatedCard : relatedCards) {
        QString relatedCardName = relatedCard->getName();
        CardRef cardRef = {relatedCardName, exactCard.getPrinting().getUuid()};
        QAction *viewCard = viewRelatedCards->addAction(relatedCardName);
        Q_UNUSED(viewCard);

        connect(viewCard, &QAction::triggered, player->getGame(),
                [this, cardRef] { player->getGame()->getTab()->viewCardInfo(cardRef); });
    }
}

void CardMenu::addRelatedCardActions()
{
    if (!card) {
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

    addSeparator();
    int index = 0;
    QAction *createRelatedCards = nullptr;
    for (const CardRelation *cardRelation : relatedCards) {
        ExactCard relatedCard =
            CardDatabaseManager::query()->getCardFromSameSet(cardRelation->getName(), card->getCard().getPrinting());
        if (!relatedCard) {
            relatedCard = CardDatabaseManager::query()->getCard({cardRelation->getName()});
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
        addAction(createRelated);
    }

    if (createRelatedCards) {
        if (shortcutsActive) {
            createRelatedCards->setShortcuts(
                SettingsCache::instance().shortcuts().getShortcut("Player/aCreateRelatedTokens"));
        }
        connect(createRelatedCards, &QAction::triggered, player->getPlayerActions(),
                &PlayerActions::actCreateAllRelatedCards);
        addAction(createRelatedCards);
    }
}

void CardMenu::retranslateUi()
{
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

    mCardCounters->setTitle(tr("Ca&rd counters"));

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
}

void CardMenu::setShortcutsActive()
{
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
    aSetAnnotation->setShortcuts(shortcuts.getShortcut("Player/aSetAnnotation"));

    aSelectAll->setShortcuts(shortcuts.getShortcut("Player/aSelectAll"));
    aSelectRow->setShortcuts(shortcuts.getShortcut("Player/aSelectRow"));
    aSelectColumn->setShortcuts(shortcuts.getShortcut("Player/aSelectColumn"));

    aPlayFacedown->setShortcuts(shortcuts.getShortcut("Player/aPlayFacedown"));
    aPlay->setShortcuts(shortcuts.getShortcut("Player/aPlay"));

    static const QStringList colorWords = {"Red", "Yellow", "Green", "Cyan", "Purple", "Magenta"};
    for (int i = 0; i < aAddCounter.size(); i++) {
        aAddCounter[i]->setShortcuts(shortcuts.getShortcut("Player/aCC" + colorWords[i]));
        aRemoveCounter[i]->setShortcuts(shortcuts.getShortcut("Player/aRC" + colorWords[i]));
        aSetCounter[i]->setShortcuts(shortcuts.getShortcut("Player/aSC" + colorWords[i]));
    }
}