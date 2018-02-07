#include "carddatabase.h"
#include "pictureloader.h"
#include "settingscache.h"
#include "spoilerbackgroundupdater.h"

#include <QCryptographicHash>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <utility>

const int CardDatabase::versionNeeded = 3;
const char *CardDatabase::TOKENS_SETNAME = "TK";

static QXmlStreamWriter &operator<<(QXmlStreamWriter &xml, const CardSetPtr &set)
{
    if (set.isNull()) {
        qDebug() << "&operator<< set is nullptr";
        return xml;
    }

    xml.writeStartElement("set");
    xml.writeTextElement("name", set->getShortName());
    xml.writeTextElement("longname", set->getLongName());
    xml.writeTextElement("settype", set->getSetType());
    xml.writeTextElement("releasedate", set->getReleaseDate().toString(Qt::ISODate));
    xml.writeEndElement();

    return xml;
}

CardSet::CardSet(const QString &_shortName,
                 const QString &_longName,
                 const QString &_setType,
                 const QDate &_releaseDate)
    : shortName(_shortName), longName(_longName), releaseDate(_releaseDate), setType(_setType)
{
    loadSetOptions();
}

CardSetPtr CardSet::newInstance(const QString &_shortName,
                                const QString &_longName,
                                const QString &_setType,
                                const QDate &_releaseDate)
{
    CardSetPtr ptr(new CardSet(_shortName, _longName, _setType, _releaseDate));
    // ptr->setSmartPointer(ptr);
    return ptr;
}

QString CardSet::getCorrectedShortName() const
{
    // For Windows machines.
    QSet<QString> invalidFileNames;
    invalidFileNames << "CON"
                     << "PRN"
                     << "AUX"
                     << "NUL"
                     << "COM1"
                     << "COM2"
                     << "COM3"
                     << "COM4"
                     << "COM5"
                     << "COM6"
                     << "COM7"
                     << "COM8"
                     << "COM9"
                     << "LPT1"
                     << "LPT2"
                     << "LPT3"
                     << "LPT4"
                     << "LPT5"
                     << "LPT6"
                     << "LPT7"
                     << "LPT8"
                     << "LPT9";

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
    settingsCache->cardDatabase().setSortKey(shortName, _sortKey);
}

void CardSet::setEnabled(bool _enabled)
{
    enabled = _enabled;
    settingsCache->cardDatabase().setEnabled(shortName, _enabled);
}

void CardSet::setIsKnown(bool _isknown)
{
    isknown = _isknown;
    settingsCache->cardDatabase().setIsKnown(shortName, _isknown);
}

class SetList::KeyCompareFunctor
{
public:
    inline bool operator()(const CardSetPtr &a, const CardSetPtr &b) const
    {
        if (a.isNull() || b.isNull()) {
            qDebug() << "SetList::KeyCompareFunctor a or b is null";
            return false;
        }

        return a->getSortKey() < b->getSortKey();
    }
};

void SetList::sortByKey()
{
    qSort(begin(), end(), KeyCompareFunctor());
}

int SetList::getEnabledSetsNum()
{
    int num = 0;
    for (int i = 0; i < size(); ++i) {
        CardSetPtr set = at(i);
        if (set && set->getEnabled()) {
            ++num;
        }
    }
    return num;
}

int SetList::getUnknownSetsNum()
{
    int num = 0;
    for (int i = 0; i < size(); ++i) {
        CardSetPtr set = at(i);
        if (set && !set->getIsKnown() && !set->getIsKnownIgnored()) {
            ++num;
        }
    }
    return num;
}

QStringList SetList::getUnknownSetsNames()
{
    QStringList sets = QStringList();
    for (int i = 0; i < size(); ++i) {
        CardSetPtr set = at(i);
        if (set && !set->getIsKnown() && !set->getIsKnownIgnored()) {
            sets << set->getShortName();
        }
    }
    return sets;
}

