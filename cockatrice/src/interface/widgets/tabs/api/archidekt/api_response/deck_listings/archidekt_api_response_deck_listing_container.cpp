#include "archidekt_api_response_deck_listing_container.h"

#include "archidekt_api_response_deck_owner.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>

void ArchidektApiResponseDeckListingContainer::fromJson(const QJsonObject &json)
{
    id = json.value("id").toInt();
    name = json.value("name").toString();
    size = json.value("size").toInt();
    updatedAt = json.value("updatedAt").toString();
    createdAt = json.value("createdAt").toString();
    deckFormat = json.value("deckFormat").toInt();
    edhBracket = json.value("edhBracket").toInt();
    featured = json.value("featured").toString();
    customFeatured = json.value("customFeatured").toString();
    viewCount = json.value("viewCount").toInt();
    privateDeck = json.value("private").toBool();
    unlisted = json.value("unlisted").toBool();
    theoryCrafted = json.value("theoryCrafted").toBool();
    game = json.value("game").toString();
    hasDescription = json.value("hasDescription").toBool();
    // TODO
    // tags = {""};
    parentFolderId = json.value("parentFolderId").toInt();
    owner.fromJson(json.value("owner").toObject());

    auto colorsJson = json.value("colors").toObject();

    for (auto color : colorsJson.keys()) {
        colors[color] = colorsJson[color].toInt();
    }

    cardPackage = json.value("cardPackage").toString();
    contest = json.value("contest").toString();
}

void ArchidektApiResponseDeckListingContainer::debugPrint() const
{
    qDebug() << "Id:" << id;
    qDebug() << "Name:" << name;
    qDebug() << "Size:" << size;
    qDebug() << "UpdatedAt:" << updatedAt;
    qDebug() << "CreatedAt:" << createdAt;
    qDebug() << "DeckFormat:" << deckFormat;
    qDebug() << "EdhBracket:" << edhBracket;
    qDebug() << "Featured:" << featured;
    qDebug() << "CustomFeatured:" << customFeatured;
    qDebug() << "ViewCount:" << viewCount;
    qDebug() << "Private:" << privateDeck;
    qDebug() << "Unlisted:" << unlisted;
    qDebug() << "TheoryCrafted:" << theoryCrafted;
    qDebug() << "Game:" << game;
    qDebug() << "HasDescription:" << hasDescription;
    qDebug() << "Tags:" << tags;
    qDebug() << "ParentFolderId:" << parentFolderId;
    owner.debugPrint();
    qDebug() << "Colors:" << colors;
    qDebug() << "CardPackage" << cardPackage;
    qDebug() << "Contest:" << contest;
}
