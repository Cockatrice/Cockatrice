/**
 * @file tearoff_menu.h
 * @ingroup GameMenus
 * @brief TODO: Document this.
 */

#pragma once

#include <QMenu>
#include <libcockatrice/settings/cache_settings.h>

class TearOffMenu : public QMenu
{
public:
    explicit TearOffMenu(const QString &title, QWidget *parent = nullptr) : QMenu(title, parent)
    {
        connect(&SettingsCache::instance(), &SettingsCache::useTearOffMenusChanged, this,
                [this](const bool state) { setTearOffEnabled(state); });
        setTearOffEnabled(SettingsCache::instance().getUseTearOffMenus());
    }

    explicit TearOffMenu(QWidget *parent = nullptr) : QMenu(parent)
    {
        connect(&SettingsCache::instance(), &SettingsCache::useTearOffMenusChanged, this,
                [this](const bool state) { setTearOffEnabled(state); });
        setTearOffEnabled(SettingsCache::instance().getUseTearOffMenus());
    }

    TearOffMenu *addTearOffMenu(const QString &title)
    {
        auto *menu = new TearOffMenu(title, this);
        addMenu(menu);
        return menu;
    }
};
