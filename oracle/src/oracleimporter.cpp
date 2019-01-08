#include "oracleimporter.h"
#include "carddbparser/cockatricexml3.h"

#include <QDebug>
#include <QtWidgets>
#include <climits>

#include "qt-json/json.h"

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

    QString edition;
    QString editionLong;
    QVariant editionCards;
    QString setType;
    QDate releaseDate;

    while (it.hasNext()) {
        map = it.next().toMap();
        edition = map.value("code").toString().toUpper();
        editionLong = map.value("name").toString();
        editionCards = map.value("cards");
        setType = map.value("type").toString();
        // capitalize set type
        if (setType.length() > 0)
            setType[0] = setType[0].toUpper();
        releaseDate = map.value("releaseDate").toDate();

        newSetList.append(SetToDownload(edition, editionLong, editionCards, setType, releaseDate));
    }

    qSort(newSetList);

    if (newSetList.isEmpty())
        return false;
    allSets = newSetList;
    return true;
}

CardInfoPtr OracleImporter::addCard(const QString &setName,
                                    QString cardName,
                                    bool isToken,
                                    int cardId,
                                    QString &cardUuId,
                                    QString &setNumber,
                                    QString &cardCost,
                                    QString &cmc,
                                    const QString &cardType,
                                    const QString &cardPT,
                                    const QString &cardLoyalty,
                                    const QString &cardText,
                                    const QStringList &colors,
                                    const QList<CardRelation *> &relatedCards,
                                    const QList<CardRelation *> &reverseRelatedCards,
                                    bool upsideDown,
                                    QString &rarity)
{
    QStringList cardTextRows = cardText.split("\n");

    // Workaround for card name weirdness
    cardName = cardName.replace("Æ", "AE");
    cardName = cardName.replace("’", "'");

    CardInfoPtr card;
    if (cards.contains(cardName)) {
        card = cards.value(cardName);
    } else {
        // Remove {} around mana costs, except if it's split cost
        QStringList symbols = cardCost.split("}");
        QString formattedCardCost = QString();
        for (QString symbol : symbols) {
            if (symbol.contains(QRegExp("[0-9WUBGRP]/[0-9WUBGRP]"))) {
                symbol.append("}");
            } else {
                symbol.remove(QChar('{'));
            }
            formattedCardCost.append(symbol);
        }

        // detect mana generator artifacts
        bool mArtifact = false;
        if (cardType.endsWith("Artifact")) {
            for (int i = 0; i < cardTextRows.size(); ++i) {
                cardTextRows[i].remove(QRegularExpression(R"(\".*?\")"));
                if (cardTextRows[i].contains("{T}") && cardTextRows[i].contains("to your mana pool")) {
                    mArtifact = true;
                }
            }
        }

        // detect cards that enter the field tapped
        bool cipt =
            cardText.contains("Hideaway") || (cardText.contains(cardName + " enters the battlefield tapped") &&
                                              !cardText.contains(cardName + " enters the battlefield tapped unless"));

        // insert the card and its properties
        card = CardInfo::newInstance(cardName, isToken, formattedCardCost, cmc, cardType, cardPT, cardText, colors,
                                     relatedCards, reverseRelatedCards, upsideDown, cardLoyalty, cipt);
        int tableRow = 1;
        QString mainCardType = card->getMainCardType();
        if ((mainCardType == "Land") || mArtifact)
            tableRow = 0;
        else if ((mainCardType == "Sorcery") || (mainCardType == "Instant"))
            tableRow = 3;
        else if (mainCardType == "Creature")
            tableRow = 2;
        card->setTableRow(tableRow);

        cards.insert(cardName, card);
    }

    card->setMuId(setName, cardId);
    card->setUuId(setName, cardUuId);
    card->setSetNumber(setName, setNumber);
    card->setRarity(setName, rarity);

    return card;
}

