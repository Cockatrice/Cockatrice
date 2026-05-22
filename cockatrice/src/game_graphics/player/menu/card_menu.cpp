#include "card_menu.h"

#include "../../../client/settings/card_counter_settings.h"
#include "../../../interface/widgets/tabs/tab_game.h"
#include "../../board/abstract_counter.h"
#include "../../board/card_item.h"
#include "../../game/player/player_actions.h"
#include "../../game/player/player_logic.h"
#include "../../game/zones/view_zone_logic.h"
#include "../card_menu_action_type.h"
#include "../player_graphics_item.h"
#include "move_menu.h"
#include "pt_menu.h"

#include <QPainter>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/card/relation/card_relation.h>
#include <libcockatrice/utility/counter_ids.h>
#include <libcockatrice/utility/zone_names.h>

/**
 * @brief Creates a circular icon filled with the specified color.
 */
static QIcon createCircleIcon(const QColor &color)
{
    QPixmap pixmap(32, 32);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(color);
    painter.drawEllipse(pixmap.rect());

    return QIcon(pixmap);
}

template <typename Slot>
static QAction *makeAction(QObject *parent, Slot &&slot, bool checkable = false, bool checked = false)
{
    auto *a = new QAction(parent);
    a->setCheckable(checkable);
    if (checkable) {
        a->setChecked(checked);
    }
    QObject::connect(a, &QAction::triggered, parent, std::forward<Slot>(slot));
    return a;
}

CardMenu::CardMenu(PlayerGraphicsItem *_player, const CardItem *_card, bool _shortcutsActive)
    : player(_player), card(_card), shortcutsActive(_shortcutsActive)
{
    const QList<PlayerLogic *> &players = player->getLogic()->getGame()->getPlayerManager()->getPlayers().values();

    for (auto playerToAdd : players) {
        if (playerToAdd == player->getLogic()) {
            continue;
        }
        playersInfo.append(qMakePair(playerToAdd->getPlayerInfo()->getName(), playerToAdd->getPlayerInfo()->getId()));
    }

    connect(player->getLogic()->getGame()->getPlayerManager(), &PlayerManager::playerRemoved, this,
            &CardMenu::removePlayer);

    auto *actions = player->getLogic()->getPlayerActions();
    auto *gameScene = player->getGameScene();

    // Single selection resolver used by all lambdas — called at trigger time
    auto sel = [gameScene]() { return gameScene->selectedCards(); };

    // Unified dispatcher for card menu actions
    auto invoke = [actions, sel](CardMenuActionType type) {
        return [actions, sel, type]() { actions->cardMenuAction(sel(), type); };
    };

    // Actions using invoke (type dispatch, need selection)
    aTap = makeAction(this, invoke(cmTap));
    aDoesntUntap = makeAction(this, invoke(cmDoesntUntap), /*checkable=*/true, card && card->getDoesntUntap());
    aFlip = makeAction(this, invoke(cmFlip));
    aPeek = makeAction(this, invoke(cmPeek));
    aClone = makeAction(this, invoke(cmClone));

    // Actions using selection directly
    aUnattach = makeAction(this, [actions, sel]() { actions->actUnattach(sel()); });
    aSetAnnotation = makeAction(this, [actions, sel]() { actions->actRequestSetAnnotationDialog(sel()); });
    aPlay = makeAction(this, [actions, sel]() { actions->actPlay(sel()); });
    aPlayFacedown = makeAction(this, [actions, sel]() { actions->actPlayFacedown(sel()); });
    aHide = makeAction(this, [actions, sel]() { actions->actHide(sel()); });
    aReduceLifeByPower = makeAction(this, [actions, sel]() { actions->actReduceLifeByPower(sel()); });

    // Actions that use activeCard, not selection — direct connection
    aAttach = new QAction(this);
    aDrawArrow = new QAction(this);
    aSelectAll = new QAction(this);
    aSelectRow = new QAction(this);
    aSelectColumn = new QAction(this);

    aPlayAndIncreaseTax = new QAction(this);
    connect(aPlayAndIncreaseTax, &QAction::triggered, playerActions, &PlayerActions::actPlayAndIncreaseTax);
    aPlayAndIncreasePartnerTax = new QAction(this);
    connect(aPlayAndIncreasePartnerTax, &QAction::triggered, playerActions,
            &PlayerActions::actPlayAndIncreasePartnerTax);

    connect(aAttach, &QAction::triggered, actions, &PlayerActions::actAttach);
    connect(aDrawArrow, &QAction::triggered, actions, &PlayerActions::actDrawArrow);
    connect(aSelectAll, &QAction::triggered, actions, &PlayerActions::actSelectAll);
    connect(aSelectRow, &QAction::triggered, actions, &PlayerActions::actSelectRow);
    connect(aSelectColumn, &QAction::triggered, actions, &PlayerActions::actSelectColumn);

    aRevealToAll = new QAction(this);

    mCardCounters = new QMenu;

    // Card counters
    for (int i = 0; i < 6; ++i) {
        QColor color = SettingsCache::instance().cardCounters().color(i);
        QIcon circleIcon = createCircleIcon(color);

        auto *addAction = makeAction(this, [actions, sel, i]() { actions->actAddCardCounter(sel(), i); });
        addAction->setIcon(circleIcon);
        aAddCounter.append(addAction);

        auto *removeAction = makeAction(this, [actions, sel, i]() { actions->actRemoveCardCounter(sel(), i); });
        removeAction->setIcon(circleIcon);
        aRemoveCounter.append(removeAction);

        auto *setAction = makeAction(this, [actions, sel, i]() { actions->actRequestSetCardCounterDialog(sel(), i); });
        setAction->setIcon(circleIcon);
        aSetCounter.append(setAction);
    }

    setShortcutsActive();

    retranslateUi();

    if (card == nullptr) {
        return;
    }

    bool revealedCard = false;
    bool writeableCard = player->getLogic()->getPlayerInfo()->getLocalOrJudge();
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
        addSeparator();
        addAction(aClone);
        addSeparator();
        addAction(aSelectAll);
        addAction(aSelectColumn);
        addRelatedCardView();
    } else {
        if (card->getZone()) {
            if (card->getZone()->getName() == ZoneNames::TABLE) {
                createTableMenu(writeableCard);
            } else if (card->getZone()->getName() == ZoneNames::STACK) {
                createStackMenu(writeableCard);
            } else if (card->getZone()->getName() == ZoneNames::EXILE ||
                       card->getZone()->getName() == ZoneNames::GRAVE) {
                createGraveyardOrExileMenu(writeableCard);
            } else if (card->getZone()->getName() == ZoneNames::COMMAND) {
                if (writeableCard) {
                    addAction(aPlay);

                    AbstractCounter *cmdTax = player->getCounterWidget(CounterIds::CommanderTax);
                    if (cmdTax && cmdTax->isActive()) {
                        addAction(aPlayAndIncreaseTax);
                    }

                    AbstractCounter *partnerTax = player->getCounterWidget(CounterIds::PartnerTax);
                    if (partnerTax && partnerTax->isActive()) {
                        addAction(aPlayAndIncreasePartnerTax);
                    }

                    // No reveal submenu - command zone is public
                    addSeparator();
                    addAction(aClone);
                    addMenu(new MoveMenu(player));
                } else {
                    addAction(aDrawArrow);
                    addSeparator();
                    addAction(aClone);
                }
                addSeparator();
                addAction(aSelectAll);
                addRelatedCardView();
                addRelatedCardActions();
            } else {
                createHandOrCustomZoneMenu(writeableCard);
            }
        } else {
            createZonelessMenu(writeableCard);
        }
    }
}

