#include "loaded_deck.h"

LoadedDeck::FileFormat LoadedDeck::getFormatFromName(const QString &fileName)
{
    if (fileName.endsWith(".cod", Qt::CaseInsensitive)) {
        return CockatriceFormat;
    }
    return PlainTextFormat;
}

bool LoadedDeck::LoadInfo::isEmpty() const
{
    return fileName.isEmpty() && remoteDeckId == NON_REMOTE_ID;
}

bool LoadedDeck::isEmpty() const
{
    return deckList.isEmpty() && lastLoadInfo.isEmpty();
}