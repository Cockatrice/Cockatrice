#include "dlg_load_deck.h"

#include "../deck/deck_loader.h"
#include "settings/cache_settings.h"

DlgLoadDeck::DlgLoadDeck(QWidget *parent) : QFileDialog(parent, tr("Load Deck"))
{
    QString startingDir = SettingsCache::instance().recents().getLatestDeckDirPath();
    if (startingDir.isEmpty()) {
        startingDir = SettingsCache::instance().getDeckPath();
    }

    setDirectory(startingDir);
    setNameFilters(DeckLoader::FILE_NAME_FILTERS);

    connect(this, &DlgLoadDeck::accepted, this, &DlgLoadDeck::actAccepted);
}

void DlgLoadDeck::actAccepted()
{
    SettingsCache::instance().recents().setLatestDeckDirPath(directory().absolutePath());
}