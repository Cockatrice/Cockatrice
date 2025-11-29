#include "deck_link_to_api_transformer.h"

#include <QRegularExpression>

namespace DeckLinkToApiTransformer
{

static const QString TAPPEDOUT_BASE = "https://tappedout.net/mtg-decks/";
static const QString TAPPEDOUT_SUFFIX = "/?fmt=txt";

static const QString ARCHIDEKT_BASE = "https://archidekt.com/api/decks/";
static const QString ARCHIDEKT_SUFFIX = "/?format=json";

static const QString MOXFIELD_BASE = "https://api.moxfield.com/v2/decks/all/";
static const QString MOXFIELD_SUFFIX = "/";

static const QString DECKSTATS_SUFFIX = "?include_comments=1&export_mtgarena=1";

bool parseDeckUrl(const QString &url, ParsedDeckInfo &outInfo)
{
    static QRegularExpression rxTappedOut("tappedout\\.net/(?:mtg-decks/)?([^/?#]+)");
    static QRegularExpression rxArchidekt("archidekt\\.com/decks/(\\d+)");
    static QRegularExpression rxMoxfield("moxfield\\.com/decks/([a-zA-Z0-9_-]+)");
    static QRegularExpression rxDeckstats("deckstats\\.net/decks/(\\d+/[a-zA-Z0-9_-]+)");

    QRegularExpressionMatch match;

    if ((match = rxTappedOut.match(url)).hasMatch()) {
        const QString slug = match.captured(1);
        outInfo = ParsedDeckInfo{.baseUrl = TAPPEDOUT_BASE,
                                 .deckID = slug,
                                 .fullUrl = TAPPEDOUT_BASE + slug + TAPPEDOUT_SUFFIX,
                                 .provider = DeckProvider::TappedOut};
        return true;
    } else if ((match = rxArchidekt.match(url)).hasMatch()) {
        const QString deckID = match.captured(1);
        outInfo = ParsedDeckInfo{.baseUrl = ARCHIDEKT_BASE,
                                 .deckID = deckID,
                                 .fullUrl = ARCHIDEKT_BASE + deckID + ARCHIDEKT_SUFFIX,
                                 .provider = DeckProvider::Archidekt};
        return true;
    } else if ((match = rxMoxfield.match(url)).hasMatch()) {
        const QString deckID = match.captured(1);
        outInfo = ParsedDeckInfo{.baseUrl = MOXFIELD_BASE,
                                 .deckID = deckID,
                                 .fullUrl = MOXFIELD_BASE + deckID + MOXFIELD_SUFFIX,
                                 .provider = DeckProvider::Moxfield};
        return true;
    } else if ((match = rxDeckstats.match(url)).hasMatch()) {
        const QString deckPath = match.captured(1);
        outInfo = ParsedDeckInfo{.baseUrl = "https://deckstats.net/decks/",
                                 .deckID = deckPath,
                                 .fullUrl = "https://deckstats.net/decks/" + deckPath + DECKSTATS_SUFFIX,
                                 .provider = DeckProvider::Deckstats};
        return true;
    }

    return false;
}

} // namespace DeckLinkToApiTransformer
