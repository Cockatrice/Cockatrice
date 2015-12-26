#include "carddatabase.h"
#include "pictureloader.h"
#include "settingscache.h"
#include "thememanager.h"

#include <QCryptographicHash>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>

const int CardDatabase::versionNeeded = 3;
const char* CardDatabase::TOKENS_SETNAME = "TK";

static QXmlStreamWriter &operator<<(QXmlStreamWriter &xml, const CardSet *set)
{
    xml.writeStartElement("set");
    xml.writeTextElement("name", set->getShortName());
    xml.writeTextElement("longname", set->getLongName());
    xml.writeTextElement("settype", set->getSetType());
    xml.writeTextElement("releasedate", set->getReleaseDate().toString(Qt::ISODate));
    xml.writeEndElement();

    return xml;
}

CardSet::CardSet(const QString &_shortName, const QString &_longName, const QString &_setType, const QDate &_releaseDate)
    : shortName(_shortName), longName(_longName), releaseDate(_releaseDate), setType(_setType)
{
    loadSetOptions();
}

QString CardSet::getCorrectedShortName() const
{
    // Because windows is horrible.
    QSet<QString> invalidFileNames;
    invalidFileNames << "CON" << "PRN" << "AUX" << "NUL" << "COM1" << "COM2" <<
        "COM3" << "COM4" << "COM5" << "COM6" << "COM7" << "COM8" << "COM9" <<
        "LPT1" << "LPT2" << "LPT3" << "LPT4" << "LPT5" << "LPT6" << "LPT7" <<
        "LPT8" << "LPT9";

    return invalidFileNames.contains(shortName) ? shortName + "_" : shortName;
}

void CardSet::loadSetOptions()
{
    sortKey = settingsCache->cardDatabase().getSortKey(shortName);
    enabled = settingsCache->cardDatabase().isEnabled(shortName);
    isknown = settingsCache->cardDatabase().isKnown(shortName);
}

void CardSet::setSortKey(unsigned int _sortKey)
{
    sortKey = _sortKey;
    settingsCache->cardDatabase().setSortKey(shortName,_sortKey);
}

void CardSet::setEnabled(bool _enabled)
{
    enabled = _enabled;
    settingsCache->cardDatabase().setEnabled(shortName,_enabled);
}

void CardSet::setIsKnown(bool _isknown)
{
    isknown = _isknown;
    settingsCache->cardDatabase().setIsKnown(shortName,_isknown);
}

class SetList::KeyCompareFunctor {
public:
    inline bool operator()(CardSet *a, CardSet *b) const
    {
        return a->getSortKey() < b->getSortKey();
    }
};

void SetList::sortByKey()
{
    qSort(begin(), end(), KeyCompareFunctor());
}

int SetList::getEnabledSetsNum()
{
    int num=0;
    for (int i = 0; i < size(); ++i)
    {
        CardSet *set = at(i);
        if(set->getEnabled())
            ++num;
    }
    return num;
}

int SetList::getUnknownSetsNum()
{
    int num=0;
    for (int i = 0; i < size(); ++i)
    {
        CardSet *set = at(i);
        if(!set->getIsKnown())
            ++num;
    }
    return num;
}

void SetList::enableAllUnknown()
{
    for (int i = 0; i < size(); ++i)
    {
        CardSet *set = at(i);
        if(!set->getIsKnown())
        {
            set->setIsKnown(true);
            set->setEnabled(true);
        }
    }
}

void SetList::enableAll()
{
    for (int i = 0; i < size(); ++i)
    {
        CardSet *set = at(i);
        set->setIsKnown(true);
        set->setEnabled(true);
    }
}

void SetList::markAllAsKnown()
{
    for (int i = 0; i < size(); ++i)
    {
        CardSet *set = at(i);
        if(!set->getIsKnown())
        {
            set->setIsKnown(true);
        }
    }
}

void SetList::guessSortKeys()
{
    // sort by release date DESC; invalid dates to the bottom.
    QDate distantFuture(2050, 1, 1);
    int aHundredYears = 36500;
    for (int i = 0; i < size(); ++i)
    {
        CardSet *set = at(i);
        QDate date = set->getReleaseDate();
        if(date.isNull())
            set->setSortKey(aHundredYears);
        else
            set->setSortKey(date.daysTo(distantFuture));
    }
}

