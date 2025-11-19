/**
 * @file interface_json_deck_parser.h
 * @ingroup ApiInterfaces
 * @brief TODO: Document this.
 */

#ifndef INTERFACE_JSON_DECK_PARSER_H
#define INTERFACE_JSON_DECK_PARSER_H
#include "../../../interface/deck_loader/deck_loader.h"

#include <QJsonArray>
#include <QJsonObject>

class IJsonDeckParser
{
public:
    virtual ~IJsonDeckParser() = default;

    virtual DeckLoader *parse(const QJsonObject &obj) = 0;
};

class ArchidektJsonParser : public IJsonDeckParser
{
public:
    DeckLoader *parse(const QJsonObject &obj) override
    {
        DeckLoader *loader = new DeckLoader(nullptr);

        QString deckName = obj.value("name").toString();
        QString deckDescription = obj.value("description").toString();

        loader->getDeckList()->setName(deckName);
        loader->getDeckList()->setComments(deckDescription);

        QString outputText;
        QTextStream outStream(&outputText);

        for (auto entry : obj.value("cards").toArray()) {
            auto quantity = entry.toObject().value("quantity").toInt();

            auto card = entry.toObject().value("card").toObject();
            auto oracleCard = card.value("oracleCard").toObject();
            QString cardName = oracleCard.value("name").toString();
            QString setName = card.value("edition").toObject().value("editioncode").toString().toUpper();
            QString collectorNumber = card.value("collectorNumber").toString();

            outStream << quantity << ' ' << cardName << " (" << setName << ") " << collectorNumber << '\n';
        }

        loader->getDeckList()->loadFromStream_Plain(outStream, false);
        DeckLoader::resolveSetNameAndNumberToProviderID(loader->getDeckList());

        return loader;
    }
};

class MoxfieldJsonParser : public IJsonDeckParser
{
public:
    DeckLoader *parse(const QJsonObject &obj) override
    {
        DeckLoader *loader = new DeckLoader(nullptr);

        QString deckName = obj.value("name").toString();
        QString deckDescription = obj.value("description").toString();

        loader->getDeckList()->setName(deckName);
        loader->getDeckList()->setComments(deckDescription);

        QString outputText;
        QTextStream outStream(&outputText);

        for (auto entry : obj.value("mainboard").toObject()) {
            auto quantity = entry.toObject().value("quantity").toInt();

            auto card = entry.toObject().value("card").toObject();
            QString cardName = card.value("name").toString();
            QString setName = card.value("set").toString().toUpper();
            QString collectorNumber = card.value("cn").toString();

            outStream << quantity << ' ' << cardName << " (" << setName << ") " << collectorNumber << '\n';
        }

        outStream << '\n';

        for (auto entry : obj.value("sideboard").toObject()) {
            auto quantity = entry.toObject().value("quantity").toInt();

            auto card = entry.toObject().value("card").toObject();
            QString cardName = card.value("name").toString();
            QString setName = card.value("set").toString().toUpper();
            QString collectorNumber = card.value("cn").toString();

            outStream << quantity << ' ' << cardName << " (" << setName << ") " << collectorNumber << '\n';
        }

        loader->getDeckList()->loadFromStream_Plain(outStream, false);
        DeckLoader::resolveSetNameAndNumberToProviderID(loader->getDeckList());

        QJsonObject commandersObj = obj.value("commanders").toObject();
        if (!commandersObj.isEmpty()) {
            for (auto it = commandersObj.begin(); it != commandersObj.end(); ++it) {
                QJsonObject cardData = it.value().toObject().value("card").toObject();
                QString commanderName = cardData.value("name").toString();
                QString setName = cardData.value("set").toString().toUpper();
                QString collectorNumber = cardData.value("cn").toString();
                QString providerId = cardData.value("scryfall_id").toString();

                loader->getDeckList()->setBannerCard({commanderName, providerId});
                loader->getDeckList()->addCard(commanderName, DECK_ZONE_MAIN, -1, setName, collectorNumber, providerId);
            }
        }

        return loader;
    }
};

#endif // INTERFACE_JSON_DECK_PARSER_H
