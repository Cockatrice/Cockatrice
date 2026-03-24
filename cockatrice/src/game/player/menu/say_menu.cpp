#include "say_menu.h"

#include "../../../client/settings/cache_settings.h"
#include "../player.h"
#include "../player_actions.h"

SayMenu::SayMenu(Player *_player) : player(_player)
{
    connect(&SettingsCache::instance().messages(), &MessageSettings::messageMacrosChanged, this, &SayMenu::initSayMenu);
    initSayMenu();
    retranslateUi();
}

void SayMenu::retranslateUi()
{
    setTitle(tr("S&ay"));
}

void SayMenu::setShortcutsActive()
{
    shortcutsActive = true;

    const auto menuActions = actions();
    for (int i = 0; i < menuActions.size() && i < 10; ++i) {
        menuActions[i]->setShortcut(QKeySequence("Ctrl+" + QString::number((i + 1) % 10)));
    }
}

void SayMenu::setShortcutsInactive()
{
    shortcutsActive = false;

    for (auto *action : actions()) {
        action->setShortcut(QKeySequence());
    }
}

void SayMenu::initSayMenu()
{
    clear();

    int count = SettingsCache::instance().messages().getCount();
    setEnabled(count > 0);

    for (int i = 0; i < count; ++i) {
        auto *newAction = new QAction(SettingsCache::instance().messages().getMessageAt(i), this);
        connect(newAction, &QAction::triggered, player->getPlayerActions(), &PlayerActions::actSayMessage);
        addAction(newAction);
    }

    if (shortcutsActive) {
        setShortcutsActive();
    }
}
