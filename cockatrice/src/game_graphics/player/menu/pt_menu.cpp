#include "pt_menu.h"

#include "../../game/player/player_actions.h"
#include "../../game/player/player_logic.h"
#include "../player_graphics_item.h"

PtMenu::PtMenu(PlayerGraphicsItem *player) : QMenu(tr("Power / toughness"))
{
    PlayerActions *playerActions = player->getLogic()->getPlayerActions();

    aIncP = new QAction(this);
    connect(aIncP, &QAction::triggered, playerActions,
            [player, playerActions] { playerActions->actIncP(player->getGameScene()->selectedCards()); });
    aDecP = new QAction(this);
    connect(aDecP, &QAction::triggered, playerActions,
            [player, playerActions] { playerActions->actDecP(player->getGameScene()->selectedCards()); });
    aIncT = new QAction(this);
    connect(aIncT, &QAction::triggered, playerActions,
            [player, playerActions] { playerActions->actIncT(player->getGameScene()->selectedCards()); });
    aDecT = new QAction(this);
    connect(aDecT, &QAction::triggered, playerActions,
            [player, playerActions] { playerActions->actDecT(player->getGameScene()->selectedCards()); });
    aIncPT = new QAction(this);
    connect(aIncPT, &QAction::triggered, playerActions,
            [player, playerActions] { playerActions->actIncPT(player->getGameScene()->selectedCards()); });
    aDecPT = new QAction(this);
    connect(aDecPT, &QAction::triggered, playerActions,
            [player, playerActions] { playerActions->actDecPT(player->getGameScene()->selectedCards()); });
    aFlowP = new QAction(this);
    connect(aFlowP, &QAction::triggered, playerActions,
            [player, playerActions] { playerActions->actFlowP(player->getGameScene()->selectedCards()); });
    aFlowT = new QAction(this);
    connect(aFlowT, &QAction::triggered, playerActions,
            [player, playerActions] { playerActions->actFlowT(player->getGameScene()->selectedCards()); });
    aSetPT = new QAction(this);
    connect(aSetPT, &QAction::triggered, playerActions,
            [player, playerActions] { playerActions->actRequestSetPTDialog(player->getGameScene()->selectedCards()); });
    aResetPT = new QAction(this);
    connect(aResetPT, &QAction::triggered, playerActions,
            [player, playerActions] { playerActions->actResetPT(player->getGameScene()->selectedCards()); });

    addAction(aIncP);
    addAction(aDecP);
    addAction(aFlowP);
    addSeparator();
    addAction(aIncT);
    addAction(aDecT);
    addAction(aFlowT);
    addSeparator();
    addAction(aIncPT);
    addAction(aDecPT);
    addSeparator();
    addAction(aSetPT);
    addAction(aResetPT);

    setShortcutsActive();

    retranslateUi();
}

void PtMenu::retranslateUi()
{
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
}

void PtMenu::setShortcutsActive()
{
    ShortcutsSettings &shortcuts = SettingsCache::instance().shortcuts();

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
}