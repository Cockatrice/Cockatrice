#include "oracleimporter.h"

#include "carddbparser/cockatricexml4.h"
#include "qt-json/json.h"

#include <QtWidgets>
#include <algorithm>
#include <climits>

SplitCardPart::SplitCardPart(const QString &_name,
                             const QString &_text,
                             const QVariantHash &_properties,
                             const CardInfoPerSet _setInfo)
    : name(_name), text(_text), properties(_properties), setInfo(_setInfo)
{
}

OracleImporter::OracleImporter(const QString &_dataDir, QObject *parent) : CardDatabase(parent), dataDir(_dataDir)
{
}

bool OracleImporter::readSetsFromByteArray(const QByteArray &data)
{
    QList<SetToDownload> newSetList;

    bool ok;
    setsMap = QtJson::Json::parse(QString(data), ok).toMap().value("data").toMap();
    if (!ok) {
        qDebug() << "error: QtJson::Json::parse()";
        return false;
    }

    QListIterator<QVariant> it(setsMap.values());
    QVariantMap map;

    QString shortName;
    QString longName;
    QList<QVariant> setCards;
    QString setType;
    QDate releaseDate;

    while (it.hasNext()) {
        map = it.next().toMap();
        shortName = map.value("code").toString().toUpper();
        longName = map.value("name").toString();
        setCards = map.value("cards").toList();
        setType = map.value("type").toString();
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
        if (!nonEnglishSets.contains(shortName)) {
            releaseDate = map.value("releaseDate").toDate();
        } else {
            releaseDate = QDate();
        }
        newSetList.append(SetToDownload(shortName, longName, setCards, setType, releaseDate));
    }

    std::sort(newSetList.begin(), newSetList.end());

    if (newSetList.isEmpty()) {
        return false;
    }
    allSets = newSetList;
    return true;
}

QString OracleImporter::getMainCardType(const QStringList &typeList)
{
    if (typeList.isEmpty()) {
        return {};
    }

    for (const auto &type : mainCardTypes) {
        if (typeList.contains(type)) {
            return type;
        }
    }

    return typeList.first();
}

