#include "commander_spellbook_card_result.h"

CommanderSpellbookCardResult CommanderSpellbookCardResult::fromJson(const QJsonObject &json)
{
    CommanderSpellbookCardResult result;

    result.id = json.value("id").toString();
    result.name = json.value("name").toString();
    result.oracleId = json.value("oracleId").toString();
    result.spoiler = json.value("spoiler").toBool();
    result.typeLine = json.value("typeLine").toString();

    result.imageUriFrontPng = json.value("imageUriFrontPng").toString();
    result.imageUriFrontLarge = json.value("imageUriFrontLarge").toString();
    result.imageUriFrontNormal = json.value("imageUriFrontNormal").toString();
    result.imageUriFrontSmall = json.value("imageUriFrontSmall").toString();
    result.imageUriFrontArtCrop = json.value("imageUriFrontArtCrop").toString();

    result.imageUriBackPng = json.value("imageUriBackPng").toString();
    result.imageUriBackLarge = json.value("imageUriBackLarge").toString();
    result.imageUriBackNormal = json.value("imageUriBackNormal").toString();
    result.imageUriBackSmall = json.value("imageUriBackSmall").toString();
    result.imageUriBackArtCrop = json.value("imageUriBackArtCrop").toString();

    return result;
}