CardInfo::CardInfo(CardDatabase *_db,
                   const QString &_name,
                   bool _isToken,
                   const QString &_manacost,
                   const QString &_cmc,
                   const QString &_cardtype,
                   const QString &_powtough,
                   const QString &_text,
                   const QStringList &_colors,
                   const QStringList &_relatedCards,
                   const QStringList &_reverseRelatedCards,
                   bool _upsideDownArt,
                   int _loyalty,
                   bool _cipt,
                   int _tableRow,
                   const SetList &_sets,
                   const QStringMap &_customPicURLs,
                   MuidMap _muIds
                   )
    : db(_db),
      name(_name),
      isToken(_isToken),
      sets(_sets),
      manacost(_manacost),
      cmc(_cmc),
      cardtype(_cardtype),
      powtough(_powtough),
      text(_text),
      colors(_colors),
      relatedCards(_relatedCards),
      reverseRelatedCards(_reverseRelatedCards),
      upsideDownArt(_upsideDownArt),
      loyalty(_loyalty),
      customPicURLs(_customPicURLs),
      muIds(_muIds),
      cipt(_cipt),
      tableRow(_tableRow)
{
    pixmapCacheKey = QLatin1String("card_") + name;
    simpleName = CardInfo::simplifyName(name);

    for (int i = 0; i < sets.size(); i++)
        sets[i]->append(this);
}

CardInfo::~CardInfo()
{
    PictureLoader::clearPixmapCache(this);
}

QString CardInfo::getMainCardType() const
{
    QString result = getCardType();
    /*
    Legendary Artifact Creature - Golem
    Instant // Instant
    */

    int pos;
    if ((pos = result.indexOf('-')) != -1)
        result.remove(pos, result.length());
    if ((pos = result.indexOf("—")) != -1)
        result.remove(pos, result.length());
    if ((pos = result.indexOf("//")) != -1)
        result.remove(pos, result.length());
    result = result.simplified();
    /*
    Legendary Artifact Creature
    Instant
    */

    if ((pos = result.lastIndexOf(' ')) != -1)
        result = result.mid(pos + 1);
    /*
    Creature
    Instant
    */

    return result;
}

QString CardInfo::getCorrectedName() const
{
    QString result = name;
    // Fire // Ice, Circle of Protection: Red, "Ach! Hans, Run!", Who/What/When/Where/Why, Question Elemental?
    return result.remove(" // ").remove(':').remove('"').remove('?').replace('/', ' ');
}

void CardInfo::addToSet(CardSet *set)
{
    set->append(this);
    sets << set;
}

QString CardInfo::simplifyName(const QString &name) {
    QString simpleName(name);

    // So Aetherling would work, but not Ætherling since 'Æ' would get replaced
    // with nothing.
    simpleName.replace("æ", "ae");
    simpleName.replace("Æ", "AE");

    // Replace Jötun Grunt with Jotun Grunt.
    simpleName = simpleName.normalized(QString::NormalizationForm_KD);

    // Replace dashes with spaces so that we can say "garruk the veil cursed"
    // instead of the unintuitive "garruk the veilcursed".
    simpleName = simpleName.replace("-", " ");

    simpleName.remove(QRegExp("[^a-zA-Z0-9 ]"));
    simpleName = simpleName.toLower();
    return simpleName;
}

static QXmlStreamWriter &operator<<(QXmlStreamWriter &xml, const CardInfo *info)
{
    xml.writeStartElement("card");
    xml.writeTextElement("name", info->getName());

    const SetList &sets = info->getSets();
    QString tmpString;
    QString tmpSet;
    for (int i = 0; i < sets.size(); i++) {
        xml.writeStartElement("set");

        tmpSet=sets[i]->getShortName();
        xml.writeAttribute("muId", QString::number(info->getMuId(tmpSet)));

        tmpString = info->getCustomPicURL(tmpSet);
        if(!tmpString.isEmpty())
            xml.writeAttribute("picURL", tmpString);

        xml.writeCharacters(tmpSet);
        xml.writeEndElement();
    }
    const QStringList &colors = info->getColors();
    for (int i = 0; i < colors.size(); i++)
        xml.writeTextElement("color", colors[i]);

    const QStringList &related = info->getRelatedCards();
    for (int i = 0; i < related.size(); i++)
        xml.writeTextElement("related", related[i]);

    const QStringList &reverseRelated = info->getReverseRelatedCards();
    for (int i = 0; i < reverseRelated.size(); i++)
        xml.writeTextElement("reverse-related", reverseRelated[i]);

    xml.writeTextElement("manacost", info->getManaCost());
    xml.writeTextElement("cmc", info->getCmc());
    xml.writeTextElement("type", info->getCardType());
    if (!info->getPowTough().isEmpty())
        xml.writeTextElement("pt", info->getPowTough());
    xml.writeTextElement("tablerow", QString::number(info->getTableRow()));
    xml.writeTextElement("text", info->getText());
    if (info->getMainCardType() == "Planeswalker")
        xml.writeTextElement("loyalty", QString::number(info->getLoyalty()));
    if (info->getCipt())
        xml.writeTextElement("cipt", "1");
    if (info->getIsToken())
        xml.writeTextElement("token", "1");
    if (info->getUpsideDownArt())
        xml.writeTextElement("upsidedown", "1");
    xml.writeEndElement(); // card

    return xml;
}