CardInfoPtr OracleImporter::addCard(QString name,
                                    QString text,
                                    bool isToken,
                                    QVariantHash properties,
                                    QList<CardRelation *> &relatedCards,
                                    CardInfoPerSet setInfo)
{
    // Workaround for card name weirdness
    name = name.replace("Æ", "AE");
    name = name.replace("’", "'");
    if (cards.contains(name)) {
        CardInfoPtr card = cards.value(name);
        card->addToSet(setInfo.getPtr(), setInfo);
        return card;
    }

    // Remove {} around mana costs, except if it's split cost
    QString manacost = properties.value("manacost").toString();
    if (!manacost.isEmpty()) {
        QStringList symbols = manacost.split("}");
        QString formattedCardCost;
        for (QString symbol : symbols) {
            if (symbol.contains(QRegularExpression("[0-9WUBGRP]/[0-9WUBGRP]"))) {
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

    // cards that enter the field tapped
    bool cipt = text.contains(" it enters the battlefield tapped") ||
                (text.contains(name + " enters the battlefield tapped") &&
                 !text.contains(name + " enters the battlefield tapped unless"));

    // table row
    int tableRow = 1;
    QString mainCardType = properties.value("maintype").toString();
    if ((mainCardType == "Land"))
        tableRow = 0;
    else if ((mainCardType == "Sorcery") || (mainCardType == "Instant"))
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
    CardInfoPerSetMap setsInfo;
    setsInfo.insert(setInfo.getPtr()->getShortName(), setInfo);
    CardInfoPtr newCard = CardInfo::newInstance(name, text, isToken, properties, relatedCards, reverseRelatedCards,
                                                setsInfo, cipt, tableRow, upsideDown);

    if (name.isEmpty()) {
        qDebug() << "warning: an empty card was added to set" << setInfo.getPtr()->getShortName();
    }
    cards.insert(name, newCard);

    return newCard;
}

QString OracleImporter::getStringPropertyFromMap(const QVariantMap &card, const QString &propertyName)
{
    return card.contains(propertyName) ? card.value(propertyName).toString() : QString("");
}

int OracleImporter::importCardsFromSet(const CardSetPtr &currentSet,
                                       const QList<QVariant> &cardsList,
                                       bool skipSpecialCards)
{
    // mtgjson name => xml name
    static const QMap<QString, QString> cardProperties{
        {"manaCost", "manacost"}, {"convertedManaCost", "cmc"}, {"type", "type"},
        {"loyalty", "loyalty"},   {"layout", "layout"},         {"side", "side"},
    };

    // mtgjson name => xml name
    static const QMap<QString, QString> setInfoProperties{{"number", "num"}, {"rarity", "rarity"}};

    // mtgjson name => xml name
    static const QMap<QString, QString> identifierProperties{{"multiverseId", "muid"}, {"scryfallId", "uuid"}};

    int numCards = 0;
    QMap<QString, QList<SplitCardPart>> splitCards;
    QString ptSeparator("/");
    QVariantMap card;
    QString layout, name, text, colors, colorIdentity, maintype, faceName;
    static const bool isToken = false;
    QVariantHash properties;
    CardInfoPerSet setInfo;
    QList<CardRelation *> relatedCards;
    static const QList<QString> specialNumChars = {"★", "s", "†"};
    QMap<QString, QVariant> specialPromoCards;
    QList<QString> allNameProps;

    for (const QVariant &cardVar : cardsList) {
        card = cardVar.toMap();

        // skip alternatives
        if (getStringPropertyFromMap(card, "isAlternative") == "true") {
            continue;
        }

        /* Currently used layouts are:
         * augment, double_faced_token, flip, host, leveler, meld, normal, planar,
         * saga, scheme, split, token, transform, vanguard
         */
        layout = getStringPropertyFromMap(card, "layout");

        // don't import tokens from the json file
        if (layout == "token") {
            continue;
        }

        // normal cards handling
        name = getStringPropertyFromMap(card, "name");
        text = getStringPropertyFromMap(card, "text");
        faceName = getStringPropertyFromMap(card, "faceName");
        if (faceName.isEmpty()) {
            faceName = name;
        }

        // card properties
        properties.clear();
        QMapIterator<QString, QString> it(cardProperties);
        while (it.hasNext()) {
            it.next();
            QString mtgjsonProperty = it.key();
            QString xmlPropertyName = it.value();
            QString propertyValue = getStringPropertyFromMap(card, mtgjsonProperty);
            if (!propertyValue.isEmpty())
                properties.insert(xmlPropertyName, propertyValue);
        }

        // per-set properties
        setInfo = CardInfoPerSet(currentSet);
        QMapIterator<QString, QString> it2(setInfoProperties);
        while (it2.hasNext()) {
            it2.next();
            QString mtgjsonProperty = it2.key();
            QString xmlPropertyName = it2.value();
            QString propertyValue = getStringPropertyFromMap(card, mtgjsonProperty);
            if (!propertyValue.isEmpty())
                setInfo.setProperty(xmlPropertyName, propertyValue);
        }

        // Identifiers
        QMapIterator<QString, QString> it3(identifierProperties);
        while (it3.hasNext()) {
            it3.next();
            auto mtgjsonProperty = it3.key();
            auto xmlPropertyName = it3.value();
            auto propertyValue = getStringPropertyFromMap(card.value("identifiers").toMap(), mtgjsonProperty);
            if (!propertyValue.isEmpty()) {
                setInfo.setProperty(xmlPropertyName, propertyValue);
            }
        }

        QString numComponent{};
        if (skipSpecialCards) {
            QString numProperty = setInfo.getProperty("num");
            // skip promo cards if it's not the only print, cards with two faces are different cards
            if (allNameProps.contains(faceName)) {
                // check for alternative versions
                if (layout != "normal")
                    continue;

                // alternative versions have a letter in the end of num like abc
                // note this will also catch p and s, those will get removed later anyway
                QChar lastChar = numProperty.at(numProperty.size() - 1);
                if (!lastChar.isLetter())
                    continue;

                numComponent = " (" + QString(lastChar) + ")";
                faceName += numComponent; // add to facename to make it unique
            }
            if (getStringPropertyFromMap(card, "isPromo") == "true") {
                specialPromoCards.insert(faceName, cardVar);
                continue;
            }
            bool skip = false;
            // skip cards containing special stuff in the collectors number like promo cards
            for (const QString &specialChar : specialNumChars) {
                if (numProperty.contains(specialChar)) {
                    skip = true;
                    break;
                }
            }
            if (skip) {
                specialPromoCards.insert(faceName, cardVar);
                continue;
            } else {
                allNameProps.append(faceName);
            }
        }

        // special handling properties
        colors = card.value("colors").toStringList().join("");
        if (!colors.isEmpty()) {
            properties.insert("colors", colors);
        }

        // special handling properties
        colorIdentity = card.value("colorIdentity").toStringList().join("");
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
        for (const QString &fmtName : legalities.keys()) {
            properties.insert(QString("format-%1").arg(fmtName), legalities.value(fmtName).toString().toLower());
        }

        // split cards are considered a single card, enqueue for later merging
        if (layout == "split" || layout == "aftermath" || layout == "adventure") {
            auto _faceName = getStringPropertyFromMap(card, "faceName");
            SplitCardPart split(_faceName, text, properties, setInfo);
            auto found_iter = splitCards.find(name);
            if (found_iter == splitCards.end()) {
                splitCards.insert(name, {split});
            } else if (layout == "adventure") {
                found_iter->insert(0, split);
            } else {
                found_iter->append(split);
            }
        } else {
            // relations
            relatedCards.clear();

            // add other face for split cards as card relation
            if (!getStringPropertyFromMap(card, "side").isEmpty()) {
                properties["cmc"] = getStringPropertyFromMap(card, "faceConvertedManaCost");
                if (layout == "meld") { // meld cards don't work
                    static const QRegularExpression meldNameRegex{"then meld them into ([^\\.]*)"};
                    QString additionalName = meldNameRegex.match(text).captured(1);
                    if (!additionalName.isNull()) {
                        relatedCards.append(new CardRelation(additionalName, CardRelation::TransformInto));
                    }
                } else {
                    for (const QString &additionalName : name.split(" // ")) {
                        if (additionalName != faceName) {
                            relatedCards.append(new CardRelation(additionalName, CardRelation::TransformInto));
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
                            new CardRelation(spbkName, CardRelation::DoesNotAttach, false, false, 1, true));
                    }
                }
            }

            CardInfoPtr newCard = addCard(name + numComponent, text, isToken, properties, relatedCards, setInfo);
            numCards++;
        }
    }

    // split cards handling
    static const QString splitCardPropSeparator = QString(" // ");
    static const QString splitCardTextSeparator = QString("\n\n---\n\n");
    for (const QString &nameSplit : splitCards.keys()) {
        // get all parts for this specific card
        QList<SplitCardPart> splitCardParts = splitCards.value(nameSplit);
        QSet<QString> done{};

        text.clear();
        properties.clear();
        relatedCards.clear();

        for (const SplitCardPart &tmp : splitCardParts) {
            // some sets have 2 different variations of the same split card,
            // eg. Fire // Ice in WC02. Avoid adding duplicates.
            QString splitName = tmp.getName();
            if (done.contains(splitName)) {
                continue;
            }
            done.insert(splitName);

            if (!text.isEmpty()) {
                text.append(splitCardTextSeparator);
            }
            text.append(tmp.getText());

            if (properties.isEmpty()) {
                properties = tmp.getProperties();
                setInfo = tmp.getSetInfo();
            } else {
                const QVariantHash &tmpProps = tmp.getProperties();
                for (const QString &prop : tmpProps.keys()) {
                    QString originalPropertyValue = properties.value(prop).toString();
                    QString thisCardPropertyValue = tmpProps.value(prop).toString();
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
        CardInfoPtr newCard = addCard(nameSplit, text, isToken, properties, relatedCards, setInfo);
        numCards++;
    }

    // only add the unique promo cards that didn't already exist in the set
    if (skipSpecialCards) {
        QList<QVariant> nonDuplicatePromos;
        for (auto cardIter = specialPromoCards.constBegin(); cardIter != specialPromoCards.constEnd(); ++cardIter) {
            if (!allNameProps.contains(cardIter.key())) {
                nonDuplicatePromos.append(cardIter.value());
            }
        }
        if (!nonDuplicatePromos.isEmpty()) {
            numCards += importCardsFromSet(currentSet, nonDuplicatePromos, false);
        }
    }
    return numCards;
}

void OracleImporter::sortAndReduceColors(QString &colors)
{
    // sort
    const QHash<QChar, unsigned int> colorOrder{{'W', 0}, {'U', 1}, {'B', 2}, {'R', 3}, {'G', 4}};
    std::sort(colors.begin(), colors.end(), [&colorOrder](const QChar a, const QChar b) {
        return colorOrder.value(a, INT_MAX) < colorOrder.value(b, INT_MAX);
    });
    // reduce
    QChar lastChar = '\0';
    for (int i = 0; i < colors.size(); ++i) {
        if (colors.at(i) == lastChar)
            colors.remove(i, 1);
        else
            lastChar = colors.at(i);
    }
}

int OracleImporter::startImport()
{
    int setCards = 0, setIndex = 0;
    // add an empty set for tokens
    CardSetPtr tokenSet = CardSet::newInstance(TOKENS_SETNAME, tr("Dummy set containing tokens"), "Tokens");
    sets.insert(TOKENS_SETNAME, tokenSet);

    for (const SetToDownload &curSetToParse : allSets) {
        CardSetPtr newSet = CardSet::newInstance(curSetToParse.getShortName(), curSetToParse.getLongName(),
                                                 curSetToParse.getSetType(), curSetToParse.getReleaseDate());
        if (!sets.contains(newSet->getShortName()))
            sets.insert(newSet->getShortName(), newSet);

        int numCardsInSet = importCardsFromSet(newSet, curSetToParse.getCards());

        ++setIndex;

        emit setIndexChanged(numCardsInSet, setIndex, curSetToParse.getLongName());
    }

    emit setIndexChanged(setCards, setIndex, QString());

    // total number of sets
    return setIndex;
}

bool OracleImporter::saveToFile(const QString &fileName, const QString &sourceUrl, const QString &sourceVersion)
{
    CockatriceXml4Parser parser;
    return parser.saveToFile(sets, cards, fileName, sourceUrl, sourceVersion);
}

void OracleImporter::clear()
{
    CardDatabase::clear();
    allSets.clear();
}