int OracleImporter::importTextSpoiler(CardSetPtr set, const QVariant &data)
{
    int cards = 0;

    QListIterator<QVariant> it(data.toList());
    QVariantMap map;
    QString cardName;
    QString cardCost;
    QString cmc;
    QString cardType;
    QString cardPT;
    QString cardText;
    QStringList colors;
    QList<CardRelation *> relatedCards;
    QList<CardRelation *> reverseRelatedCards; // dummy
    int cardId;
    QString cardUuId;
    QString setNumber;
    QString rarity;
    QString cardLoyalty;
    bool upsideDown;
    QMap<int, QVariantMap> splitCards;

    while (it.hasNext()) {
        map = it.next().toMap();

        /* Currently used layouts are:
         * augment, double_faced_token, flip, host, leveler, meld, normal, planar,
         * saga, scheme, split, token, transform, vanguard
         */
        QString layout = map.value("layout").toString();

        // don't import tokens from the json file
        if (layout == "token")
            continue;

        // Aftermath card layout seems to have been integrated in "split"
        if (layout == "split") {
            // Enqueue split card for later handling
            cardId = map.contains("multiverseId") ? map.value("multiverseId").toInt() : 0;
            if (cardId)
                splitCards.insertMulti(cardId, map);
            continue;
        }

        // normal cards handling
        cardName = map.contains("name") ? map.value("name").toString() : QString("");
        cardCost = map.contains("manaCost") ? map.value("manaCost").toString() : QString("");
        cmc = map.contains("convertedManaCost") ? map.value("convertedManaCost").toString() : QString("0");
        cardType = map.contains("type") ? map.value("type").toString() : QString("");
        cardPT = map.contains("power") || map.contains("toughness")
                     ? map.value("power").toString() + QString('/') + map.value("toughness").toString()
                     : QString("");
        cardText = map.contains("text") ? map.value("text").toString() : QString("");
        cardId = map.contains("multiverseId") ? map.value("multiverseId").toInt() : 0;
        cardUuId = map.contains("scryfallId") ? map.value("scryfallId").toString() : QString("");
        setNumber = map.contains("number") ? map.value("number").toString() : QString("");
        rarity = map.contains("rarity") ? map.value("rarity").toString() : QString("");
        cardLoyalty = map.contains("loyalty") ? map.value("loyalty").toString() : QString("");
        colors = map.contains("colors") ? map.value("colors").toStringList() : QStringList();
        relatedCards = QList<CardRelation *>();
        if (map.contains("names"))
            for (const QString &name : map.value("names").toStringList()) {
                if (name != cardName)
                    relatedCards.append(new CardRelation(name, true));
            }

        if (0 == QString::compare(map.value("layout").toString(), QString("flip"), Qt::CaseInsensitive)) {
            QStringList cardNames = map.contains("names") ? map.value("names").toStringList() : QStringList();
            upsideDown = (cardNames.indexOf(cardName) > 0);
        } else {
            upsideDown = false;
        }

        CardInfoPtr card =
            addCard(set->getShortName(), cardName, false, cardId, cardUuId, setNumber, cardCost, cmc, cardType, cardPT,
                    cardLoyalty, cardText, colors, relatedCards, reverseRelatedCards, upsideDown, rarity);

        if (!set->contains(card)) {
            card->addToSet(set);
            cards++;
        }
    }

    // split cards handling - get all unique card muids
    QList<int> muids = splitCards.uniqueKeys();
    for (int muid : muids) {
        // get all cards for this specific muid
        QList<QVariantMap> maps = splitCards.values(muid);
        QStringList names;
        // now, reorder the cards using the ordered list of names
        QMap<int, QVariantMap> orderedMaps;
        for (const QVariantMap &inner_map : maps) {
            if (names.isEmpty())
                names = inner_map.contains("names") ? inner_map.value("names").toStringList() : QStringList();
            QString name = inner_map.value("name").toString();
            int index = names.indexOf(name);
            orderedMaps.insertMulti(index, inner_map);
        }

        // clean variables
        cardName = "";
        cardCost = "";
        cmc = "";
        cardType = "";
        cardPT = "";
        cardText = "";
        cardUuId = "";
        setNumber = "";
        rarity = "";
        cardLoyalty = "";
        colors.clear();

        // loop cards and merge their contents
        QString prefix = QString(" // ");
        QString prefix2 = QString("\n\n---\n\n");
        for (const QVariantMap &inner_map : orderedMaps.values()) {
            if (inner_map.contains("name")) {
                if (!cardName.isEmpty())
                    cardName += (orderedMaps.count() > 2) ? QString("/") : prefix;
                cardName += inner_map.value("name").toString();
            }
            if (inner_map.contains("manaCost")) {
                if (!cardCost.isEmpty())
                    cardCost += prefix;
                cardCost += inner_map.value("manaCost").toString();
            }
            if (inner_map.contains("convertedManaCost")) {
                if (!cmc.isEmpty())
                    cmc += prefix;
                cmc += inner_map.value("convertedManaCost").toString();
            }
            if (inner_map.contains("type")) {
                if (!cardType.isEmpty())
                    cardType += prefix;
                cardType += inner_map.value("type").toString();
            }
            if (inner_map.contains("power") || inner_map.contains("toughness")) {
                if (!cardPT.isEmpty())
                    cardPT += prefix;
                cardPT += inner_map.value("power").toString() + QString('/') + inner_map.value("toughness").toString();
            }
            if (inner_map.contains("text")) {
                if (!cardText.isEmpty())
                    cardText += prefix2;
                cardText += inner_map.value("text").toString();
            }
            if (inner_map.contains("uuid")) {
                if (cardUuId.isEmpty())
                    cardUuId = inner_map.value("uuid").toString();
            }
            if (inner_map.contains("number")) {
                if (setNumber.isEmpty())
                    setNumber = inner_map.value("number").toString();
            }
            if (inner_map.contains("rarity")) {
                if (rarity.isEmpty())
                    rarity = inner_map.value("rarity").toString();
            }

            colors << inner_map.value("colors").toStringList();
        }

        colors.removeDuplicates();
        if (colors.length() > 1) {
            sortColors(colors);
        }

        // Fortunately, there are no split cards that flip, transform or meld.
        relatedCards = QList<CardRelation *>();
        reverseRelatedCards = QList<CardRelation *>();
        upsideDown = false;

        // add the card
        CardInfoPtr card =
            addCard(set->getShortName(), cardName, false, muid, cardUuId, setNumber, cardCost, cmc, cardType, cardPT,
                    cardLoyalty, cardText, colors, relatedCards, reverseRelatedCards, upsideDown, rarity);

        if (!set->contains(card)) {
            card->addToSet(set);
            cards++;
        }
    }

    return cards;
}

