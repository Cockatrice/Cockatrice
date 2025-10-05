#include "recents_settings.h"

#define MAX_RECENT_DECK_COUNT 10

RecentsSettings::RecentsSettings(const QString &settingPath, QObject *parent)
    : SettingsManager(settingPath + "recents.ini", parent)
{
}

QStringList RecentsSettings::getRecentlyOpenedDeckPaths()
{
    return getValue("deckpaths", "deckbuilder").toStringList();
}
void RecentsSettings::clearRecentlyOpenedDeckPaths()
{
    deleteValue("deckpaths", "deckbuilder");
    emit recentlyOpenedDeckPathsChanged();
}
void RecentsSettings::updateRecentlyOpenedDeckPaths(const QString &deckPath)
{
    auto deckPaths = getValue("deckpaths", "deckbuilder").toStringList();
    deckPaths.removeAll(deckPath);

    deckPaths.prepend(deckPath);

    while (deckPaths.size() > MAX_RECENT_DECK_COUNT) {
        deckPaths.removeLast();
    }

    setValue(deckPaths, "deckpaths", "deckbuilder");
    emit recentlyOpenedDeckPathsChanged();
}

QString RecentsSettings::getLatestDeckDirPath()
{
    return getValue("latestDeckDir", "dirs").toString();
}

void RecentsSettings::setLatestDeckDirPath(const QString &dirPath)
{
    setValue(dirPath, "latestDeckDir", "dirs");
}