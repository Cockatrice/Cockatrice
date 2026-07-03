#include "utility_menu.h"

#include "../../../interface/deck_loader/deck_loader.h"
#include "../../game/player/player_actions.h"
#include "../../game/player/player_logic.h"
#include "../player_graphics_item.h"
#include "player_menu.h"

#include <libcockatrice/deck_list/tree/deck_list_card_node.h>
#include <libcockatrice/deck_list/tree/inner_deck_list_node.h>

UtilityMenu::UtilityMenu(PlayerGraphicsItem *_player, QMenu *playerMenu) : QMenu(playerMenu), player(_player)
{
    PlayerActions *playerActions = player->getLogic()->getPlayerActions();
    connect(playerActions, &PlayerActions::requestEnableAndSetCreateAnotherTokenAction, this,
            &UtilityMenu::setAndEnableCreateAnotherTokenAction);
    connect(playerActions, &PlayerActions::requestSetLastToken, this, &UtilityMenu::setLastToken);

    if (player->getLogic()->getPlayerInfo()->getLocalOrJudge()) {
        aUntapAll = new QAction(this);
        connect(aUntapAll, &QAction::triggered, playerActions, &PlayerActions::actUntapAll);

        aRollDie = new QAction(this);
        connect(aRollDie, &QAction::triggered, playerActions, &PlayerActions::actRequestRollDieDialog);

        aFlipCoin = new QAction(this);
        connect(aFlipCoin, &QAction::triggered, playerActions, &PlayerActions::actFlipCoin);

        aCreateToken = new QAction(this);
        connect(aCreateToken, &QAction::triggered, playerActions, [this]() {
            player->getLogic()->getPlayerActions()->actRequestCreateTokenDialog(getPredefinedTokens());
        });

        aCreateAnotherToken = new QAction(this);
        connect(aCreateAnotherToken, &QAction::triggered, playerActions, &PlayerActions::actCreateAnotherToken);
        aCreateAnotherToken->setEnabled(false);

        aIncrementAllCardCounters = new QAction(this);
        connect(aIncrementAllCardCounters, &QAction::triggered, playerActions, [this]() {
            player->getLogic()->getPlayerActions()->actIncrementAllCardCounters(
                player->getGameScene()->selectedCards());
        });

        createPredefinedTokenMenu = new QMenu(QString());
        createPredefinedTokenMenu->setEnabled(false);
        connect(player->getLogic(), &PlayerLogic::deckChanged, this, &UtilityMenu::populatePredefinedTokensMenu);

        playerMenu->addAction(aIncrementAllCardCounters);
        playerMenu->addSeparator();
        playerMenu->addAction(aUntapAll);
        playerMenu->addSeparator();
        playerMenu->addAction(aRollDie);
        playerMenu->addAction(aFlipCoin);
        playerMenu->addSeparator();
        playerMenu->addAction(aCreateToken);
        playerMenu->addAction(aCreateAnotherToken);
        playerMenu->addMenu(createPredefinedTokenMenu);
        playerMenu->addSeparator();
    } else {
        aCreateToken = nullptr;
        aCreateAnotherToken = nullptr;
        createPredefinedTokenMenu = nullptr;
        aIncrementAllCardCounters = nullptr;
        aUntapAll = nullptr;
        aRollDie = nullptr;
        aFlipCoin = nullptr;
    }

    retranslateUi();
}

void UtilityMenu::populatePredefinedTokensMenu()
{
    clear();
    setEnabled(false);
    predefinedTokens.clear();
    const DeckList &deckList = player->getLogic()->getDeck();

    if (deckList.isEmpty()) {
        return;
    }

    auto tokenCardNodes = deckList.getCardNodes({DECK_ZONE_TOKENS});

    if (!tokenCardNodes.isEmpty()) {
        setEnabled(true);

        for (int i = 0; i < tokenCardNodes.size(); ++i) {
            const QString tokenName = tokenCardNodes[i]->getName();
            predefinedTokens.append(tokenName);
            QAction *a = addAction(tokenName);
            if (i < 10) {
                a->setShortcut(QKeySequence("Alt+" + QString::number((i + 1) % 10)));
            }
            connect(a, &QAction::triggered, player->getLogic()->getPlayerActions(),
                    &PlayerActions::actCreatePredefinedToken);
        }
    }
}

void UtilityMenu::setLastToken(CardInfoPtr lastToken)
{
    if (!createAnotherTokenActionExists()) {
        return;
    }

    player->getLogic()->getPlayerActions()->setLastTokenInfo(lastToken);
}

void UtilityMenu::retranslateUi()
{
    if (player->getLogic()->getPlayerInfo()->getLocalOrJudge()) {
        aIncrementAllCardCounters->setText(tr("Increment all card counters"));
        aUntapAll->setText(tr("&Untap all permanents"));
        aRollDie->setText(tr("R&oll die..."));
        aFlipCoin->setText(tr("Flip coin"));
        aCreateToken->setText(tr("&Create token..."));
        aCreateAnotherToken->setText(tr("C&reate another token"));
        createPredefinedTokenMenu->setTitle(tr("Cr&eate predefined token"));
    }
}

void UtilityMenu::setShortcutsActive()
{
    ShortcutsSettings &shortcuts = SettingsCache::instance().shortcuts();

    if (player->getLogic()->getPlayerInfo()->getLocalOrJudge()) {
        aIncrementAllCardCounters->setShortcuts(shortcuts.getShortcut("Player/aIncrementAllCardCounters"));
        aUntapAll->setShortcuts(shortcuts.getShortcut("Player/aUntapAll"));
        aRollDie->setShortcuts(shortcuts.getShortcut("Player/aRollDie"));
        aFlipCoin->setShortcuts(shortcuts.getShortcut("Player/aFlipCoin"));
        aCreateToken->setShortcuts(shortcuts.getShortcut("Player/aCreateToken"));
        aCreateAnotherToken->setShortcuts(shortcuts.getShortcut("Player/aCreateAnotherToken"));
    }
}

void UtilityMenu::setShortcutsInactive()
{
    if (player->getLogic()->getPlayerInfo()->getLocalOrJudge()) {
        aUntapAll->setShortcut(QKeySequence());
        aRollDie->setShortcut(QKeySequence());
        aFlipCoin->setShortcut(QKeySequence());
        aCreateToken->setShortcut(QKeySequence());
        aCreateAnotherToken->setShortcut(QKeySequence());
        aIncrementAllCardCounters->setShortcut(QKeySequence());
    }
}