void SetList::enableAllUnknown()
{
    for (int i = 0; i < size(); ++i) {
        CardSetPtr set = at(i);
        if (set && !set->getIsKnown() && !set->getIsKnownIgnored()) {
            set->setIsKnown(true);
            set->setEnabled(true);
        } else if (set && set->getIsKnownIgnored() && !set->getEnabled()) {
            set->setEnabled(true);
        }
    }
}

void SetList::enableAll()
{
    for (int i = 0; i < size(); ++i) {
        CardSetPtr set = at(i);

        if (set == nullptr) {
            qDebug() << "enabledAll has null";
            continue;
        }

        if (!set->getIsKnownIgnored()) {
            set->setIsKnown(true);
        }

        set->setEnabled(true);
    }
}

void SetList::markAllAsKnown()
{
    for (int i = 0; i < size(); ++i) {
        CardSetPtr set = at(i);
        if (set && !set->getIsKnown() && !set->getIsKnownIgnored()) {
            set->setIsKnown(true);
            set->setEnabled(false);
        } else if (set && set->getIsKnownIgnored() && !set->getEnabled()) {
            set->setEnabled(true);
        }
    }
}

void SetList::guessSortKeys()
{
    // sort by release date DESC; invalid dates to the bottom.
    QDate distantFuture(2050, 1, 1);
    int aHundredYears = 36500;
    for (int i = 0; i < size(); ++i) {
        CardSetPtr set = at(i);
        if (set.isNull()) {
            qDebug() << "guessSortKeys set is null";
            continue;
        }

        QDate date = set->getReleaseDate();
        if (date.isNull()) {
            set->setSortKey(static_cast<unsigned int>(aHundredYears));
        } else {
            set->setSortKey(static_cast<unsigned int>(date.daysTo(distantFuture)));
        }
    }
}

CardInfo::CardInfo(const QString &_name,
                   bool _isToken,
                   const QString &_manacost,
                   const QString &_cmc,
                   const QString &_cardtype,
                   const QString &_powtough,
                   const QString &_text,
                   const QStringList &_colors,
                   const QList<CardRelation *> &_relatedCards,
                   const QList<CardRelation *> &_reverseRelatedCards,
                   bool _upsideDownArt,
                   const QString &_loyalty,
                   bool _cipt,
                   int _tableRow,
                   const SetList &_sets,
                   const QStringMap &_customPicURLs,
                   MuidMap _muIds,
                   QStringMap _collectorNumbers,
                   QStringMap _rarities)
    : name(_name), isToken(_isToken), sets(_sets), manacost(_manacost), cmc(_cmc), cardtype(_cardtype),
      powtough(_powtough), text(_text), colors(_colors), relatedCards(_relatedCards),
      reverseRelatedCards(_reverseRelatedCards), setsNames(), upsideDownArt(_upsideDownArt), loyalty(_loyalty),
      customPicURLs(_customPicURLs), muIds(std::move(_muIds)), collectorNumbers(std::move(_collectorNumbers)),
      rarities(std::move(_rarities)), cipt(_cipt), tableRow(_tableRow)
{
    pixmapCacheKey = QLatin1String("card_") + name;
    simpleName = CardInfo::simplifyName(name);

    refreshCachedSetNames();
}

CardInfo::~CardInfo()
{
    PictureLoader::clearPixmapCache(smartThis);
}

CardInfoPtr CardInfo::newInstance(const QString &_name,
                                  bool _isToken,
                                  const QString &_manacost,
                                  const QString &_cmc,
                                  const QString &_cardtype,
                                  const QString &_powtough,
                                  const QString &_text,
                                  const QStringList &_colors,
                                  const QList<CardRelation *> &_relatedCards,
                                  const QList<CardRelation *> &_reverseRelatedCards,
                                  bool _upsideDownArt,
                                  const QString &_loyalty,
                                  bool _cipt,
                                  int _tableRow,
                                  const SetList &_sets,
                                  const QStringMap &_customPicURLs,
                                  MuidMap _muIds,
                                  QStringMap _collectorNumbers,
                                  QStringMap _rarities)
{
    CardInfoPtr ptr(new CardInfo(_name, _isToken, _manacost, _cmc, _cardtype, _powtough, _text, _colors, _relatedCards,
                                 _reverseRelatedCards, _upsideDownArt, _loyalty, _cipt, _tableRow, _sets,
                                 _customPicURLs, std::move(_muIds), std::move(_collectorNumbers),
                                 std::move(_rarities)));
    ptr->setSmartPointer(ptr);

    for (int i = 0; i < _sets.size(); i++) {
        _sets[i]->append(ptr);
    }

    return ptr;
}

