#include "edhrec_deck_api_response.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonValueConstRef>
#include <QRegularExpression>
#include <QString>
#include <QTextStream>
#include <functional>
#include <libcockatrice/card/import/card_name_normalizer.h>
#include <libcockatrice/deck_list/tree/deck_list_card_node.h>
#include <qlogging.h>
#include <utility>

bool EdhrecDeckApiResponse::fromJson(const QJsonArray &json)
{
    if (json.isEmpty()) {
        return false;
    }
    QString deckList;
    static const QRegularExpression cardLine(R"(^\s*[\w\[\(\{].*$)", QRegularExpression::UseUnicodePropertiesOption);
    for (const QJsonValue &cardlistValue : json) {
        const QString line = cardlistValue.toString();
        if (!cardlistValue.isString() || !cardLine.match(line).hasMatch() || line.contains('\n') ||
            line.contains('\r')) {
            return false;
        }
        deckList += line + "\n";
    }

    DeckList parsedDeck;
    QTextStream stream(&deckList);
    // The plain-text parser creates a node for each accepted row, including repeated card names.
    // Require every API row to survive parsing rather than silently importing a partial deck.
    if (!parsedDeck.loadFromStream_Plain(stream, false, CardNameNormalizer()) ||
        parsedDeck.getCardNodes().size() != json.size()) {
        return false;
    }
    for (const auto *card : parsedDeck.getCardNodes()) {
        if (card->getNumber() <= 0 || card->getName().trimmed().isEmpty()) {
            return false;
        }
    }
    deck = std::move(parsedDeck);
    return true;
}

void EdhrecDeckApiResponse::debugPrint() const
{
    qDebug() << "Breadcrumb:";
}
