/**
 * @file tab_settings.h
 * @brief Settings tab hosting the shared SettingsView
 * @ingroup Tabs
 */
#ifndef TAB_SETTINGS_H
#define TAB_SETTINGS_H

#include "tab.h"

#include <QString>
#include <qtmetamacros.h>

class SettingsView;
class TabSupervisor;

/**
 * @brief Singleton settings tab owned by the TabSupervisor
 *
 * Thin Tab frame around SettingsView: page selection, tab title and the close
 * validation live here. All settings content is owned by SettingsView.
 */
class TabSettings : public Tab
{
    Q_OBJECT
private:
    SettingsView *settingsView;

public:
    explicit TabSettings(TabSupervisor *_tabSupervisor, int pageIndex = -1);
    /** @brief Selects the given SettingsView page (range-checked, no-op when out of range) */
    void setTab(int pageIndex);
    /** @brief The hosted settings content */
    [[nodiscard]] SettingsView *getSettingsView() const
    {
        return settingsView;
    }
    void retranslateUi() override;
    [[nodiscard]] QString getTabText() const override
    {
        return tr("Settings");
    }
    bool closeRequest() override;
};

#endif // TAB_SETTINGS_H
