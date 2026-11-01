#include "edhrec_average_deck_api_response.h"

#include "../archidekt_links/edhrec_api_response_archidekt_links.h"
#include "../cards/edhrec_api_response_card_container.h"
#include "../commander/edhrec_commander_api_response_average_deck_statistics.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonValue>
#include <qlogging.h>
#include <utility>

bool EdhrecAverageDeckApiResponse::fromJson(const QJsonObject &json)
{
    if (!json.value("deck").isArray()) {
        return false;
    }
    EdhrecAverageDeckApiResponse parsed;
    if (!parsed.deck.fromJson(json.value("deck").toArray())) {
        return false;
    }

    parsed.deckStats.fromJson(json);
    parsed.archidekt.fromJson(json.value("archidekt").toArray());
    parsed.similar = json.value("similar").toObject();
    parsed.header = json.value("header").toString();
    parsed.panels = json.value("panels").toObject();
    parsed.description = json.value("description").toString();
    parsed.container.fromJson(json.value("container").toObject());

    parsed.deck.deck.setName(parsed.header.isEmpty() ? parsed.container.getTitle() : parsed.header);
    parsed.deck.deck.setComments(parsed.description.isEmpty() ? parsed.container.getDescription() : parsed.description);
    parsed.deck.deck.setGameFormat("commander");

    *this = std::move(parsed);
    return true;
}

void EdhrecAverageDeckApiResponse::debugPrint() const
{
    qDebug() << "Deck Statistics:";
    qDebug() << "  Creature:" << deckStats.creature;
    qDebug() << "  Instant:" << deckStats.instant;
    qDebug() << "  Sorcery:" << deckStats.sorcery;
    qDebug() << "  Artifact:" << deckStats.artifact;
    qDebug() << "  Enchantment:" << deckStats.enchantment;
    qDebug() << "  Battle:" << deckStats.battle;
    qDebug() << "  Planeswalker:" << deckStats.planeswalker;
    qDebug() << "  Land:" << deckStats.land;
    qDebug() << "  Basic:" << deckStats.basic;
    qDebug() << "  Nonbasic:" << deckStats.nonbasic;

    archidekt.debugPrint();

    qDebug() << "Similar:" << similar;
    qDebug() << "Header:" << header;
    qDebug() << "Panels:" << panels;
    qDebug() << "Description:" << description;
    container.debugPrint();
}
