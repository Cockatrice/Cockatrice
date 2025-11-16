#include "oracleimporter.h"

#include "client/settings/cache_settings.h"
#include "libcockatrice/interfaces/noop_card_preference_provider.h"
#include "libcockatrice/interfaces/noop_card_set_priority_controller.h"
#include "parsehelpers.h"
#include "qt-json/json.h"

#include <QDebug>
#include <QRegularExpression>
#include <algorithm>
#include <climits>
#include <libcockatrice/card/database/parser/cockatrice_xml_4.h>
#include <libcockatrice/card/relation/card_relation.h>

SplitCardPart::SplitCardPart(const QString &_name,
                             const QString &_text,
                             const QVariantHash &_properties,
                             const PrintingInfo &_printingInfo)
    : name(_name), text(_text), properties(_properties), printingInfo(_printingInfo)
{
}

const QRegularExpression OracleImporter::formatRegex = QRegularExpression("^format-");

OracleImporter::OracleImporter(QObject *parent) : QObject(parent)
{
}

static CardSet::Priority getSetPriority(const QString &setType, const QString &shortName)
{
    if (!setTypePriorities.contains(setType.toLower())) {
        qDebug() << "warning: Set type" << setType << "unrecognized for prioritization";
    }
    CardSet::Priority priority = setTypePriorities.value(setType.toLower(), CardSet::PriorityOther);
    if (nonEnglishSets.contains(shortName)) {
        priority = CardSet::PriorityLowest;
    }
    return priority;
}

bool OracleImporter::readSetsFromByteArray(const QByteArray &data)
{
    bool ok;
    auto setsMap = QtJson::Json::parse(QString(data), ok).toMap().value("data").toMap();
    if (!ok) {
        qDebug() << "error: QtJson::Json::parse()";
        return false;
    }

    QList<SetToDownload> newSetList;

    QListIterator it(setsMap.values());

    while (it.hasNext()) {
        QVariantMap map = it.next().toMap();
        QString shortName = map.value("code").toString().toUpper();
        QString longName = map.value("name").toString();
        QList<QVariant> setCards = map.value("cards").toList();
        QString setType = map.value("type").toString();
        QDate releaseDate = map.value("releaseDate").toDate();
        CardSet::Priority priority = getSetPriority(setType, shortName);
        // capitalize set type
        if (setType.length() > 0) {
            // basic grammar for words that aren't capitalized, like in "From the Vault"
            const QStringList noCapitalize = {"the", "a", "an", "on", "to", "for", "of", "in", "and", "with", "or"};
            QStringList words = setType.split("_");
            setType.clear();
            bool first = false;
            for (auto &item : words) {
                if (first && noCapitalize.contains(item)) {
                    setType += item + QString(" ");
                } else {
                    setType += item[0].toUpper() + item.mid(1, -1) + QString(" ");
                    first = true;
                }
            }
            setType = setType.trimmed();
        }
        newSetList.append(SetToDownload(shortName, longName, setCards, priority, setType, releaseDate));
    }

    std::sort(newSetList.begin(), newSetList.end());

    if (newSetList.isEmpty()) {
        return false;
    }
    allSets = newSetList;
    return true;
}

static QString getMainCardType(const QStringList &typeList)
{
    if (typeList.isEmpty()) {
        return {};
    }

    static const QStringList typePriority = {"Planeswalker", "Creature", "Land",       "Sorcery",
                                             "Instant",      "Artifact", "Enchantment"};

    for (const auto &type : typePriority) {
        if (typeList.contains(type)) {
            return type;
        }
    }

    return typeList.first();
}

/**
 * Sorts and deduplicates the color chars in the string by WUBRG order.
 *
 * @param colors The string containing the color chars. Will be modified in-place
 */
static void sortAndReduceColors(QString &colors)
{
    // sort
    static const QHash<QChar, unsigned int> colorOrder{{'W', 0}, {'U', 1}, {'B', 2}, {'R', 3}, {'G', 4}};
    std::sort(colors.begin(), colors.end(),
              [](const QChar a, const QChar b) { return colorOrder.value(a, INT_MAX) < colorOrder.value(b, INT_MAX); });
    // reduce
    QChar lastChar = '\0';
    for (int i = 0; i < colors.size(); ++i) {
        if (colors.at(i) == lastChar)
            colors.remove(i, 1);
        else
            lastChar = colors.at(i);
    }
}

