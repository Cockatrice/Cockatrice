#include "loaded_deck.h"

bool LoadedDeck::LoadInfo::isEmpty() const
{
    return fileName.isEmpty() && remoteDeckId == NON_REMOTE_ID;
}

bool LoadedDeck::isEmpty() const
{
    return deckList.isEmpty() && lastLoadInfo.isEmpty();
}