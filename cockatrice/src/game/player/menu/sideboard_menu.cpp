#include "sideboard_menu.h"

#include "../player.h"
#include "../player_actions.h"

SideboardMenu::SideboardMenu(Player *player, QMenu *playerMenu) : QMenu(playerMenu)
{
    aViewSideboard = new QAction(this);
    connect(aViewSideboard, &QAction::triggered, player->getPlayerActions(), &PlayerActions::actViewSideboard);

    if (player->getPlayerInfo()->getLocalOrJudge()) {
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