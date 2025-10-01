/**
 * @file deck_link_to_api_transformer.h
 * @ingroup ApiInterfaces
 * @brief TODO: Document this.
 */

#ifndef DECK_LINK_TO_API_TRANSFORMER_H
#define DECK_LINK_TO_API_TRANSFORMER_H

#include <QString>

enum class DeckProvider
{
    TappedOut,
    Archidekt,
    Moxfield,
    Deckstats,
    Unknown
};

struct ParsedDeckInfo
{
    QString baseUrl;
    QString deckID;
    QString fullUrl;
    DeckProvider provider;
};

namespace DeckLinkToApiTransformer
{

// Returns true if the input URL is recognized and fills outInfo.
bool parseDeckUrl(const QString &url, ParsedDeckInfo &outInfo);

} // namespace DeckLinkToApiTransformer

#endif // DECK_LINK_TO_API_TRANSFORMER_H
