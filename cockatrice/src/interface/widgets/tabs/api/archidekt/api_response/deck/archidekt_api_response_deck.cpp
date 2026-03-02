#include "archidekt_api_response_deck.h"

#include "../card/archidekt_api_response_card_entry.h"

void ArchidektApiResponseDeck::fromJson(const QJsonObject &json)
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
    points = json.value("points").toInt();
    userInput = json.value("userInput").toInt();
    owner.fromJson(json.value("owner").toObject());
    commentRoot = json.value("commentRoot").toInt();
    editors = json.value("editors").toString();
    parentFolderId = json.value("parentFolderId").toInt();
    bookmarked = json.value("bookmarked").toBool();

    auto categoriesJson = json.value("categories").toArray();
    for (auto category : categoriesJson) {
        ArchidektApiResponseDeckCategory categoryEntry;
        categoryEntry.fromJson(category.toObject());
        categories.append(categoryEntry);
    }

    // deckTags = {""};
    playgroupDeckUrl = json.value("playgroupDeckUrl").toString();
    cardPackage = json.value("cardPackage").toString();

    auto cardsObject = json.value("cards").toArray();

    for (auto card : cardsObject) {
        ArchidektApiResponseCardEntry entry;
        entry.fromJson(card.toObject());
        cards.append(entry);
    }

    // TODO but not really important
    // customCards = {""};
}

void ArchidektApiResponseDeck::debugPrint() const
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
    qDebug() << "Points:" << points;
    qDebug() << "UserInput:" << userInput;
    owner.debugPrint();
    qDebug() << "CommentRoot:" << commentRoot;
    qDebug() << "Editors:" << editors;
    qDebug() << "ParentFolderId:" << parentFolderId;
    qDebug() << "Bookmarked:" << bookmarked;
    qDebug() << "DeckTags:" << deckTags;
    qDebug() << "PlaygroupDeckUrl:" << playgroupDeckUrl;
    qDebug() << "CardPackage:" << cardPackage;
    for (auto card : cards) {
        card.debugPrint();
    }
}