CardInfoPtr OracleImporter::addCard(QString name,
                                    const QString &text,
                                    bool isToken,
                                    QVariantHash properties,
                                    const QList<CardRelation *> &relatedCards,
                                    const PrintingInfo &printingInfo)
{
    // Workaround for card name weirdness
    name = name.replace("Æ", "AE");
    name = name.replace("’", "'");
    if (cards.contains(name)) {
        CardInfoPtr card = cards.value(name);
        card->addToSet(printingInfo.getSet(), printingInfo);
        if (card->getProperties().filter(formatRegex).empty()) {
            card->combineLegalities(properties);
        }
        return card;
    }

    // Remove {} around mana costs, except if it's split cost
    QString manacost = properties.value("manacost").toString();
    if (!manacost.isEmpty()) {
        QStringList symbols = manacost.split("}");
        QString formattedCardCost;
        for (QString symbol : symbols) {
            static const auto manaCostPattern = QRegularExpression("[0-9WUBGRP]/[0-9WUBGRP]");
            if (symbol.contains(manaCostPattern)) {
                symbol.append("}");
            } else {
                symbol.remove(QChar('{'));
            }
            formattedCardCost.append(symbol);
        }
        properties.insert("manacost", formattedCardCost);
    }

    // fix colors
    QString allColors = properties.value("colors").toString();
    if (allColors.size() > 1) {
        sortAndReduceColors(allColors);
        properties.insert("colors", allColors);
    }
    QString allColorIdent = properties.value("coloridentity").toString();
    if (allColorIdent.size() > 1) {
        sortAndReduceColors(allColorIdent);
        properties.insert("coloridentity", allColorIdent);
    }

    // DETECT CARD POSITIONING INFO

    bool landscapeOrientation = properties.value("maintype").toString() == "Battle" ||
                                properties.value("layout").toString() == "split" ||
                                properties.value("layout").toString() == "planar";

    // cards that enter the field tapped
    bool cipt = parseCipt(name, text) || landscapeOrientation;

    // table row
    int tableRow = 1;
    QString mainCardType = properties.value("maintype").toString();
    if (mainCardType == "Land")
        tableRow = 0;
    else if (mainCardType == "Sorcery" || mainCardType == "Instant")
        tableRow = 3;
    else if (mainCardType == "Creature")
        tableRow = 2;

    // card side
    QString side = properties.value("side").toString() == "b" ? "back" : "front";
    properties.insert("side", side);

    // upsideDown (flip cards)
    QString layout = properties.value("layout").toString();
    bool upsideDown = layout == "flip" && side == "back";

    // insert the card and its properties
    QList<CardRelation *> reverseRelatedCards;
    SetToPrintingsMap setsInfo;
    setsInfo[printingInfo.getSet()->getShortName()].append(printingInfo);
    CardInfoPtr newCard = CardInfo::newInstance(name, text, isToken, properties, relatedCards, reverseRelatedCards,
                                                setsInfo, cipt, landscapeOrientation, tableRow, upsideDown);

    if (name.isEmpty()) {
        qDebug() << "warning: an empty card was added to set" << printingInfo.getSet()->getShortName();
    }
    cards.insert(name, newCard);

    return newCard;
}

static QString getStringPropertyFromMap(const QVariantMap &card, const QString &propertyName)
{
    return card.contains(propertyName) ? card.value(propertyName).toString() : QString("");
}