CardDatabase::CardDatabase(QObject *parent)
    : QObject(parent), noCard(0), loadStatus(NotLoaded)
{
    connect(settingsCache, SIGNAL(cardDatabasePathChanged()), this, SLOT(loadCardDatabase()));
    connect(settingsCache, SIGNAL(tokenDatabasePathChanged()), this, SLOT(loadTokenDatabase()));

    loadCardDatabase();
    loadTokenDatabase();

    noCard = new CardInfo(this);
}

CardDatabase::~CardDatabase()
{
    clear();
    delete noCard;
}

void CardDatabase::clear()
{
    QHashIterator<QString, CardSet *> setIt(sets);
    while (setIt.hasNext()) {
        setIt.next();
        delete setIt.value();
    }
    sets.clear();
    
    QHashIterator<QString, CardInfo *> i(cards);
    while (i.hasNext()) {
        i.next();
        delete i.value();
    }
    cards.clear();

    // The pointers themselves were already deleted, so we don't delete them
    // again.
    simpleNameCards.clear();
}

void CardDatabase::addCard(CardInfo *card)
{
    cards.insert(card->getName(), card);
    simpleNameCards.insert(card->getSimpleName(), card);
    emit cardAdded(card);
}

void CardDatabase::removeCard(CardInfo *card)
{
    cards.remove(card->getName());
    simpleNameCards.remove(card->getSimpleName());
    emit cardRemoved(card);
}

CardInfo *CardDatabase::getCard(const QString &cardName, bool createIfNotFound) {
    return getCardFromMap(cards, cardName, createIfNotFound);
}

QList<CardInfo *> CardDatabase::getCards(const QStringList &cardNames)
{
    QList<CardInfo *> cardInfos;
    foreach(QString cardName, cardNames)
        cardInfos.append(getCardFromMap(cards, cardName, false));

    return cardInfos;
}

CardInfo *CardDatabase::getCardBySimpleName(const QString &cardName, bool createIfNotFound) {
    QString simpleName = CardInfo::simplifyName(cardName);
    return getCardFromMap(simpleNameCards, simpleName, createIfNotFound);
}

CardSet *CardDatabase::getSet(const QString &setName)
{
    if (sets.contains(setName))
        return sets.value(setName);
    else {
        CardSet *newSet = new CardSet(setName);
        sets.insert(setName, newSet);
        return newSet;
    }
}

SetList CardDatabase::getSetList() const
{
    SetList result;
    QHashIterator<QString, CardSet *> i(sets);
    while (i.hasNext()) {
        i.next();
        result << i.value();
    }
    return result;
}

void CardDatabase::loadSetsFromXml(QXmlStreamReader &xml)
{
    while (!xml.atEnd()) {
        if (xml.readNext() == QXmlStreamReader::EndElement)
            break;
        if (xml.name() == "set") {
            QString shortName, longName, setType;
            QDate releaseDate;
            while (!xml.atEnd()) {
                if (xml.readNext() == QXmlStreamReader::EndElement)
                    break;
                if (xml.name() == "name")
                    shortName = xml.readElementText();
                else if (xml.name() == "longname")
                    longName = xml.readElementText();
                else if (xml.name() == "settype")
                    setType = xml.readElementText();
                else if (xml.name() == "releasedate")
                    releaseDate = QDate::fromString(xml.readElementText(), Qt::ISODate);
            }

            CardSet * newSet = getSet(shortName);
            newSet->setLongName(longName);
            newSet->setSetType(setType);
            newSet->setReleaseDate(releaseDate);
        }
    }
}

