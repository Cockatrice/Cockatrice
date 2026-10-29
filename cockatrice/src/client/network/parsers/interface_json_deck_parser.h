/**
 * @file interface_json_deck_parser.h
 * @ingroup ApiInterfaces
 */
//! \todo Document this file.

#ifndef INTERFACE_JSON_DECK_PARSER_H
#define INTERFACE_JSON_DECK_PARSER_H

#include "../../../interface/deck_loader/card_node_function.h"
#include "libcockatrice/deck_list/tree/inner_deck_list_node.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QString>
#include <QTextStream>
#include <libcockatrice/card/import/card_name_normalizer.h>
#include <libcockatrice/deck_list/deck_list.h>
#include <qobject.h>

inline Q_LOGGING_CATEGORY(InterfaceJsonDeckParserLog, "interface_json_deck_parser");

class IJsonDeckParser
{
public:
    virtual ~IJsonDeckParser() = default;

    virtual DeckList parse(const QJsonObject &obj) = 0;
};

class ArchidektJsonParser : public IJsonDeckParser
{
public:
    DeckList parse(const QJsonObject &obj) override
    {
        DeckList deckList;

        QString deckName = obj.value("name").toString();
        QString deckDescription = obj.value("description").toString();

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

        deckList.loadFromStream_Plain(outStream, false, CardNameNormalizer());
        deckList.forEachCard(CardNodeFunction::ResolveProviderId());
        deckList.setName(deckName);
        deckList.setComments(deckDescription);

        return deckList;
    }
};

class MoxfieldJsonParser : public IJsonDeckParser
{
public:
    DeckList parse(const QJsonObject &obj) override
    {
        DeckList deckList;

        QString deckName = obj.value("name").toString();
        QString deckDescription = obj.value("description").toString();

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

        deckList.loadFromStream_Plain(outStream, false, CardNameNormalizer());
        deckList.forEachCard(CardNodeFunction::ResolveProviderId());
        deckList.setName(deckName);
        deckList.setComments(deckDescription);

        QJsonObject commandersObj = obj.value("commanders").toObject();
        if (!commandersObj.isEmpty()) {
            for (auto it = commandersObj.begin(); it != commandersObj.end(); ++it) {
                QJsonObject cardData = it.value().toObject().value("card").toObject();
                QString commanderName = cardData.value("name").toString();
                QString setName = cardData.value("set").toString().toUpper();
                QString collectorNumber = cardData.value("cn").toString();
                QString providerId = cardData.value("scryfall_id").toString();

                deckList.setBannerCard({commanderName, providerId});
                deckList.addCard(commanderName, DECK_ZONE_MAIN, -1, setName, collectorNumber, providerId);
            }
        }

        return deckList;
    }
};

class ManaboxJsonParser : public IJsonDeckParser
{
    enum BoardType : quint32
    {
        MainBoard = 0,
        SideBoard = 1,
        Commander = 2,
        MaybeBoard = 3,
    };

public:
    DeckList parse(const QJsonObject &obj) override
    {
        DeckList deckList;

        QString deckName = obj.value("name").toString();

        QString outputText;
        QString mainboardText;
        QString sideboardText;
        QTextStream mainboardStream(&mainboardText);
        QTextStream sideboardStream(&sideboardText);
        QTextStream outStream(&outputText);

        auto commanderList = QList<QJsonObject>();
        auto maybeboardList = QList<QJsonObject>();
        for (auto entry : obj.value("cards").toArray()) {
            auto card = entry.toObject();

            auto collectorNumber = card.value("collectorNumber").toString();
            auto cardName = card.value("name").toString();
            auto boardType = static_cast<BoardType>(card.value("boardCategory").toInt());
            auto setName = card.value("setId").toString();
            auto quantity = card.value("quantity").toInt();
            auto providerId = card.value("scryfallId").toString();

            switch (boardType) {
                case MainBoard:
                    mainboardStream << quantity << ' ' << cardName << " (" << setName << ") " << collectorNumber
                                    << '\n';
                    break;
                case SideBoard:
                    sideboardStream << "SB: " << quantity << ' ' << cardName << " (" << setName << ") "
                                    << collectorNumber << '\n';
                    break;
                case MaybeBoard:
                    maybeboardList.append(card);
                    break;
                case Commander:
                    commanderList.append(card);
                    break;
                default:
                    qCWarning(InterfaceJsonDeckParserLog) << "Unknown ManaBox Zone";
                    break;
            }
        }

        outStream << mainboardText << sideboardText;

        deckList.loadFromStream_Plain(outStream, false, CardNameNormalizer());
        deckList.setName(deckName);
        deckList.forEachCard(CardNodeFunction::ResolveProviderId());

        if (!maybeboardList.isEmpty()) {
            for (const auto &maybeboardCard : maybeboardList) {
                QString cardName = maybeboardCard.value("name").toString();
                QString setName = maybeboardCard.value("setId").toString().toUpper();
                QString collectorNumber = maybeboardCard.value("collectorNumber").toString();
                QString providerId = maybeboardCard.value("scryfallId").toString();
                auto quantity = maybeboardCard.value("quantity").toInt();

                for (int x = 0; x < quantity; x++) {
                    deckList.addCard(cardName, DECK_ZONE_MAYBEBOARD, -1, setName, collectorNumber, providerId);
                }
            }
        }
        if (!commanderList.isEmpty()) {
            for (const auto &commanderCard : commanderList) {

                QString commanderName = commanderCard.value("name").toString();
                QString setName = commanderCard.value("setId").toString().toUpper();
                QString collectorNumber = commanderCard.value("collectorNumber").toString();
                QString providerId = commanderCard.value("scryfallId").toString();

                deckList.setBannerCard({commanderName, providerId});
                deckList.addCard(commanderName, DECK_ZONE_MAIN, -1, setName, collectorNumber, providerId);
            }
        }

        return deckList;
    }
};

#endif // INTERFACE_JSON_DECK_PARSER_H
