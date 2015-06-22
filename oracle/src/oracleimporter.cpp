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
    QString setType;
    QDate releaseDate;

    while (it.hasNext()) {
        map = it.next().toMap();
        edition = map.value("code").toString();
        editionLong = map.value("name").toString();
        editionCards = map.value("cards");
        setType = map.value("type").toString();
        // capitalize set type
        if(setType.length() > 0)
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

CardInfo *OracleImporter::addCard(const QString &setName,
                                  QString cardName,
                                  bool isToken,
                                  int cardId,
                                  QString &cardCost,
                                  QString &cmc,
                                  const QString &cardType,
                                  const QString &cardPT,
                                  int cardLoyalty,
                                  const QString &cardText,
                                  const QStringList & colors,
                                  const QStringList & relatedCards,
                                  bool upsideDown
                                  )
{
    QStringList cardTextRows = cardText.split("\n");
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
    if (cards.contains(cardName)) {
        card = cards.value(cardName);
        if (splitCard && !card->getText().contains(cardText))
            card->setText(card->getText() + "\n---\n" + cardText);
    } else {
        bool mArtifact = false;
        if (cardType.endsWith("Artifact"))
            for (int i = 0; i < cardTextRows.size(); ++i)
                if (cardTextRows[i].contains("{T}") && cardTextRows[i].contains("to your mana pool"))
                    mArtifact = true;
                    
        bool cipt = cardText.contains("Hideaway") || (cardText.contains(cardName + " enters the battlefield tapped") && !cardText.contains(cardName + " enters the battlefield tapped unless"));
        
        card = new CardInfo(this, cardName, isToken, cardCost, cmc, cardType, cardPT, cardText, colors, relatedCards, upsideDown, cardLoyalty, cipt);
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

    return card;
}

void OracleImporter::extractColors(const QStringList & in, QStringList & out)
{
    foreach(QString c, in)
    {
        if (c == "White")
            out << "W";
        else if (c == "Blue")
            out << "U";
        else if (c == "Black")
            out << "B";
        else if (c == "Red")
            out << "R";
        else if (c == "Green")
            out << "G";
        else
            qDebug() << "error: unknown color:" << c;
    }
}

int OracleImporter::importTextSpoiler(CardSet *set, const QVariant &data)
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
    QStringList relatedCards;
    int cardId;
    int cardLoyalty;
    bool cardIsToken = false;
    bool upsideDown = false;
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
                QStringList names=map.contains("names") ? map.value("names").toStringList() : QStringList();
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
                cmc = card1->contains("cmc") ? card1->value("cmc").toString() : QString("0");
                cardType = card1->contains("type") ? card1->value("type").toString() : QString("");
                cardPT = card1->contains("power") || card1->contains("toughness") ? card1->value("power").toString() + QString('/') + card1->value("toughness").toString() : QString("");
                cardText = card1->contains("text") ? card1->value("text").toString() : QString("");

                // add second card's data
                cardName += card2->contains("name") ? QString(" // ") + card2->value("name").toString() : QString("");
                cardCost += card2->contains("manaCost") ? QString(" // ") + card2->value("manaCost").toString() : QString("");
                cmc += card2->contains("cmc") ? QString(" // ") + card2->value("cmc").toString() : QString("0");
                cardType += card2->contains("type") ? QString(" // ") + card2->value("type").toString() : QString("");
                cardPT += card2->contains("power") || card2->contains("toughness") ? QString(" // ") + card2->value("power").toString() + QString('/') + card2->value("toughness").toString() : QString("");
                cardText += card2->contains("text") ? QString("\n\n---\n\n") + card2->value("text").toString() : QString("");

                colors.clear();
                extractColors(card1->value("colors").toStringList(), colors);
                extractColors(card2->value("colors").toStringList(), colors);
                colors.removeDuplicates();

                relatedCards = QStringList();
                upsideDown = false;
            } else {
                // first card of a pair; enqueue for later merging
                // Conditional on cardId because promo prints have no muid - see #640
                if (cardId)
                  splitCards.insert(cardId, map);
                continue;
            }
        } else {
            // normal cards handling
            cardName = map.contains("name") ? map.value("name").toString() : QString("");
            cardCost = map.contains("manaCost") ? map.value("manaCost").toString() : QString("");
            cmc = map.contains("cmc") ? map.value("cmc").toString() : QString("0");
            cardType = map.contains("type") ? map.value("type").toString() : QString("");
            cardPT = map.contains("power") || map.contains("toughness") ? map.value("power").toString() + QString('/') + map.value("toughness").toString() : QString("");
            cardText = map.contains("text") ? map.value("text").toString() : QString("");
            cardId = map.contains("multiverseid") ? map.value("multiverseid").toInt() : 0;
            cardLoyalty = map.contains("loyalty") ? map.value("loyalty").toInt() : 0;
            cardIsToken = map.value("layout") == "token";
            relatedCards = map.contains("names") ? map.value("names").toStringList() : QStringList();
            relatedCards.removeAll(cardName);

            if(0 == QString::compare(map.value("layout").toString(), QString("flip"), Qt::CaseInsensitive)) 
            {
                QStringList cardNames = map.contains("names") ? map.value("names").toStringList() : QStringList();
                upsideDown = (cardNames.indexOf(cardName) > 0);
            } else {
                upsideDown = false;
            }

            colors.clear();
            extractColors(map.value("colors").toStringList(), colors);

            // Distinguish Vanguard cards from regular cards of the same name.
            if (map.value("layout") == "vanguard") {
                cardName += " Avatar";
            }
        }

        if (!cardIsToken) {
            CardInfo *card = addCard(set->getShortName(), cardName, cardIsToken, cardId, cardCost, cmc, cardType, cardPT, cardLoyalty, cardText, colors, relatedCards, upsideDown);

            if (!set->contains(card)) {
                card->addToSet(set);
                cards++;
            }
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

    // add an empty set for tokens
    CardSet *tokenSet = new CardSet(TOKENS_SETNAME, tr("Dummy set containing tokens"), "Tokens");
    sets.insert(TOKENS_SETNAME, tokenSet);

    while (it.hasNext())
    {
        curSet = & it.next();            
        CardSet *set = new CardSet(curSet->getShortName(), curSet->getLongName(), curSet->getSetType(), curSet->getReleaseDate());
        if (!sets.contains(set->getShortName()))
            sets.insert(set->getShortName(), set);

        int setCards = importTextSpoiler(set, curSet->getCards());

        ++setIndex;
            
        emit setIndexChanged(setCards, setIndex, curSet->getLongName());
    }
    
    emit setIndexChanged(setCards, setIndex, QString());

    // total number of sets
    return setIndex;
}
