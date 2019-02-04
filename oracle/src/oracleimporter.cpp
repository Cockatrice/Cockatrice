#include "oracleimporter.h"
#include "carddbparser/cockatricexml4.h"

#include <QDebug>
#include <QtWidgets>
#include <climits>

#include "qt-json/json.h"

SplitCardPart::SplitCardPart(const int _index,
                             const QString &_text,
                             const QVariantHash &_properties,
                             const CardInfoPerSet _setInfo)
    : index(_index), text(_text), properties(_properties), setInfo(_setInfo)
{
}

OracleImporter::OracleImporter(const QString &_dataDir, QObject *parent) : CardDatabase(parent), dataDir(_dataDir)
{
}

bool OracleImporter::readSetsFromByteArray(const QByteArray &data)
{
    QList<SetToDownload> newSetList;

    bool ok;
    setsMap = QtJson::Json::parse(QString(data), ok).toMap();
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
        if (setType.length() > 0)
            setType[0] = setType[0].toUpper();
        releaseDate = map.value("releaseDate").toDate();
        newSetList.append(SetToDownload(shortName, longName, setCards, setType, releaseDate));
    }

    qSort(newSetList);

    if (newSetList.isEmpty())
        return false;
    allSets = newSetList;
    return true;
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
            if (symbol.contains(QRegExp("[0-9WUBGRP]/[0-9WUBGRP]"))) {
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

    // DETECT CARD POSITIONING INFO

    // cards that enter the field tapped
    bool cipt = text.contains("Hideaway") || (text.contains(name + " enters the battlefield tapped") &&
                                              !text.contains(name + " enters the battlefield tapped unless"));

    // detect mana generator artifacts
    QStringList cardTextRows = text.split("\n");
    bool mArtifact = false;
    QString cardType = properties.value("type").toString();
    if (cardType.endsWith("Artifact")) {
        for (int i = 0; i < cardTextRows.size(); ++i) {
            cardTextRows[i].remove(QRegularExpression(R"(\".*?\")"));
            if (cardTextRows[i].contains("{T}") && cardTextRows[i].contains("to your mana pool")) {
                mArtifact = true;
            }
        }
    }

    // table row
    int tableRow = 1;
    QString mainCardType = properties.value("maintype").toString();
    if ((mainCardType == "Land") || mArtifact)
        tableRow = 0;
    else if ((mainCardType == "Sorcery") || (mainCardType == "Instant"))
        tableRow = 3;
    else if (mainCardType == "Creature")
        tableRow = 2;

    // card side
    QString side = properties.value("side").toString() == "b" ? "back" : "front";
    properties.insert("side", side);

    // upsideDown (flip cards)
    bool upsideDown = false;
    QStringList additionalNames = properties.value("names").toStringList();
    QString layout = properties.value("layout").toString();
    if (layout == "flip") {
        if (properties.value("side").toString() != "front") {
            upsideDown = true;
        }
        // reset the side property, since the card has no back image
        properties.insert("side", "front");
    }

    // insert the card and its properties
    QList<CardRelation *> reverseRelatedCards;
    CardInfoPerSetMap setsInfo;
    setsInfo.insert(setInfo.getPtr()->getShortName(), setInfo);
    CardInfoPtr newCard = CardInfo::newInstance(name, text, isToken, properties, relatedCards, reverseRelatedCards,
                                                setsInfo, cipt, tableRow, upsideDown);

    cards.insert(name, newCard);
    return newCard;
}

QString OracleImporter::getStringPropertyFromMap(QVariantMap card, QString propertyName)
{
    return card.contains(propertyName) ? card.value(propertyName).toString() : QString("");
}