void CardDatabase::loadCardsFromXml(QXmlStreamReader &xml, bool tokens)
{
    while (!xml.atEnd()) {
        if (xml.readNext() == QXmlStreamReader::EndElement)
            break;
        if (xml.name() == "card") {
            QString name, manacost, cmc, type, pt, text;
            QStringList colors, relatedCards, reverseRelatedCards;
            QStringMap customPicURLs;
            MuidMap muids;
            SetList sets;
            int tableRow = 0;
            int loyalty = 0;
            bool cipt = false;
            bool isToken = false;
            bool upsideDown = false;
            while (!xml.atEnd()) {
                if (xml.readNext() == QXmlStreamReader::EndElement)
                    break;
                if (xml.name() == "name")
                    name = xml.readElementText();
                else if (xml.name() == "manacost")
                    manacost = xml.readElementText();
                else if (xml.name() == "cmc")
                    cmc = xml.readElementText();
                else if (xml.name() == "type")
                    type = xml.readElementText();
                else if (xml.name() == "pt")
                    pt = xml.readElementText();
                else if (xml.name() == "text")
                    text = xml.readElementText();
                else if (xml.name() == "set") {
                    QXmlStreamAttributes attrs = xml.attributes();
                    QString setName = xml.readElementText();
                    sets.append(getSet(setName));
                    if (attrs.hasAttribute("muId")) {
                        muids[setName] = attrs.value("muId").toString().toInt();
                    }
                    if (attrs.hasAttribute("picURL")) {
                        customPicURLs[setName] = attrs.value("picURL").toString();
                    }
                } else if (xml.name() == "color")
                    colors << xml.readElementText();
                else if (xml.name() == "related")
                    relatedCards << xml.readElementText();
                else if (xml.name() == "reverse-related")
                    reverseRelatedCards << xml.readElementText();
                else if (xml.name() == "tablerow")
                    tableRow = xml.readElementText().toInt();
                else if (xml.name() == "cipt")
                    cipt = (xml.readElementText() == "1");
                else if (xml.name() == "upsidedown")
                    upsideDown = (xml.readElementText() == "1");
                else if (xml.name() == "loyalty")
                    loyalty = xml.readElementText().toInt();
                else if (xml.name() == "token")
                    isToken = xml.readElementText().toInt();
            }

            if (isToken == tokens) {
                addCard(new CardInfo(this, name, isToken, manacost, cmc, type, pt, text, colors, relatedCards, reverseRelatedCards, upsideDown, loyalty, cipt, tableRow, sets, customPicURLs, muids));
            }
        }
    }
}

CardInfo *CardDatabase::getCardFromMap(CardNameMap &cardMap, const QString &cardName, bool createIfNotFound) {
    if (cardMap.contains(cardName))
        return cardMap.value(cardName);
    
    if (createIfNotFound) {
        CardInfo *newCard = new CardInfo(this, cardName, true);
        newCard->addToSet(getSet(CardDatabase::TOKENS_SETNAME));
        cardMap.insert(cardName, newCard);
        return newCard;
    }

    return noCard;
}

LoadStatus CardDatabase::loadFromFile(const QString &fileName, bool tokens)
{
    QFile file(fileName);
    file.open(QIODevice::ReadOnly);
    if (!file.isOpen())
        return FileError;

    QXmlStreamReader xml(&file);
    while (!xml.atEnd()) {
        if (xml.readNext() == QXmlStreamReader::StartElement) {
            if (xml.name() != "cockatrice_carddatabase")
                return Invalid;
            int version = xml.attributes().value("version").toString().toInt();
            if (version < versionNeeded) {
                qDebug() << "loadFromFile(): Version too old: " << version;
                return VersionTooOld;
            }
            while (!xml.atEnd()) {
                if (xml.readNext() == QXmlStreamReader::EndElement)
                    break;
                if (xml.name() == "sets")
                    loadSetsFromXml(xml);
                else if (xml.name() == "cards")
                    loadCardsFromXml(xml, tokens);
            }
        }
    }
    qDebug() << cards.size() << "cards in" << sets.size() << "sets loaded";

    if (cards.isEmpty()) return NoCards;

    return Ok;
}

