#include "edhrec_average_deck_api_response.h"

#include <QDebug>
#include <QJsonArray>

void EdhrecAverageDeckApiResponse::fromJson(const QJsonObject &json)
{
    // Parse the collapsed DeckStatistics
    deckStats.fromJson(json);

    // Parse Archidekt section
    QJsonArray archidektJson = json.value("archidekt").toArray();
    archidekt.fromJson(archidektJson);

    // Parse other fields
    similar = json.value("similar").toObject();
    header = json.value("header").toString();
    panels = json.value("panels").toObject();
    description = json.value("description").toString();
    QJsonObject containerJson = json.value("container").toObject();
    container.fromJson(containerJson);
    QJsonArray cardsJson = json.value("deck").toArray();
    deck.fromJson(cardsJson);
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
