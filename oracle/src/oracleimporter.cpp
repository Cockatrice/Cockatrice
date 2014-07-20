#include "oracleimporter.h"
#if QT_VERSION < 0x050000
    #include <QtGui>
#else
    #include <QtWidgets>
#endif
#include <QDebug>

#include "qt-json/json.h"

OracleImporter::OracleImporter(const QString &_dataDir, QObject *parent)
    : CardDatabase(parent), dataDir(_dataDir)
{
}

bool OracleImporter::readSetsFromByteArray(const QByteArray &data)
{
    QList<SetToDownload> newSetList;
    
    bool ok;
    setsMap = QtJson::Json::parse(QString(data), ok).toMap();
    if (!ok) {
        qDebug() << "error: QtJson::Json::parse()";
        return 0;
    }
    
    QListIterator<QVariant> it(setsMap.values());
    QVariantMap map;

    QString edition;
    QString editionLong;
    QVariant editionCards;
    bool import;

    while (it.hasNext()) {
        map = it.next().toMap();
        edition = map.value("code").toString();
        editionLong = map.value("name").toString();
        editionCards = map.value("cards");

        // core and expansion sets are marked to be imported by default
        import = (0 == QString::compare(map.value("type").toString(), QString("core"), Qt::CaseInsensitive) ||
            0 == QString::compare(map.value("type").toString(), QString("expansion"), Qt::CaseInsensitive));

        newSetList.append(SetToDownload(edition, editionLong, editionCards, import));
    }

    qSort(newSetList);

    if (newSetList.isEmpty())
        return false;
    allSets = newSetList;
    return true;
}

CardInfo *OracleImporter::addCard(const QString &setName,
                                  QString cardName,
                                  bool isToken,
                                  int cardId,
                                  QString &cardCost,
                                  const QString &cardType,
                                  const QString &cardPT,
                                  int cardLoyalty,
                                  const QStringList &cardText)
{
    QString fullCardText = cardText.join("\n");
    bool splitCard = false;
    if (cardName.contains('(')) {
        cardName.remove(QRegExp(" \\(.*\\)"));
        splitCard = true;
    }
    // Workaround for card name weirdness
    if (cardName.contains("XX"))
        cardName.remove("XX");
    cardName = cardName.replace("Æ", "AE");
    cardName = cardName.replace("’", "'");

    // Remove {} around mana costs
    cardCost.remove(QChar('{'));
    cardCost.remove(QChar('}'));

    CardInfo *card;
    if (cardHash.contains(cardName)) {
        card = cardHash.value(cardName);
        if (splitCard && !card->getText().contains(fullCardText))
            card->setText(card->getText() + "\n---\n" + fullCardText);
    } else {
        bool mArtifact = false;
        if (cardType.endsWith("Artifact"))
            for (int i = 0; i < cardText.size(); ++i)
                if (cardText[i].contains("{T}") && cardText[i].contains("to your mana pool"))
                    mArtifact = true;
                    
        QStringList colors;
        QStringList allColors = QStringList() << "W" << "U" << "B" << "R" << "G";
        for (int i = 0; i < allColors.size(); i++)
            if (cardCost.contains(allColors[i]))
                colors << allColors[i];
        
        if (cardText.contains(cardName + " is white."))
            colors << "W";
        if (cardText.contains(cardName + " is blue."))
            colors << "U";
        if (cardText.contains(cardName + " is black."))
            colors << "B";
        if (cardText.contains(cardName + " is red."))
            colors << "R";
        if (cardText.contains(cardName + " is green."))
            colors << "G";
        
        bool cipt = (cardText.contains(cardName + " enters the battlefield tapped."));
        
        card = new CardInfo(this, cardName, isToken, cardCost, cardType, cardPT, fullCardText, colors, cardLoyalty, cipt);
        int tableRow = 1;
        QString mainCardType = card->getMainCardType();
        if ((mainCardType == "Land") || mArtifact)
            tableRow = 0;
        else if ((mainCardType == "Sorcery") || (mainCardType == "Instant"))
            tableRow = 3;
        else if (mainCardType == "Creature")
            tableRow = 2;
        card->setTableRow(tableRow);
        
        cardHash.insert(cardName, card);
    }
    card->setMuId(setName, cardId);

    return card;
}

