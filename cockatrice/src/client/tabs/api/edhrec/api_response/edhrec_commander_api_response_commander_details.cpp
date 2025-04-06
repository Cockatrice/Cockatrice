#include "edhrec_commander_api_response_commander_details.h"

#include <QDebug>

void EdhrecCommanderApiResponseCommanderDetails::fromJson(const QJsonObject &json)
{
    // Parse card-related data
    aetherhubUri = json.value("aetherhub_uri").toString();
    archidektUri = json.value("archidekt_uri").toString();
    cmc = json.value("cmc").toInt(0);
    colorIdentity = json.value("color_identity").toArray();
    combos = json.value("combos").toBool(false);
    deckstatsUri = json.value("deckstats_uri").toString();

    // Parse image URIs
    QJsonArray imageUrisArray = json.value("image_uris").toArray();
    for (const QJsonValue &imageValue : imageUrisArray) {
        QJsonObject imageObject = imageValue.toObject();
        imageUris.push_back(imageObject.value("normal").toString());
        imageUris.push_back(imageObject.value("art_crop").toString());
    }

    inclusion = json.value("inclusion").toInt(0);
    isCommander = json.value("is_commander").toBool(false);
    label = json.value("label").toString();
    layout = json.value("layout").toString();
    legalCommander = json.value("legal_commander").toBool(false);
    moxfieldUri = json.value("moxfield_uri").toString();
    mtggoldfishUri = json.value("mtggoldfish_uri").toString();
    name = json.value("name").toString();
    names = json.value("names").toArray();
    numDecks = json.value("num_decks").toInt(0);
    potentialDecks = json.value("potential_decks").toInt(0);
    precon = json.value("precon").toString();

    // Parse prices
    prices.fromJson(json.value("prices").toObject());

    primaryType = json.value("primary_type").toString();
    rarity = json.value("rarity").toString();
    salt = json.value("salt").toDouble(0.0);
    sanitized = json.value("sanitized").toString();
    sanitizedWo = json.value("sanitized_wo").toString();
    scryfallUri = json.value("scryfall_uri").toString();
    spellbookUri = json.value("spellbook_uri").toString();
    type = json.value("type").toString();
    url = json.value("url").toString();
}

void EdhrecCommanderApiResponseCommanderDetails::debugPrint() const
{
    qDebug() << "Card Data:";
    qDebug() << "Aetherhub URI:" << aetherhubUri;
    qDebug() << "Archidekt URI:" << archidektUri;
    qDebug() << "CMC:" << cmc;
    qDebug() << "Color Identity:" << colorIdentity;
    qDebug() << "Combos:" << combos;
    qDebug() << "Deckstats URI:" << deckstatsUri;

    qDebug() << "Image URIs:";
    for (const auto &uri : imageUris) {
        qDebug() << uri;
    }

    qDebug() << "Inclusion:" << inclusion;
    qDebug() << "Is Commander:" << isCommander;
    qDebug() << "Label:" << label;
    qDebug() << "Layout:" << layout;
    qDebug() << "Legal Commander:" << legalCommander;
    qDebug() << "Moxfield URI:" << moxfieldUri;
    qDebug() << "MTGGoldfish URI:" << mtggoldfishUri;
    qDebug() << "Name:" << name;
    qDebug() << "Names:" << names;
    qDebug() << "Number of Decks:" << numDecks;
    qDebug() << "Potential Decks:" << potentialDecks;
    qDebug() << "Precon:" << precon;

    // Print the prices using the debugPrint method from CardPrices
    prices.debugPrint();

    qDebug() << "Primary Type:" << primaryType;
    qDebug() << "Rarity:" << rarity;
    qDebug() << "Salt:" << salt;
    qDebug() << "Sanitized:" << sanitized;
    qDebug() << "Sanitized WO:" << sanitizedWo;
    qDebug() << "Scryfall URI:" << scryfallUri;
    qDebug() << "Spellbook URI:" << spellbookUri;
    qDebug() << "Type:" << type;
    qDebug() << "URL:" << url;
}