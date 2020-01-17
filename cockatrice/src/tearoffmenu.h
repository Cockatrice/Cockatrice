#pragma once

#include "settingscache.h"

#include <QMenu>

class TearOffMenu : public QMenu
{
public:
    TearOffMenu(const QString &title, QWidget *parent = nullptr) : QMenu(title, parent)
    {
        connect(settingsCache, &SettingsCache::useTearOffMenusChanged, this,
                [=](bool state) { setTearOffEnabled(state); });
        setTearOffEnabled(settingsCache->getUseTearOffMenus());
    }

    TearOffMenu(QWidget *parent = nullptr) : QMenu(parent)
    {
        connect(settingsCache, &SettingsCache::useTearOffMenusChanged, this,
                [=](bool state) { setTearOffEnabled(state); });
        setTearOffEnabled(settingsCache->getUseTearOffMenus());
    }

    TearOffMenu *addTearOffMenu(const QString &title)
    {
        TearOffMenu *menu = new TearOffMenu(title, this);
        addMenu(menu);
        return menu;
    }
};
