#include "archidekt_api_response_card.h"

void ArchidektApiResponseCard::fromJson(const QJsonObject &json)
{
    id = json.value("id").toInt();
    artist = json.value("artist").toString();
    tcgProductId = json.value("tcgProductId").toInt();
    ckFoilId = json.value("ckFoilId").toInt();
    ckNormalId = json.value("ckNormalId").toInt();
    cmEd = json.value("cmEd").toString();
    scgSku = json.value("scgSku").toString();
    scgFoilSku = json.value("scgFoilSku").toString();
    collectorNumber = json.value("collectorNumber").toString();
    multiverseId = json.value("multiverseId").toInt();
    mtgoFoilId = json.value("mtgoFoilId").toInt();
    mtgoNormalId = json.value("mtgoNormalId").toInt();
    uid = json.value("uid").toString();
    displayName = json.value("displayName").toString();
    releasedAt = json.value("releasedAt").toString();

    edition.fromJson(json.value("edition").toObject());

    flavor = json.value("flavor").toString();
    // TODO but not really important
    // games = {""};
    // options = {""};
    scryfallImageHash = json.value("scryfallImageHash").toString();
    oracleCard = json.value("oracleCard").toObject();
    owned = json.value("owned").toInt();
    pinnedStatus = json.value("pinnedStatus").toInt();
    rarity = json.value("rarity").toString();
    // TODO but not really important
    // globalCategories = {""};
}

void ArchidektApiResponseCard::debugPrint() const
{
    qDebug() << "Id:" << id;
    qDebug() << "id:" << artist;
    qDebug() << "artist:" << tcgProductId;
    qDebug() << "tcgProductId:" << ckFoilId;
    qDebug() << "ckFoilId:" << ckNormalId;
    qDebug() << "ckNormalId:" << cmEd;
    qDebug() << "cmEd:" << scgSku;
    qDebug() << "scgSku:" << scgFoilSku;
    qDebug() << "scgFoilSku:" << collectorNumber;
    qDebug() << "collectorNumber:" << multiverseId;
    qDebug() << "multiverseId:" << mtgoFoilId;
    qDebug() << "mtgoFoilId:" << mtgoNormalId;
    qDebug() << "mtgoNormalId:" << uid;
    qDebug() << "uid:" << displayName;
    qDebug() << "displayName:" << releasedAt;
    qDebug() << "releasedAt:" << flavor;
    qDebug() << "flavor:" << games;
    qDebug() << "games:" << options;
    qDebug() << "options:" << scryfallImageHash;
    qDebug() << "scryfallImageHash:" << owned;
    qDebug() << "owned:" << pinnedStatus;
    qDebug() << "pinnedStatus:" << rarity;
    qDebug() << "rarity:" << globalCategories;
}