int OracleImporter::importCardsFromSet(const CardSetPtr &currentSet, const QList<QVariant> &cardsList)
{
    // mtgjson name => xml name
    static const QMap<QString, QString> cardProperties{
        {"manaCost", "manacost"},     {"manaValue", "cmc"}, {"type", "type"},
        {"loyalty", "loyalty"},       {"layout", "layout"}, {"side", "side"},
        {"convertedManaCost", "cmc"}, // old name for manaValue, for backwards compatibility
    };

    // mtgjson name => xml name
    static const QMap<QString, QString> setInfoProperties{
        {"number", "num"}, {"rarity", "rarity"}, {"isOnlineOnly", "isOnlineOnly"}, {"isRebalanced", "isRebalanced"}};

    // mtgjson name => xml name
    static const QMap<QString, QString> identifierProperties{{"multiverseId", "muid"}, {"scryfallId", "uuid"}};

    static const QString ptSeparator = "/";
    static constexpr bool isToken = false;
    static const QList<QString> setsWithCardsWithSameNameButDifferentText = {"UST"};

    int numCards = 0;

    // Keeps track of any split card faces encountered so far
    QMap<QString, QPair<QList<SplitCardPart>, QString>> splitCards;

    // Keeps track of all names encountered so far
    QList<QString> allNameProps;

    for (const QVariant &cardVar : cardsList) {
        QVariantMap card = cardVar.toMap();

        /* Currently used layouts are:
         * augment, double_faced_token, flip, host, leveler, meld, normal, planar,
         * saga, scheme, split, token, transform, vanguard
         */
        QString layout = getStringPropertyFromMap(card, "layout");

        // don't import tokens from the json file
        if (layout == "token") {
            continue;
        }

        // normal cards handling
        QString name = getStringPropertyFromMap(card, "name");
        QString text = getStringPropertyFromMap(card, "text");
        QString faceName = getStringPropertyFromMap(card, "faceName");
        if (faceName.isEmpty()) {
            faceName = name;
        }

        // card properties
        QVariantHash properties;
        for (auto i = cardProperties.cbegin(), end = cardProperties.cend(); i != end; ++i) {
            QString mtgjsonProperty = i.key();
            QString xmlPropertyName = i.value();
            QString propertyValue = getStringPropertyFromMap(card, mtgjsonProperty);
            if (!propertyValue.isEmpty())
                properties.insert(xmlPropertyName, propertyValue);
        }

        // per-set properties
        PrintingInfo printingInfo = PrintingInfo(currentSet);
        for (auto i = setInfoProperties.cbegin(), end = setInfoProperties.cend(); i != end; ++i) {
            QString mtgjsonProperty = i.key();
            QString xmlPropertyName = i.value();
            QString propertyValue = getStringPropertyFromMap(card, mtgjsonProperty);
            if (!propertyValue.isEmpty())
                printingInfo.setProperty(xmlPropertyName, propertyValue);
        }

        // Identifiers
        for (auto i = identifierProperties.cbegin(), end = identifierProperties.cend(); i != end; ++i) {
            QString mtgjsonProperty = i.key();
            QString xmlPropertyName = i.value();
            QString propertyValue = getStringPropertyFromMap(card.value("identifiers").toMap(), mtgjsonProperty);
            if (!propertyValue.isEmpty()) {
                printingInfo.setProperty(xmlPropertyName, propertyValue);
            }
        }

        QString numComponent;
        const QString numProperty = printingInfo.getProperty("num");
        const QChar lastChar = numProperty.isEmpty() ? QChar() : numProperty.back();

        // Un-Sets do some wonky stuff. Split up these cards as individual entries.
        // these cards will have a num with a letter (abc) behind it, put that letter into the name
        if (setsWithCardsWithSameNameButDifferentText.contains(currentSet->getShortName()) &&
            allNameProps.contains(faceName) && layout == "normal" && lastChar.isLetter()) {
            numComponent = " (" + QString(lastChar).toLower() + ")";
        }
        allNameProps.append(faceName);

        // special handling properties
        QString colors = card.value("colors").toStringList().join("");
        if (!colors.isEmpty()) {
            properties.insert("colors", colors);
        }

        // special handling properties
        QString colorIdentity = card.value("colorIdentity").toStringList().join("");
        if (!colorIdentity.isEmpty()) {
            properties.insert("coloridentity", colorIdentity);
        }

        const auto &mainCardType = getMainCardType(card.value("types").toStringList());
        if (mainCardType.isEmpty()) {
            qDebug() << "warning: no mainCardType for card:" << name;
        } else {
            properties.insert("maintype", mainCardType);
        }

        // Depending on whether power and/or toughness are present, the format
        // is either P/T (most common), P (no toughness), or /T (no power).
        QString power = getStringPropertyFromMap(card, "power");
        QString toughness = getStringPropertyFromMap(card, "toughness");
        if (toughness.isEmpty() && !power.isEmpty()) {
            properties.insert("pt", power);
        } else if (!toughness.isEmpty()) {
            properties.insert("pt", power + ptSeparator + toughness);
        }

        auto legalities = card.value("legalities").toMap();
        for (auto i = legalities.cbegin(), end = legalities.cend(); i != end; ++i) {
            properties.insert(QString("format-%1").arg(i.key()), i.value().toString().toLower());
        }

        // split cards are considered a single card, enqueue for later merging
        if (layout == "split" || layout == "aftermath" || layout == "adventure") {
            auto _faceName = getStringPropertyFromMap(card, "faceName");
            SplitCardPart split(_faceName, text, properties, printingInfo);
            auto found_iter = splitCards.find(name + numProperty);
            if (found_iter == splitCards.end()) {
                splitCards.insert(name + numProperty, {{split}, name});
            } else if (layout == "adventure") {
                found_iter->first.insert(0, split);
            } else {
                found_iter->first.append(split);
            }
        } else {
            // relations
            QList<CardRelation *> relatedCards;

            // add other face for split cards as card relation
            if (!getStringPropertyFromMap(card, "side").isEmpty()) {
                auto faceManaValue = getStringPropertyFromMap(card, "faceManaValue");
                if (faceManaValue.isEmpty()) {
                    // check the old name for the property, for backwards compatibility purposes
                    faceManaValue = getStringPropertyFromMap(card, "faceConvertedManaCost");
                }
                properties["cmc"] = faceManaValue;

                if (layout == "meld") { // meld cards don't work
                    static const QRegularExpression meldNameRegex{"then meld them into ([^\\.]*)"};
                    QString additionalName = meldNameRegex.match(text).captured(1);
                    if (!additionalName.isNull()) {
                        relatedCards.append(new CardRelation(additionalName, CardRelationType::TransformInto));
                    }
                } else {
                    for (const QString &additionalName : name.split(" // ")) {
                        if (additionalName != faceName) {
                            relatedCards.append(new CardRelation(additionalName, CardRelationType::TransformInto));
                        }
                    }
                }
                name = faceName;
            }

            // mtgjon related cards
            if (card.contains("relatedCards")) {
                QVariantMap givenRelated = card.value("relatedCards").toMap();
                // conjured cards from a spellbook
                if (givenRelated.contains("spellbook")) {
                    auto spbk = givenRelated.value("spellbook").toStringList();
                    for (const QString &spbkName : spbk) {
                        relatedCards.append(
                            new CardRelation(spbkName, CardRelationType::DoesNotAttach, false, false, 1, true));
                    }
                }
            }

            CardInfoPtr newCard = addCard(name + numComponent, text, isToken, properties, relatedCards, printingInfo);
            numCards++;
        }
    }

    // split cards handling
    static const QString splitCardPropSeparator = QString(" // ");
    static const QString splitCardTextSeparator = QString("\n\n---\n\n");
    static const QList<CardRelation *> noRelatedCards = {};

    QList<QPair<QList<SplitCardPart>, QString>> partsAndNames = splitCards.values();
    for (auto [splitCardParts, name] : partsAndNames) {
        QString text;
        QVariantHash properties;
        PrintingInfo printingInfo;

        for (const SplitCardPart &tmp : splitCardParts) {
            if (!text.isEmpty()) {
                text.append(splitCardTextSeparator);
            }
            text.append(tmp.getText());

            if (properties.isEmpty()) {
                properties = tmp.getProperties();
                printingInfo = tmp.getPrintingInfo();
            } else {
                const QVariantHash &tmpProps = tmp.getProperties();
                for (auto i = tmpProps.cbegin(), end = tmpProps.cend(); i != end; ++i) {
                    QString prop = i.key();
                    QString originalPropertyValue = properties.value(prop).toString();
                    QString thisCardPropertyValue = i.value().toString();
                    if (!thisCardPropertyValue.isEmpty() && originalPropertyValue != thisCardPropertyValue) {
                        if (originalPropertyValue.isEmpty()) { // don't create //es if one field is empty
                            properties.insert(prop, thisCardPropertyValue);
                        } else if (prop == "colors") { // the card is both colors
                            properties.insert(prop, originalPropertyValue + thisCardPropertyValue);
                        } else if (prop == "maintype") { // don't create maintypes with //es in them
                            continue;
                        } else {
                            properties.insert(prop,
                                              originalPropertyValue + splitCardPropSeparator + thisCardPropertyValue);
                        }
                    }
                }
            }
        }
        CardInfoPtr newCard = addCard(name, text, isToken, properties, noRelatedCards, printingInfo);
        numCards++;
    }

    return numCards;
}

