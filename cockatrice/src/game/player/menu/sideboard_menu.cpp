#include "sideboard_menu.h"

#include "../player_actions.h"
#include "../player_logic.h"

SideboardMenu::SideboardMenu(PlayerGraphicsItem *player, QMenu *playerMenu) : QMenu(playerMenu)
{
    aViewSideboard = new QAction(this);
    connect(aViewSideboard, &QAction::triggered, player->getPlayerLogic()->getPlayerActions(),
            &PlayerActions::actViewSideboard);

    if (player->getPlayerLogic()->getPlayerInfo()->getLocalOrJudge()) {
        addAction(aViewSideboard);
    }

    retranslateUi();
}

void SideboardMenu::retranslateUi()
{
    setTitle(tr("&Sideboard"));
    aViewSideboard->setText(tr("&View sideboard"));
}

void SideboardMenu::setShortcutsActive()
{
    ShortcutsSettings &shortcuts = SettingsCache::instance().shortcuts();

    aViewSideboard->setShortcuts(shortcuts.getShortcut("Player/aViewSideboard"));
}

void SideboardMenu::setShortcutsInactive()
{
    aViewSideboard->setShortcut(QKeySequence());
}