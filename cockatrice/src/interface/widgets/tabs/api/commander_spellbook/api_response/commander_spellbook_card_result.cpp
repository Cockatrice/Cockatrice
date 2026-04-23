#include "commander_spellbook_card_result.h"

void CommanderSpellbookCardResult::fromJson(const QJsonObject &json)
{
    id = json.value("id").toString();
    name = json.value("name").toString();
    oracleId = json.value("oracleId").toString();
    spoiler = json.value("spoiler").toBool();
    typeLine = json.value("typeLine").toString();

    imageUriFrontPng = json.value("imageUriFrontPng").toString();
    imageUriFrontLarge = json.value("imageUriFrontLarge").toString();
    imageUriFrontNormal = json.value("imageUriFrontNormal").toString();
    imageUriFrontSmall = json.value("imageUriFrontSmall").toString();
    imageUriFrontArtCrop = json.value("imageUriFrontArtCrop").toString();

    imageUriBackPng = json.value("imageUriBackPng").toString();
    imageUriBackLarge = json.value("imageUriBackLarge").toString();
    imageUriBackNormal = json.value("imageUriBackNormal").toString();
    imageUriBackSmall = json.value("imageUriBackSmall").toString();
    imageUriBackArtCrop = json.value("imageUriBackArtCrop").toString();
}