void CardMenu::removePlayer(PlayerLogic *playerToRemove)
{
    for (auto it = playersInfo.begin(); it != playersInfo.end();) {
        if (it->second == playerToRemove->getPlayerInfo()->getId()) {
            it = playersInfo.erase(it);
        } else {
            ++it;
        }
    }
}

void CardMenu::createTableMenu(bool canModifyCard)
{
    // Card is on the battlefield
    if (!canModifyCard) {
        addAction(aDrawArrow);
        addSeparator();
        addAction(aClone);
        addSeparator();
        addAction(aReduceLifeByPower);
        addSeparator();
        addAction(aSelectAll);
        addAction(aSelectRow);
        addRelatedCardView();
        addRelatedCardActions();
        return;
    }

    addAction(aTap);
    addAction(aDoesntUntap);
    addAction(aFlip);
    if (card->getFaceDown()) {
        addAction(aPeek);
    }
    addSeparator();
    addAction(aClone);
    addMenu(new MoveMenu(player));
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
    addAction(aReduceLifeByPower);
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
    addRelatedCardView();
    addRelatedCardActions();
}

void CardMenu::createStackMenu(bool canModifyCard)
{
    // Card is on the stack
    if (!canModifyCard) {
        addAction(aDrawArrow);
        addSeparator();
        addAction(aClone);
        addSeparator();
        addAction(aSelectAll);
        addRelatedCardView();
        addRelatedCardActions();
        return;
    }

    addAction(aPlay);
    addAction(aPlayFacedown);
    addSeparator();
    addAction(aClone);
    addMenu(new MoveMenu(player));
    addSeparator();
    addAction(aAttach);
    addAction(aDrawArrow);
    addSeparator();
    addAction(aSelectAll);
    addRelatedCardView();
    addRelatedCardActions();
}

void CardMenu::createGraveyardOrExileMenu(bool canModifyCard)
{
    // Card is in the graveyard or exile
    if (!canModifyCard) {
        addAction(aDrawArrow);
        addSeparator();
        addAction(aClone);
        addSeparator();
        addAction(aSelectAll);
        addAction(aSelectColumn);
        addRelatedCardView();
        addRelatedCardActions();
        return;
    }

    addAction(aPlay);
    addAction(aPlayFacedown);
    addSeparator();
    addAction(aClone);
    addMenu(new MoveMenu(player));
    addSeparator();
    addAction(aAttach);
    addAction(aDrawArrow);
    addSeparator();
    addAction(aSelectAll);
    addAction(aSelectColumn);
    addRelatedCardView();
    addRelatedCardActions();
}

