/**
 * @file tab_settings.cpp
 * @ingroup Tabs
 */
#include "tab_settings.h"

#include "../../client/settings/cache_settings.h"
#include "../dialogs/settings_view.h"

#include <QCoreApplication>
#include <QDir>
#include <QLoggingCategory>
#include <QMessageBox>
#include <libcockatrice/card/database/card_database.h>
#include <libcockatrice/card/database/card_database_loader.h>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/settings/paths_settings.h>

inline Q_LOGGING_CATEGORY(TabSettingsLog, "tab_settings");

TabSettings::TabSettings(TabSupervisor *_tabSupervisor, int pageIndex) : Tab(_tabSupervisor)
{
    settingsView = new SettingsView(this);
    setCentralWidget(settingsView);
    if (pageIndex >= 0) {
        settingsView->setTab(pageIndex);
    }
}

void TabSettings::setTab(int pageIndex)
{
    settingsView->setTab(pageIndex);
}

void TabSettings::retranslateUi()
{
    // Nothing to do: SettingsView retranslates itself via langChanged and the
    // TabSupervisor refreshes the tab title from getTabText().
}

/**
 * @brief Validates paths and card database before the tab closes
 *
 * Moved verbatim from DlgSettings::closeEvent. The "DlgSettings" translation
 * context below is deliberate and preserves the existing translations.
 *
 * @return False when the user wants to go back and fix a setting, true otherwise
 */
bool TabSettings::closeRequest()
{
    // NOTE: "DlgSettings" contexts are intentional (translation continuity, see SettingsView).
    bool showLoadError = true;
    QString loadErrorMessage = QCoreApplication::translate("DlgSettings", "Unknown Error loading card database");
    LoadStatus loadStatus = CardDatabaseManager::getInstance()->getLoadStatus();
    qCInfo(TabSettingsLog) << "Card Database load status: " << loadStatus;
    switch (loadStatus) {
        case Ok:
            showLoadError = false;
            break;
        case Invalid:
            loadErrorMessage = QCoreApplication::translate(
                "DlgSettings", "Your card database is invalid.\n\n"
                               "Cockatrice may not function correctly with an invalid database\n\n"
                               "You may need to rerun Oracle to update your card database.\n\n"
                               "Would you like to change your database location setting?");
            break;
        case VersionTooOld:
            loadErrorMessage = QCoreApplication::translate(
                "DlgSettings", "Your card database version is too old.\n\n"
                               "This can cause problems loading card information or images\n\n"
                               "Usually this can be fixed by rerunning Oracle to to update your card database.\n\n"
                               "Would you like to change your database location setting?");
            break;
        case NotLoaded:
            loadErrorMessage = QCoreApplication::translate(
                "DlgSettings", "Your card database did not finish loading\n\n"
                               "Please file a ticket at https://github.com/Cockatrice/Cockatrice/issues with your "
                               "cards.xml attached\n\n"
                               "Would you like to change your database location setting?");
            break;
        case FileError:
            loadErrorMessage = QCoreApplication::translate(
                "DlgSettings", "File Error loading your card database.\n\n"
                               "Would you like to change your database location setting?");
            break;
        case NoCards:
            loadErrorMessage = QCoreApplication::translate(
                "DlgSettings", "Your card database was loaded but contains no cards.\n\n"
                               "Would you like to change your database location setting?");
            break;
        default:
            loadErrorMessage = QCoreApplication::translate(
                "DlgSettings", "Unknown card database load status\n\n"
                               "Please file a ticket at https://github.com/Cockatrice/Cockatrice/issues\n\n"
                               "Would you like to change your database location setting?");

            break;
    }

    if (showLoadError) {
        if (QMessageBox::critical(this, QCoreApplication::translate("DlgSettings", "Error"), loadErrorMessage,
                                  QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            return false;
        }
    }

    if (!QDir(SettingsCache::instance().paths().getDeckPath()).exists() ||
        SettingsCache::instance().paths().getDeckPath().isEmpty()) {
        if (QMessageBox::critical(this, QCoreApplication::translate("DlgSettings", "Error"),
                                  QCoreApplication::translate(
                                      "DlgSettings", "The path to your deck directory is invalid. Would you like to "
                                                     "go back and set the correct path?"),
                                  QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            return false;
        }
    }

    if (!QDir(SettingsCache::instance().paths().getPicsPath()).exists() ||
        SettingsCache::instance().paths().getPicsPath().isEmpty()) {
        if (QMessageBox::critical(
                this, QCoreApplication::translate("DlgSettings", "Error"),
                QCoreApplication::translate("DlgSettings", "The path to your card pictures directory is invalid. "
                                                           "Would you like to go back and set the correct path?"),
                QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            return false;
        }
    }
    return close();
}
