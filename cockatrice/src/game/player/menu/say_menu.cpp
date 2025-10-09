#include "say_menu.h"

#include "../player.h"
#include "../player_actions.h"

#include <libcockatrice/settings/cache_settings.h>

SayMenu::SayMenu(Player *_player) : player(_player)
{
    connect(&SettingsCache::instance().messages(), &MessageSettings::messageMacrosChanged, this, &SayMenu::initSayMenu);
    initSayMenu();
}

void SayMenu::initSayMenu()
{
    clear();

    int count = SettingsCache::instance().messages().getCount();
    setEnabled(count > 0);

    for (int i = 0; i < count; ++i) {
        auto *newAction = new QAction(SettingsCache::instance().messages().getMessageAt(i), this);
        if (i < 10) {
            newAction->setShortcut(QKeySequence("Ctrl+" + QString::number((i + 1) % 10)));
        }
        connect(newAction, &QAction::triggered, player->getPlayerActions(), &PlayerActions::actSayMessage);
        addAction(newAction);
    }
}