QString CardInfo::getMainCardType() const
{
    QString result = getCardType();
    /*
    Legendary Artifact Creature - Golem
    Instant // Instant
    */

    int pos;
    if ((pos = result.indexOf('-')) != -1) {
        result.remove(pos, result.length());
    }

    if ((pos = result.indexOf("—")) != -1) {
        result.remove(pos, result.length());
    }

    if ((pos = result.indexOf("//")) != -1) {
        result.remove(pos, result.length());
    }

    result = result.simplified();
    /*
    Legendary Artifact Creature
    Instant
    */

    if ((pos = result.lastIndexOf(' ')) != -1) {
        result = result.mid(pos + 1);
    }
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

void CardInfo::addToSet(CardSetPtr set)
{
    if (set.isNull()) {
        qDebug() << "addToSet(nullptr)";
        return;
    }

    set->append(smartThis);
    sets << set;

    refreshCachedSetNames();
}

void CardInfo::refreshCachedSetNames()
{
    // update the cached list of set names
    QStringList setList;
    for (int i = 0; i < sets.size(); i++) {
        if (sets[i]->getEnabled()) {
            setList << sets[i]->getShortName();
        }
    }
    setsNames = setList.join(", ");
}

QString CardInfo::simplifyName(const QString &name)
{
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

const QChar CardInfo::getColorChar() const
{
    switch (colors.size()) {
        case 0:
            return QChar();
        case 1:
            return colors.first().isEmpty() ? QChar() : colors.first().at(0);
        default:
            return QChar('m');
    }
}

static QXmlStreamWriter &operator<<(QXmlStreamWriter &xml, const CardInfoPtr &info)
{
    if (info.isNull()) {
        qDebug() << "operator<< info is nullptr";
        return xml;
    }

    xml.writeStartElement("card");
    xml.writeTextElement("name", info->getName());

    const SetList &sets = info->getSets();
    QString tmpString;
    QString tmpSet;
    for (int i = 0; i < sets.size(); i++) {
        xml.writeStartElement("set");

        tmpSet = sets[i]->getShortName();
        xml.writeAttribute("rarity", info->getRarity(tmpSet));
        xml.writeAttribute("muId", QString::number(info->getMuId(tmpSet)));

        tmpString = info->getCollectorNumber(tmpSet);
        if (!tmpString.isEmpty()) {
            xml.writeAttribute("num", info->getCollectorNumber(tmpSet));
        }

        tmpString = info->getCustomPicURL(tmpSet);
        if (!tmpString.isEmpty()) {
            xml.writeAttribute("picURL", tmpString);
        }

        xml.writeCharacters(tmpSet);
        xml.writeEndElement();
    }
    const QStringList &colors = info->getColors();
    for (int i = 0; i < colors.size(); i++) {
        xml.writeTextElement("color", colors[i]);
    }

    const QList<CardRelation *> related = info->getRelatedCards();
    for (auto i : related) {
        xml.writeStartElement("related");
        if (i->getDoesAttach()) {
            xml.writeAttribute("attach", "attach");
        }
        if (i->getIsCreateAllExclusion()) {
            xml.writeAttribute("exclude", "exclude");
        }

        if (i->getIsVariable()) {
            if (1 == i->getDefaultCount()) {
                xml.writeAttribute("count", "x");
            } else {
                xml.writeAttribute("count", "x=" + QString::number(i->getDefaultCount()));
            }
        } else if (1 != i->getDefaultCount()) {
            xml.writeAttribute("count", QString::number(i->getDefaultCount()));
        }
        xml.writeCharacters(i->getName());
        xml.writeEndElement();
    }
    const QList<CardRelation *> reverseRelated = info->getReverseRelatedCards();
    for (auto i : reverseRelated) {
        xml.writeStartElement("reverse-related");
        if (i->getDoesAttach()) {
            xml.writeAttribute("attach", "attach");
        }

        if (i->getIsCreateAllExclusion()) {
            xml.writeAttribute("exclude", "exclude");
        }

        if (i->getIsVariable()) {
            if (1 == i->getDefaultCount()) {
                xml.writeAttribute("count", "x");
            } else {
                xml.writeAttribute("count", "x=" + QString::number(i->getDefaultCount()));
            }
        } else if (1 != i->getDefaultCount()) {
            xml.writeAttribute("count", QString::number(i->getDefaultCount()));
        }
        xml.writeCharacters(i->getName());
        xml.writeEndElement();
    }
    xml.writeTextElement("manacost", info->getManaCost());
    xml.writeTextElement("cmc", info->getCmc());
    xml.writeTextElement("type", info->getCardType());
    if (!info->getPowTough().isEmpty()) {
        xml.writeTextElement("pt", info->getPowTough());
    }
    xml.writeTextElement("tablerow", QString::number(info->getTableRow()));
    xml.writeTextElement("text", info->getText());
    if (info->getMainCardType() == "Planeswalker") {
        xml.writeTextElement("loyalty", info->getLoyalty());
    }
    if (info->getCipt()) {
        xml.writeTextElement("cipt", "1");
    }
    if (info->getIsToken()) {
        xml.writeTextElement("token", "1");
    }
    if (info->getUpsideDownArt()) {
        xml.writeTextElement("upsidedown", "1");
    }

    xml.writeEndElement(); // card

    return xml;
}

CardDatabase::CardDatabase(QObject *parent) : QObject(parent), loadStatus(NotLoaded)
{
    qRegisterMetaType<CardInfoPtr>("CardInfoPtr");
    connect(settingsCache, SIGNAL(cardDatabasePathChanged()), this, SLOT(loadCardDatabases()));
}

CardDatabase::~CardDatabase()
{
    clear();
}

void CardDatabase::clear()
{
    clearDatabaseMutex->lock();

    QHashIterator<QString, CardInfoPtr> i(cards);
    while (i.hasNext()) {
        i.next();
        if (i.value()) {
            removeCard(i.value());
        }
    }

    cards.clear();
    simpleNameCards.clear();

    sets.clear();

    loadStatus = NotLoaded;

    clearDatabaseMutex->unlock();
}

void CardDatabase::addCard(CardInfoPtr card)
{
    if (card == nullptr) {
        qDebug() << "addCard(nullptr)";
        return;
    }

    addCardMutex->lock();
    cards.insert(card->getName(), card);
    simpleNameCards.insert(card->getSimpleName(), card);
    addCardMutex->unlock();
    emit cardAdded(card);
}

void CardDatabase::removeCard(CardInfoPtr card)
{
    if (card.isNull()) {
        qDebug() << "removeCard(nullptr)";
        return;
    }

    foreach (CardRelation *cardRelation, card->getRelatedCards())
        cardRelation->deleteLater();

    foreach (CardRelation *cardRelation, card->getReverseRelatedCards())
        cardRelation->deleteLater();

    foreach (CardRelation *cardRelation, card->getReverseRelatedCards2Me())
        cardRelation->deleteLater();

    removeCardMutex->lock();
    cards.remove(card->getName());
    simpleNameCards.remove(card->getSimpleName());
    card.clear();
    removeCardMutex->unlock();
    emit cardRemoved(card);
}

CardInfoPtr CardDatabase::getCard(const QString &cardName) const
{
    return getCardFromMap(cards, cardName);
}

QList<CardInfoPtr> CardDatabase::getCards(const QStringList &cardNames) const
{
    QList<CardInfoPtr> cardInfos;
    foreach (QString cardName, cardNames) {
        CardInfoPtr ptr = getCardFromMap(cards, cardName);
        if (ptr)
            cardInfos.append(ptr);
    }

    return cardInfos;
}

CardInfoPtr CardDatabase::getCardBySimpleName(const QString &cardName) const
{
    return getCardFromMap(simpleNameCards, CardInfo::simplifyName(cardName));
}

CardSetPtr CardDatabase::getSet(const QString &setName)
{
    if (sets.contains(setName)) {
        return sets.value(setName);
    } else {
        CardSetPtr newSet = CardSet::newInstance(setName);
        sets.insert(setName, newSet);
        return newSet;
    }
}

SetList CardDatabase::getSetList() const
{
    SetList result;
    QHashIterator<QString, CardSetPtr> i(sets);
    while (i.hasNext()) {
        i.next();
        result << i.value();
    }
    return result;
}

void CardDatabase::loadSetsFromXml(QXmlStreamReader &xml)
{
    while (!xml.atEnd()) {
        if (xml.readNext() == QXmlStreamReader::EndElement) {
            break;
        }

        if (xml.name() == "set") {
            QString shortName, longName, setType;
            QDate releaseDate;
            while (!xml.atEnd()) {
                if (xml.readNext() == QXmlStreamReader::EndElement) {
                    break;
                }

                if (xml.name() == "name") {
                    shortName = xml.readElementText();
                } else if (xml.name() == "longname") {
                    longName = xml.readElementText();
                } else if (xml.name() == "settype") {
                    setType = xml.readElementText();
                } else if (xml.name() == "releasedate") {
                    releaseDate = QDate::fromString(xml.readElementText(), Qt::ISODate);
                } else if (xml.name() != "") {
                    qDebug() << "[XMLReader] Unknown set property" << xml.name() << ", trying to continue anyway";
                    xml.skipCurrentElement();
                }
            }

            CardSetPtr newSet = getSet(shortName);
            newSet->setLongName(longName);
            newSet->setSetType(setType);
            newSet->setReleaseDate(releaseDate);
        }
    }
}

void CardDatabase::loadCardsFromXml(QXmlStreamReader &xml)
{
    while (!xml.atEnd()) {
        if (xml.readNext() == QXmlStreamReader::EndElement) {
            break;
        }

        if (xml.name() == "card") {
            QString name, manacost, cmc, type, pt, text, loyalty;
            QStringList colors;
            QList<CardRelation *> relatedCards, reverseRelatedCards;
            QStringMap customPicURLs;
            MuidMap muids;
            QStringMap collectorNumbers, rarities;
            SetList sets;
            int tableRow = 0;
            bool cipt = false;
            bool isToken = false;
            bool upsideDown = false;
            while (!xml.atEnd()) {
                if (xml.readNext() == QXmlStreamReader::EndElement) {
                    break;
                }

                if (xml.name() == "name") {
                    name = xml.readElementText();
                } else if (xml.name() == "manacost") {
                    manacost = xml.readElementText();
                } else if (xml.name() == "cmc") {
                    cmc = xml.readElementText();
                } else if (xml.name() == "type") {
                    type = xml.readElementText();
                } else if (xml.name() == "pt") {
                    pt = xml.readElementText();
                } else if (xml.name() == "text") {
                    text = xml.readElementText();
                } else if (xml.name() == "set") {
                    QXmlStreamAttributes attrs = xml.attributes();
                    QString setName = xml.readElementText();
                    sets.append(getSet(setName));
                    if (attrs.hasAttribute("muId")) {
                        muids[setName] = attrs.value("muId").toString().toInt();
                    }

                    if (attrs.hasAttribute("picURL")) {
                        customPicURLs[setName] = attrs.value("picURL").toString();
                    }

                    if (attrs.hasAttribute("num")) {
                        collectorNumbers[setName] = attrs.value("num").toString();
                    }

                    if (attrs.hasAttribute("rarity")) {
                        rarities[setName] = attrs.value("rarity").toString();
                    }
                } else if (xml.name() == "color") {
                    colors << xml.readElementText();
                } else if (xml.name() == "related" || xml.name() == "reverse-related") {
                    bool attach = false;
                    bool exclude = false;
                    bool variable = false;
                    int count = 1;
                    QXmlStreamAttributes attrs = xml.attributes();
                    QString cardName = xml.readElementText();
                    if (attrs.hasAttribute("count")) {
                        if (attrs.value("count").toString().indexOf("x=") == 0) {
                            variable = true;
                            count = attrs.value("count").toString().remove(0, 2).toInt();
                        } else if (attrs.value("count").toString().indexOf("x") == 0) {
                            variable = true;
                        } else {
                            count = attrs.value("count").toString().toInt();
                        }

                        if (count < 1) {
                            count = 1;
                        }
                    }

                    if (attrs.hasAttribute("attach")) {
                        attach = true;
                    }

                    if (attrs.hasAttribute("exclude")) {
                        exclude = true;
                    }

                    auto *relation = new CardRelation(cardName, attach, exclude, variable, count);
                    if (xml.name() == "reverse-related") {
                        reverseRelatedCards << relation;
                    } else {
                        relatedCards << relation;
                    }
                } else if (xml.name() == "tablerow") {
                    tableRow = xml.readElementText().toInt();
                } else if (xml.name() == "cipt") {
                    cipt = (xml.readElementText() == "1");
                } else if (xml.name() == "upsidedown") {
                    upsideDown = (xml.readElementText() == "1");
                } else if (xml.name() == "loyalty") {
                    loyalty = xml.readElementText();
                } else if (xml.name() == "token") {
                    isToken = static_cast<bool>(xml.readElementText().toInt());
                } else if (xml.name() != "") {
                    qDebug() << "[XMLReader] Unknown card property" << xml.name() << ", trying to continue anyway";
                    xml.skipCurrentElement();
                }
            }

            addCard(CardInfo::newInstance(name, isToken, manacost, cmc, type, pt, text, colors, relatedCards,
                                          reverseRelatedCards, upsideDown, loyalty, cipt, tableRow, sets, customPicURLs,
                                          muids, collectorNumbers, rarities));
        }
    }
}

CardInfoPtr CardDatabase::getCardFromMap(const CardNameMap &cardMap, const QString &cardName) const
{
    if (cardMap.contains(cardName))
        return cardMap.value(cardName);

    return {};
}

LoadStatus CardDatabase::loadFromFile(const QString &fileName)
{
    QFile file(fileName);
    file.open(QIODevice::ReadOnly);
    if (!file.isOpen()) {
        return FileError;
    }

    QXmlStreamReader xml(&file);
    while (!xml.atEnd()) {
        if (xml.readNext() == QXmlStreamReader::StartElement) {
            if (xml.name() != "cockatrice_carddatabase") {
                return Invalid;
            }

            int version = xml.attributes().value("version").toString().toInt();
            if (version < versionNeeded) {
                qDebug() << "[XMLReader] Version too old: " << version;
                return VersionTooOld;
            }

            while (!xml.atEnd()) {
                if (xml.readNext() == QXmlStreamReader::EndElement) {
                    break;
                }

                if (xml.name() == "sets") {
                    loadSetsFromXml(xml);
                } else if (xml.name() == "cards") {
                    loadCardsFromXml(xml);
                } else if (xml.name() != "") {
                    qDebug() << "[XMLReader] Unknown item" << xml.name() << ", trying to continue anyway";
                    xml.skipCurrentElement();
                }
            }
        }
    }

    if (cards.isEmpty()) {
        return NoCards;
    }

    return Ok;
}

bool CardDatabase::saveToFile(const QString &fileName, bool tokens)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    QXmlStreamWriter xml(&file);

    xml.setAutoFormatting(true);
    xml.writeStartDocument();
    xml.writeStartElement("cockatrice_carddatabase");
    xml.writeAttribute("version", QString::number(versionNeeded));

    if (!tokens) {
        xml.writeStartElement("sets");
        QHashIterator<QString, CardSetPtr> setIterator(sets);
        while (setIterator.hasNext()) {
            xml << setIterator.next().value();
        }

        xml.writeEndElement(); // sets
    }

    xml.writeStartElement("cards");
    QHashIterator<QString, CardInfoPtr> cardIterator(cards);
    while (cardIterator.hasNext()) {
        CardInfoPtr card = cardIterator.next().value();
        if (tokens == card->getIsToken()) {
            xml << card;
        }
    }
    xml.writeEndElement(); // cards

    xml.writeEndElement(); // cockatrice_carddatabase
    xml.writeEndDocument();

    return true;
}

LoadStatus CardDatabase::loadCardDatabase(const QString &path)
{
    LoadStatus tempLoadStatus = NotLoaded;
    if (!path.isEmpty()) {
        loadFromFileMutex->lock();
        tempLoadStatus = loadFromFile(path);
        loadFromFileMutex->unlock();
    }

    qDebug() << "[CardDatabase] loadCardDatabase(): Path =" << path << "Status =" << tempLoadStatus
             << "Cards =" << cards.size() << "Sets=" << sets.size();

    return tempLoadStatus;
}

LoadStatus CardDatabase::loadCardDatabases()
{
    reloadDatabaseMutex->lock();

    qDebug() << "CardDatabase::loadCardDatabases start";

    clear(); // remove old db

    loadStatus = loadCardDatabase(settingsCache->getCardDatabasePath()); // load main card database
    loadCardDatabase(settingsCache->getTokenDatabasePath());             // load tokens database
    loadCardDatabase(settingsCache->getSpoilerCardDatabasePath());       // load spoilers database

    // load custom card databases
    QDir dir(settingsCache->getCustomCardDatabasePath());
    foreach (QString fileName,
             dir.entryList(QStringList("*.xml"), QDir::Files | QDir::Readable, QDir::Name | QDir::IgnoreCase)) {
        loadCardDatabase(dir.absoluteFilePath(fileName));
    }

    // AFTER all the cards have been loaded

    // reorder sets (TODO: refactor, this smells)
    SetList allSets;
    QHashIterator<QString, CardSetPtr> setsIterator(sets);
    while (setsIterator.hasNext()) {
        allSets.append(setsIterator.next().value());
    }
    allSets.sortByKey();

    // resolve the reverse-related tags
    refreshCachedReverseRelatedCards();

    if (loadStatus == Ok) {
        checkUnknownSets(); // update deck editors, etc
        qDebug() << "CardDatabase::loadCardDatabases success";
    } else {
        qDebug() << "CardDatabase::loadCardDatabases failed";
        emit cardDatabaseLoadingFailed(); // bring up the settings dialog
    }

    reloadDatabaseMutex->unlock();
    return loadStatus;
}

void CardDatabase::refreshCachedReverseRelatedCards()
{
    for (const CardInfoPtr &card : cards)
        card->resetReverseRelatedCards2Me();

    for (const CardInfoPtr &card : cards) {
        if (card->getReverseRelatedCards().isEmpty()) {
            continue;
        }

        QString relatedCardName;
        if (card->getPowTough().size() > 0) {
            relatedCardName = card->getPowTough() + " " + card->getName(); // "n/n name"
        } else {
            relatedCardName = card->getName(); // "name"
        }

        foreach (CardRelation *cardRelation, card->getReverseRelatedCards()) {
            const QString &targetCard = cardRelation->getName();
            if (!cards.contains(targetCard)) {
                continue;
            }

            auto *newCardRelation = new CardRelation(relatedCardName, cardRelation->getDoesAttach(),
                                                     cardRelation->getIsCreateAllExclusion(),
                                                     cardRelation->getIsVariable(), cardRelation->getDefaultCount());
            cards.value(targetCard)->addReverseRelatedCards2Me(newCardRelation);
        }
    }
}

QStringList CardDatabase::getAllColors() const
{
    QSet<QString> colors;
    QHashIterator<QString, CardInfoPtr> cardIterator(cards);
    while (cardIterator.hasNext()) {
        const QStringList &cardColors = cardIterator.next().value()->getColors();
        if (cardColors.isEmpty()) {
            colors.insert("X");
        } else {
            for (int i = 0; i < cardColors.size(); ++i) {
                colors.insert(cardColors[i]);
            }
        }
    }
    return colors.toList();
}

QStringList CardDatabase::getAllMainCardTypes() const
{
    QSet<QString> types;
    QHashIterator<QString, CardInfoPtr> cardIterator(cards);
    while (cardIterator.hasNext()) {
        types.insert(cardIterator.next().value()->getMainCardType());
    }
    return types.toList();
}

void CardDatabase::checkUnknownSets()
{
    SetList sets = getSetList();

    if (sets.getEnabledSetsNum()) {
        // if some sets are first found on thus run, ask the user
        int numUnknownSets = sets.getUnknownSetsNum();
        QStringList unknownSetNames = sets.getUnknownSetsNames();
        if (numUnknownSets > 0) {
            emit cardDatabaseNewSetsFound(numUnknownSets, unknownSetNames);
        } else {
            sets.markAllAsKnown();
        }
    } else {
        // No set enabled. Probably this is the first time running trice
        sets.guessSortKeys();
        sets.sortByKey();
        sets.enableAll();
        notifyEnabledSetsChanged();

        emit cardDatabaseAllNewSetsEnabled();
    }
}

void CardDatabase::enableAllUnknownSets()
{
    SetList sets = getSetList();
    sets.enableAllUnknown();
}

void CardDatabase::markAllSetsAsKnown()
{
    SetList sets = getSetList();
    sets.markAllAsKnown();
}

void CardDatabase::notifyEnabledSetsChanged()
{
    // refresh the list of cached set names
    for (const CardInfoPtr &card : cards)
        card->refreshCachedSetNames();

    // inform the carddatabasemodels that they need to re-check their list of cards
    emit cardDatabaseEnabledSetsChanged();
}

bool CardDatabase::saveCustomTokensToFile()
{
    CardSetPtr customTokensSet = getSet(CardDatabase::TOKENS_SETNAME);
    QString fileName = settingsCache->getCustomCardDatabasePath() + "/" + CardDatabase::TOKENS_SETNAME + ".xml";
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    QXmlStreamWriter xml(&file);

    xml.setAutoFormatting(true);
    xml.writeStartDocument();
    xml.writeStartElement("cockatrice_carddatabase");
    xml.writeAttribute("version", QString::number(versionNeeded));

    xml.writeStartElement("cards");
    QHashIterator<QString, CardInfoPtr> cardIterator(cards);
    while (cardIterator.hasNext()) {
        CardInfoPtr card = cardIterator.next().value();
        if (card->getSets().contains(customTokensSet)) {
            xml << card;
        }
    }
    xml.writeEndElement(); // cards

    xml.writeEndElement(); // cockatrice_carddatabase
    xml.writeEndDocument();

    return true;
}

CardRelation::CardRelation(const QString &_name,
                           bool _doesAttach,
                           bool _isCreateAllExclusion,
                           bool _isVariableCount,
                           int _defaultCount)
    : name(_name), doesAttach(_doesAttach), isCreateAllExclusion(_isCreateAllExclusion),
      isVariableCount(_isVariableCount), defaultCount(_defaultCount)
{
}

void CardInfo::resetReverseRelatedCards2Me()
{
    foreach (CardRelation *cardRelation, this->getReverseRelatedCards2Me()) {
        cardRelation->deleteLater();
    }
    reverseRelatedCardsToMe = QList<CardRelation *>();
}