#include "recents_settings.h"

#define MAX_RECENT_DECK_COUNT 10

RecentsSettings::RecentsSettings(const QString &settingPath, QObject *parent)
    : SettingsManager(settingPath + "recents.ini", "deckbuilder", QString(), parent)
{
}

QStringList RecentsSettings::getRecentlyOpenedDeckPaths() const
{
    return getValue("deckPaths").toStringList();
}
void RecentsSettings::clearRecentlyOpenedDeckPaths()
{
    deleteValue("deckPaths");
    emit recentlyOpenedDeckPathsChanged();
}
void RecentsSettings::updateRecentlyOpenedDeckPaths(const QString &deckPath)
{
    auto deckPaths = getValue("deckPaths").toStringList();
    deckPaths.removeAll(deckPath);

    deckPaths.prepend(deckPath);

    while (deckPaths.size() > MAX_RECENT_DECK_COUNT) {
        deckPaths.removeLast();
    }

    setValue(deckPaths, "deckPaths");
    emit recentlyOpenedDeckPathsChanged();
}

QString RecentsSettings::getLatestDeckDirPath() const
{
    return getValue("latestDeckDir", "dirs").toString();
}

void RecentsSettings::setLatestDeckDirPath(const QString &dirPath)
{
    setValue(dirPath, "latestDeckDir", "dirs");
}