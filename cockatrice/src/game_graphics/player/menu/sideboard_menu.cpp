#include "sideboard_menu.h"

#include "../../game/player/player_actions.h"
#include "../../game/player/player_logic.h"
#include "../player_graphics_item.h"

SideboardMenu::SideboardMenu(PlayerGraphicsItem *player, QMenu *playerMenu) : QMenu(playerMenu)
{
    aViewSideboard = new QAction(this);
    connect(aViewSideboard, &QAction::triggered, player->getLogic()->getPlayerActions(),
            &PlayerActions::actViewSideboard);

    if (player->getLogic()->getPlayerInfo()->getLocalOrJudge()) {
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