int OracleImporter::importTextSpoiler(CardSet *set, const QVariant &data)
{
    int cards = 0;
    
    QListIterator<QVariant> it(data.toList());
    QVariantMap map;
    QString cardName;
    QString cardCost;
    QString cardType;
    QString cardPT;
    QString cardText;
    int cardId;
    int cardLoyalty;
    bool cardIsToken = false;
    QMap<int, QVariantMap> splitCards;

    while (it.hasNext()) {
        map = it.next().toMap();
        if(0 == QString::compare(map.value("layout").toString(), QString("split"), Qt::CaseInsensitive))
        {
            // Split card handling
            cardId = map.contains("multiverseid") ? map.value("multiverseid").toInt() : 0;
            if(splitCards.contains(cardId))
            {
                // merge two split cards
                QVariantMap tmpMap = splitCards.take(cardId);
                QVariantMap * card1 = 0, * card2 = 0;
                // same cardid
                cardId = map.contains("multiverseid") ? map.value("multiverseid").toInt() : 0;
                // this is currently an integer; can't accept 2 values
                cardLoyalty = 0;

                // determine which subcard is the first one in the split
                QStringList names=map.contains("names") ? map.value("names").toStringList() : QStringList("");
                if(names.count()>0 &&
                    map.contains("name") &&
                    0 == QString::compare(map.value("name").toString(), names.at(0)))
                {
                    // map is the left part of the split card, tmpMap is right part
                    card1 = &map;
                    card2 = &tmpMap;
                } else {
                    //tmpMap is the left part of the split card, map is right part
                    card1 = &tmpMap;
                    card2 = &map;
                }

                // add first card's data
                cardName = card1->contains("name") ? card1->value("name").toString() : QString("");
                cardCost = card1->contains("manaCost") ? card1->value("manaCost").toString() : QString("");
                cardType = card1->contains("type") ? card1->value("type").toString() : QString("");
                cardPT = card1->contains("power") || card1->contains("toughness") ? card1->value("power").toString() + QString('/') + card1->value("toughness").toString() : QString("");
                cardText = card1->contains("text") ? card1->value("text").toString() : QString("");

                // add second card's data
                cardName += card2->contains("name") ? QString(" // ") + card2->value("name").toString() : QString("");
                cardCost += card2->contains("manaCost") ? QString(" // ") + card2->value("manaCost").toString() : QString("");
                cardType += card2->contains("type") ? QString(" // ") + card2->value("type").toString() : QString("");
                cardPT += card2->contains("power") || card2->contains("toughness") ? QString(" // ") + card2->value("power").toString() + QString('/') + card2->value("toughness").toString() : QString("");
                cardText += card2->contains("text") ? QString("\n\n---\n\n") + card2->value("text").toString() : QString("");
            } else {
                // first card od a pair; enqueue for later merging
                splitCards.insert(cardId, map);
                continue;
            }
        } else {
            // normal cards handling
            cardName = map.contains("name") ? map.value("name").toString() : QString("");
            cardCost = map.contains("manaCost") ? map.value("manaCost").toString() : QString("");
            cardType = map.contains("type") ? map.value("type").toString() : QString("");
            cardPT = map.contains("power") || map.contains("toughness") ? map.value("power").toString() + QString('/') + map.value("toughness").toString() : QString("");
            cardText = map.contains("text") ? map.value("text").toString() : QString("");
            cardId = map.contains("multiverseid") ? map.value("multiverseid").toInt() : 0;
            cardLoyalty = map.contains("loyalty") ? map.value("loyalty").toInt() : 0;
            cardIsToken = map.value("layout") == "token";

            // Distinguish Vanguard cards from regular cards of the same name.
            if (map.value("layout") == "vanguard") {
                cardName += " Avatar";
            }
        }

        CardInfo *card = addCard(set->getShortName(), cardName, cardIsToken, cardId, cardCost, cardType, cardPT, cardLoyalty, cardText.split("\n"));

        if (!set->contains(card)) {
            card->addToSet(set);
            cards++;
        }
    }
    
    return cards;
}

int OracleImporter::startImport()
{
    clear();

    int setCards = 0, setIndex= 0;
    QListIterator<SetToDownload> it(allSets);
    const SetToDownload * curSet;

    while (it.hasNext())
    {
        curSet = & it.next();
        if(!curSet->getImport())
            continue;
            
        CardSet *set = new CardSet(curSet->getShortName(), curSet->getLongName());
        if (!setHash.contains(set->getShortName()))
            setHash.insert(set->getShortName(), set);

        int setCards = importTextSpoiler(set, curSet->getCards());

        ++setIndex;
            
        emit setIndexChanged(setCards, setIndex, curSet->getLongName());
    }
    
    emit setIndexChanged(setCards, setIndex, QString());

    // total number of sets
    return setIndex;
}
