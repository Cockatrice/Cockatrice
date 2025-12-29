#include "utility_menu.h"

#include "../../../interface/deck_loader/deck_loader.h"
#include "../player.h"
#include "../player_actions.h"
#include "player_menu.h"

#include <libcockatrice/deck_list/tree/deck_list_card_node.h>
#include <libcockatrice/deck_list/tree/inner_deck_list_node.h>

UtilityMenu::UtilityMenu(Player *_player, QMenu *playerMenu) : QMenu(playerMenu), player(_player)
{
    PlayerActions *playerActions = player->getPlayerActions();

    if (player->getPlayerInfo()->getLocalOrJudge()) {
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
        connect(player, &Player::deckChanged, this, &UtilityMenu::populatePredefinedTokensMenu);

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
    } else {
        aCreateToken = nullptr;
        aCreateAnotherToken = nullptr;
        createPredefinedTokenMenu = nullptr;
        aIncrementAllCardCounters = nullptr;
        aUntapAll = nullptr;
        aRollDie = nullptr;
    }

    retranslateUi();
}

void UtilityMenu::populatePredefinedTokensMenu()
{
    clear();
    setEnabled(false);
    predefinedTokens.clear();
    const DeckList &deckList = player->getDeck();

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
            connect(a, &QAction::triggered, player->getPlayerActions(), &PlayerActions::actCreatePredefinedToken);
        }
    }
}

void UtilityMenu::retranslateUi()
{
    if (player->getPlayerInfo()->getLocalOrJudge()) {
        aIncrementAllCardCounters->setText(tr("Increment all card counters"));
        aUntapAll->setText(tr("&Untap all permanents"));
        aRollDie->setText(tr("R&oll die..."));
        aCreateToken->setText(tr("&Create token..."));
        aCreateAnotherToken->setText(tr("C&reate another token"));
        createPredefinedTokenMenu->setTitle(tr("Cr&eate predefined token"));
    }
}

void UtilityMenu::setShortcutsActive()
{
    ShortcutsSettings &shortcuts = SettingsCache::instance().shortcuts();

    if (player->getPlayerInfo()->getLocalOrJudge()) {
        aIncrementAllCardCounters->setShortcuts(shortcuts.getShortcut("Player/aIncrementAllCardCounters"));
        aUntapAll->setShortcuts(shortcuts.getShortcut("Player/aUntapAll"));
        aRollDie->setShortcuts(shortcuts.getShortcut("Player/aRollDie"));
        aCreateToken->setShortcuts(shortcuts.getShortcut("Player/aCreateToken"));
        aCreateAnotherToken->setShortcuts(shortcuts.getShortcut("Player/aCreateAnotherToken"));
    }
}

void UtilityMenu::setShortcutsInactive()
{
    if (player->getPlayerInfo()->getLocalOrJudge()) {
        aUntapAll->setShortcut(QKeySequence());
        aRollDie->setShortcut(QKeySequence());
        aCreateToken->setShortcut(QKeySequence());
        aCreateAnotherToken->setShortcut(QKeySequence());
        aIncrementAllCardCounters->setShortcut(QKeySequence());
    }
}