void OracleImporter::sortColors(QStringList &colors)
{
    const QHash<QString, unsigned int> colorOrder{{"W", 0}, {"U", 1}, {"B", 2}, {"R", 3}, {"G", 4}};
    std::sort(colors.begin(), colors.end(), [&colorOrder](const QString a, const QString b) {
        return colorOrder.value(a, INT_MAX) < colorOrder.value(b, INT_MAX);
    });
}

int OracleImporter::startImport()
{
    clear();

    int setCards = 0, setIndex = 0;
    QListIterator<SetToDownload> it(allSets);
    const SetToDownload *curSet;

    // add an empty set for tokens
    CardSetPtr tokenSet = CardSet::newInstance(TOKENS_SETNAME, tr("Dummy set containing tokens"), "Tokens");
    sets.insert(TOKENS_SETNAME, tokenSet);

    while (it.hasNext()) {
        curSet = &it.next();
        CardSetPtr set = CardSet::newInstance(curSet->getShortName(), curSet->getLongName(), curSet->getSetType(),
                                              curSet->getReleaseDate());
        if (!sets.contains(set->getShortName()))
            sets.insert(set->getShortName(), set);

        int setCardsHere = importTextSpoiler(set, curSet->getCards());

        ++setIndex;

        emit setIndexChanged(setCardsHere, setIndex, curSet->getLongName());
    }

    emit setIndexChanged(setCards, setIndex, QString());

    // total number of sets
    return setIndex;
}

bool OracleImporter::saveToFile(const QString &fileName)
{
    CockatriceXml3Parser parser;
    return parser.saveToFile(sets, cards, fileName);
}