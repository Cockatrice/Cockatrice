/**
 * @file tearoff_menu.h
 * @ingroup GameMenus
 */
//! \todo Document this file.

#pragma once

#include "../../../client/settings/cache_settings.h"

#include <QMenu>
#include <libcockatrice/settings/interface_settings.h>

class TearOffMenu : public QMenu
{
public:
    explicit TearOffMenu(const QString &title, QWidget *parent = nullptr) : QMenu(title, parent)
    {
        connect(&SettingsCache::instance().userInterface(), &InterfaceSettings::useTearOffMenusChanged, this,
                [this](const bool state) { setTearOffEnabled(state); });
        setTearOffEnabled(SettingsCache::instance().userInterface().getUseTearOffMenus());
    }

    explicit TearOffMenu(QWidget *parent = nullptr) : QMenu(parent)
    {
        connect(&SettingsCache::instance().userInterface(), &InterfaceSettings::useTearOffMenusChanged, this,
                [this](const bool state) { setTearOffEnabled(state); });
        setTearOffEnabled(SettingsCache::instance().userInterface().getUseTearOffMenus());
    }

    TearOffMenu *addTearOffMenu(const QString &title)
    {
        auto *menu = new TearOffMenu(title, this);
        addMenu(menu);
        return menu;
    }
};