int OracleImporter::startImport()
{
    static ICardSetPriorityController *noOpController = new NoopCardSetPriorityController();

    // add an empty set for tokens
    CardSetPtr tokenSet =
        CardSet::newInstance(noOpController, CardSet::TOKENS_SETNAME, tr("Dummy set containing tokens"), "Tokens");
    sets.insert(CardSet::TOKENS_SETNAME, tokenSet);

    int setIndex = 0;

    for (const SetToDownload &curSetToParse : allSets) {
        CardSetPtr newSet = CardSet::newInstance(noOpController, curSetToParse.getShortName(),
                                                 curSetToParse.getLongName(), curSetToParse.getSetType(),
                                                 curSetToParse.getReleaseDate(), curSetToParse.getPriority());
        if (!sets.contains(newSet->getShortName()))
            sets.insert(newSet->getShortName(), newSet);

        int numCardsInSet = importCardsFromSet(newSet, curSetToParse.getCards());

        ++setIndex;

        emit setIndexChanged(numCardsInSet, setIndex, curSetToParse.getLongName());
    }

    emit setIndexChanged(0, setIndex, QString());

    // total number of sets
    return setIndex;
}

bool OracleImporter::saveToFile(const QString &fileName, const QString &sourceUrl, const QString &sourceVersion)
{
    CockatriceXml4Parser parser(new NoopCardPreferenceProvider());
    return parser.saveToFile(sets, cards, fileName, sourceUrl, sourceVersion);
}

void OracleImporter::clear()
{
    sets.clear();
    cards.clear();
    allSets.clear();
}
