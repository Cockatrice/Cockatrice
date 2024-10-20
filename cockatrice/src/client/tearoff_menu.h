#pragma once

#include "../settings/cache_settings.h"

#include <QMenu>

class TearOffMenu : public QMenu
{
public:
    TearOffMenu(const QString &title, QWidget *parent = nullptr) : QMenu(title, parent)
    {
        connect(&SettingsCache::instance(), &SettingsCache::useTearOffMenusChanged, this,
                [=](bool state) { setTearOffEnabled(state); });
        setTearOffEnabled(SettingsCache::instance().getUseTearOffMenus());
    }

    TearOffMenu(QWidget *parent = nullptr) : QMenu(parent)
    {
        connect(&SettingsCache::instance(), &SettingsCache::useTearOffMenusChanged, this,
                [=](bool state) { setTearOffEnabled(state); });
        setTearOffEnabled(SettingsCache::instance().getUseTearOffMenus());
    }

    TearOffMenu *addTearOffMenu(const QString &title)
    {
        TearOffMenu *menu = new TearOffMenu(title, this);
        addMenu(menu);
        return menu;
    }
};
