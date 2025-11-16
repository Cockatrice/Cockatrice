#include "recents_settings.h"

#define MAX_RECENT_DECK_COUNT 10

RecentsSettings::RecentsSettings(const QString &settingPath, QObject *parent)
    : SettingsManager(settingPath + "recents.ini", "deckbuilder", QString(), parent)
{
}

QStringList RecentsSettings::getRecentlyOpenedDeckPaths()
{
    return getValue("deckpaths").toStringList();
}
void RecentsSettings::clearRecentlyOpenedDeckPaths()
{
    deleteValue("deckpaths");
    emit recentlyOpenedDeckPathsChanged();
}
void RecentsSettings::updateRecentlyOpenedDeckPaths(const QString &deckPath)
{
    auto deckPaths = getValue("deckpaths").toStringList();
    deckPaths.removeAll(deckPath);

    deckPaths.prepend(deckPath);

    while (deckPaths.size() > MAX_RECENT_DECK_COUNT) {
        deckPaths.removeLast();
    }

    setValue(deckPaths, "deckpaths");
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