bool CardDatabase::saveToFile(const QString &fileName, bool tokens)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly))
        return false;
    QXmlStreamWriter xml(&file);

    xml.setAutoFormatting(true);
    xml.writeStartDocument();
    xml.writeStartElement("cockatrice_carddatabase");
    xml.writeAttribute("version", QString::number(versionNeeded));

    if (!tokens) {
        xml.writeStartElement("sets");
        QHashIterator<QString, CardSet *> setIterator(sets);
        while (setIterator.hasNext())
            xml << setIterator.next().value();
        xml.writeEndElement(); // sets
    }

    xml.writeStartElement("cards");
    QHashIterator<QString, CardInfo *> cardIterator(cards);
    while (cardIterator.hasNext()) {
        CardInfo *card = cardIterator.next().value();
        if (tokens == card->getIsToken()) {
            xml << card;
        }
    }
    xml.writeEndElement(); // cards

    xml.writeEndElement(); // cockatrice_carddatabase
    xml.writeEndDocument();

    return true;
}

void CardDatabase::emitCardListChanged()
{
    emit cardListChanged();
}

LoadStatus CardDatabase::loadCardDatabase(const QString &path, bool tokens)
{
    LoadStatus tempLoadStatus = NotLoaded;
    if (!path.isEmpty())
        tempLoadStatus = loadFromFile(path, tokens);

    if (tempLoadStatus == Ok) {
        SetList allSets;
        QHashIterator<QString, CardSet *> setsIterator(sets);
        while (setsIterator.hasNext())
            allSets.append(setsIterator.next().value());
        allSets.sortByKey();

        if(!tokens)
            checkUnknownSets();
        emit cardListChanged();
    }

    if (!tokens) {
        loadStatus = tempLoadStatus;
        qDebug() << "loadCardDatabase(): Path = " << path << " Status = " << loadStatus;
    }


    return tempLoadStatus;
}

void CardDatabase::loadCardDatabase()
{
    loadCardDatabase(settingsCache->getCardDatabasePath(), false);
}

void CardDatabase::loadTokenDatabase()
{
    loadCardDatabase(settingsCache->getTokenDatabasePath(), true);
}

void CardDatabase::loadCustomCardDatabases(const QString &path)
{
    QDir dir(path);
    if(!dir.exists())
        return;

    foreach(QString fileName, dir.entryList(QStringList("*.xml"), QDir::Files | QDir::Readable, QDir::Name | QDir::IgnoreCase))
    {
        loadCardDatabase(dir.absoluteFilePath(fileName), false);
    }
}

void CardDatabase::refreshCachedReverseRelatedCards()
{
    foreach(CardInfo * card, cards)
        card->resetReverseRelatedCards2Me();

    foreach(CardInfo * card, cards)
    {
        if(card->getReverseRelatedCards().isEmpty())
            continue;

        QString relatedCardName = card->getName();
        foreach(QString targetCard, card->getReverseRelatedCards())
        {
            if (!cards.contains(targetCard))
                continue;

            cards.value(targetCard)->addReverseRelatedCards2Me(relatedCardName);
        }
    }
}

QStringList CardDatabase::getAllColors() const
{
    QSet<QString> colors;
    QHashIterator<QString, CardInfo *> cardIterator(cards);
    while (cardIterator.hasNext()) {
        const QStringList &cardColors = cardIterator.next().value()->getColors();
        if (cardColors.isEmpty())
            colors.insert("X");
        else
            for (int i = 0; i < cardColors.size(); ++i)
                colors.insert(cardColors[i]);
    }
    return colors.toList();
}

QStringList CardDatabase::getAllMainCardTypes() const
{
    QSet<QString> types;
    QHashIterator<QString, CardInfo *> cardIterator(cards);
    while (cardIterator.hasNext())
        types.insert(cardIterator.next().value()->getMainCardType());
    return types.toList();
}

void CardDatabase::checkUnknownSets()
{
    SetList sets = getSetList();

    // no set is enabled. Probably this is the first time running trice
    if(!sets.getEnabledSetsNum())
    {
        sets.guessSortKeys();
        sets.sortByKey();
        sets.enableAll();

        detectedFirstRun = true;
        return;
    }

    detectedFirstRun = false;

    int numUnknownSets = sets.getUnknownSetsNum();
    // no unkown sets. 
    if(!numUnknownSets)
        return;

    QMessageBox msgbox(QMessageBox::Question, tr("New sets found"), tr("%1 new set(s) have been found in the card database. Do you want to enable them?").arg(numUnknownSets), QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

    switch(msgbox.exec())
    {
        case QMessageBox::No:
            sets.markAllAsKnown();
            break;
        case QMessageBox::Yes:
            sets.enableAllUnknown();
            break;
        default:
            break;
    }

    return;
}

bool CardDatabase::hasDetectedFirstRun()
{
    if(detectedFirstRun)
    {
        detectedFirstRun=false;
        return true;
    }

    return false;
}