void CardMenu::createHandOrCustomZoneMenu(bool canModifyCard)
{
    if (!canModifyCard) {
        addAction(aDrawArrow);
        addSeparator();
        addAction(aClone);
        addSeparator();
        addAction(aSelectAll);
        addRelatedCardView();
        addRelatedCardActions();
        return;
    }

    // Card is in hand or a custom zone specified by server
    addAction(aPlay);
    addAction(aPlayFacedown);

    QMenu *revealMenu = addMenu(tr("Re&veal to..."));

    initContextualPlayersMenu(revealMenu, aRevealToAll);

    connect(revealMenu, &QMenu::triggered, this, [this](QAction *action) {
        player->getLogic()->getPlayerActions()->actReveal(player->getGameScene()->selectedCards(), action);
    });

    addSeparator();
    addAction(aClone);
    addMenu(new MoveMenu(player));

    // actions that are really wonky when done from deck or sideboard
    if (card->getZone()->getName() == ZoneNames::HAND) {
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
    if (card->getZone()->getName() == ZoneNames::HAND) {
        addRelatedCardActions();
    }
}

void CardMenu::createZonelessMenu(bool canModifyCard)
{
    if (canModifyCard) {
        addMenu(new MoveMenu(player));
    }
}

/**
 * @brief Populates the menu with an action for each active player.
 *
 * The "all players" action is created separately, so it has to be passed into this function.
 * It will be put at the top of the menu.
 *
 * @param menu The menu to add the player actions to.
 * @param allPlayersAction The action for "all players".
 */
void CardMenu::initContextualPlayersMenu(QMenu *menu, QAction *allPlayersAction)
{
    allPlayersAction->setData(-1);
    menu->addAction(allPlayersAction);
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

        connect(viewCard, &QAction::triggered, this, [this, cardRef] { emit cardInfoRequested(cardRef); });
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
        connect(createRelated, &QAction::triggered, player->getLogic()->getPlayerActions(),
                &PlayerActions::actCreateRelatedCard);
        addAction(createRelated);
    }

    if (createRelatedCards) {
        if (shortcutsActive) {
            createRelatedCards->setShortcuts(
                SettingsCache::instance().shortcuts().getShortcut("Player/aCreateRelatedTokens"));
        }
        connect(createRelatedCards, &QAction::triggered, player->getLogic()->getPlayerActions(),
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
    aPlayAndIncreaseTax->setText(tr("Play and &Increase Commander Tax"));
    aPlayAndIncreasePartnerTax->setText(tr("Play and Increase &Partner Tax"));
    aRevealToAll->setText(tr("&All players"));
    //: Turn sideways or back again
    aTap->setText(tr("&Tap / Untap"));
    aDoesntUntap->setText(tr("Skip &untapping"));
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
    aReduceLifeByPower->setText(tr("Reduce life by power"));

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
    aPlayFacedown->setShortcuts(shortcuts.getShortcut("Player/aPlayFacedown"));
    aRevealToAll->setShortcuts(shortcuts.getShortcut("Player/aRevealToAll"));

    aTap->setShortcuts(shortcuts.getShortcut("Player/aTap"));
    aDoesntUntap->setShortcuts(shortcuts.getShortcut("Player/aDoesntUntap"));
    aFlip->setShortcuts(shortcuts.getShortcut("Player/aFlip"));
    aPeek->setShortcuts(shortcuts.getShortcut("Player/aPeek"));
    aClone->setShortcuts(shortcuts.getShortcut("Player/aClone"));
    aAttach->setShortcuts(shortcuts.getShortcut("Player/aAttach"));
    aUnattach->setShortcuts(shortcuts.getShortcut("Player/aUnattach"));
    aDrawArrow->setShortcuts(shortcuts.getShortcut("Player/aDrawArrow"));
    aSetAnnotation->setShortcuts(shortcuts.getShortcut("Player/aSetAnnotation"));
    aReduceLifeByPower->setShortcuts(shortcuts.getShortcut("Player/aReduceLifeByPower"));

    aSelectAll->setShortcuts(shortcuts.getShortcut("Player/aSelectAll"));
    aSelectRow->setShortcuts(shortcuts.getShortcut("Player/aSelectRow"));
    aSelectColumn->setShortcuts(shortcuts.getShortcut("Player/aSelectColumn"));

    static const QStringList colorWords = {"Red", "Yellow", "Green", "Cyan", "Purple", "Magenta"};
    for (int i = 0; i < aAddCounter.size(); i++) {
        aAddCounter[i]->setShortcuts(shortcuts.getShortcut("Player/aCC" + colorWords[i]));
        aRemoveCounter[i]->setShortcuts(shortcuts.getShortcut("Player/aRC" + colorWords[i]));
        aSetCounter[i]->setShortcuts(shortcuts.getShortcut("Player/aSC" + colorWords[i]));
    }
}