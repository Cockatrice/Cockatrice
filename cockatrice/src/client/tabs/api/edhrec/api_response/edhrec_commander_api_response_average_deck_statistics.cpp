#include "edhrec_commander_api_response_average_deck_statistics.h"

void EdhrecCommanderApiResponseAverageDeckStatistics::fromJson(const QJsonObject &json)
{
    creature = json.value("creature").toInt(0);
    instant = json.value("instant").toInt(0);
    sorcery = json.value("sorcery").toInt(0);
    artifact = json.value("artifact").toInt(0);
    enchantment = json.value("enchantment").toInt(0);
    battle = json.value("battle").toInt(0);
    planeswalker = json.value("planeswalker").toInt(0);
    land = json.value("land").toInt(0);
    basic = json.value("basic").toInt(0);
    nonbasic = json.value("nonbasic").toInt(0);
}