int OracleImporter::importCardsFromSet(CardSetPtr currentSet, const QList<QVariant> &cardsList)
{
    static const QMap<QString, QString> cardProperties{
        // mtgjson name => xml name
        {"manaCost", "manacost"}, {"convertedManaCost", "cmc"}, {"type", "type"},
        {"loyalty", "loyalty"},   {"layout", "layout"},         {"side", "side"},
    };

    static const QMap<QString, QString> setInfoProperties{// mtgjson name => xml name
                                                          {"multiverseId", "muid"},
                                                          {"scryfallId", "uuid"},
                                                          {"number", "num"},
                                                          {"rarity", "rarity"}};

    int numCards = 0;
    QMap<QString, SplitCardPart> splitCards;
    QString ptSeparator("/");
    QVariantMap card;
    QString layout, name, text, colors, maintype, power, toughness;
    bool isToken;
    QStringList additionalNames;
    QVariantHash properties;
    CardInfoPerSet setInfo;
    QList<CardRelation *> relatedCards;

    for (const QVariant &cardVar : cardsList) {
        card = cardVar.toMap();

        /* Currently used layouts are:
         * augment, double_faced_token, flip, host, leveler, meld, normal, planar,
         * saga, scheme, split, token, transform, vanguard
         */
        layout = getStringPropertyFromMap(card, "layout");

        // don't import tokens from the json file
        isToken = false;
        if (layout == "token")
            continue;

        // normal cards handling
        name = getStringPropertyFromMap(card, "name");
        text = getStringPropertyFromMap(card, "text");

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

        // special handling properties
        colors = card.value("colors").toStringList().join("");
        if (!colors.isEmpty())
            properties.insert("colors", colors);

        maintype = card.value("types").toStringList().first();
        if (!maintype.isEmpty())
            properties.insert("maintype", maintype);

        power = getStringPropertyFromMap(card, "power");
        toughness = getStringPropertyFromMap(card, "toughness");
        if (!(power.isEmpty() && toughness.isEmpty()))
            properties.insert("pt", power + ptSeparator + toughness);

        additionalNames = card.value("names").toStringList();
        // split cards are considered a single card, enqueue for later merging
        if (layout == "split") {
            // get the position of this card part
            int index = additionalNames.indexOf(name);
            // construct full card name
            name = additionalNames.join(QString(" // "));
            SplitCardPart split(index, text, properties, setInfo);
            splitCards.insertMulti(name, split);
        } else {
            // relations
            relatedCards.clear();
            if (additionalNames.size() > 1) {
                for (const QString &additionalName : additionalNames) {
                    if (additionalName != name)
                        relatedCards.append(new CardRelation(additionalName, true));
                }
            }

            CardInfoPtr newCard = addCard(name, text, isToken, properties, relatedCards, setInfo);
            numCards++;
        }
    }

    // split cards handling
    QString splitCardPropSeparator = QString(" // ");
    QString splitCardTextSeparator = QString("\n\n---\n\n");
    for (const QString &nameSplit : splitCards.uniqueKeys()) {
        // get all parts for this specific card
        QList<SplitCardPart> splitCardParts = splitCards.values(nameSplit);
        // sort them by index (aka position)
        qSort(splitCardParts.begin(), splitCardParts.end(),
              [](const SplitCardPart &a, const SplitCardPart &b) -> bool { return a.getIndex() < b.getIndex(); });

        text = QString("");
        isToken = false;
        properties.clear();
        relatedCards.clear();

        int lastIndex = -1;
        for (const SplitCardPart &tmp : splitCardParts) {
            // some sets have 2 different variations of the same split card,
            // eg. Fire // Ice in WC02. Avoid adding duplicates.
            if (lastIndex == tmp.getIndex())
                continue;
            lastIndex = tmp.getIndex();

            if (!text.isEmpty())
                text.append(splitCardTextSeparator);
            text.append(tmp.getText());

            if (properties.isEmpty()) {
                properties = tmp.getProperties();
                setInfo = tmp.getSetInfo();
            } else {
                const QVariantHash &props = tmp.getProperties();
                for (const QString &prop : props.keys()) {
                    QString originalPropertyValue = properties.value(prop).toString();
                    QString thisCardPropertyValue = props.value(prop).toString();
                    if (originalPropertyValue != thisCardPropertyValue) {
                        if (prop == "colors") {
                            properties.insert(prop, originalPropertyValue + thisCardPropertyValue);
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
    clear();

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

bool OracleImporter::saveToFile(const QString &fileName)
{
    CockatriceXml4Parser parser;
    return parser.saveToFile(sets, cards, fileName);
}