#include "carddatabase.h"
#include "carddbparser/cockatricexml3.h"
#include "pictureloader.h"
#include "settingscache.h"
#include "spoilerbackgroundupdater.h"

#include <QCryptographicHash>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <utility>

const char *CardDatabase::TOKENS_SETNAME = "TK";

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
                   QStringMap _uuIds,
                   QStringMap _collectorNumbers,
                   QStringMap _rarities)
    : name(_name), isToken(_isToken), sets(_sets), manacost(_manacost), cmc(_cmc), cardtype(_cardtype),
      powtough(_powtough), text(_text), colors(_colors), relatedCards(_relatedCards),
      reverseRelatedCards(_reverseRelatedCards), setsNames(), upsideDownArt(_upsideDownArt), loyalty(_loyalty),
      customPicURLs(_customPicURLs), muIds(std::move(_muIds)), uuIds(std::move(_uuIds)),
      collectorNumbers(std::move(_collectorNumbers)), rarities(std::move(_rarities)), cipt(_cipt), tableRow(_tableRow)
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
                                  QStringMap _uuIds,
                                  QStringMap _collectorNumbers,
                                  QStringMap _rarities)
{
    CardInfoPtr ptr(new CardInfo(_name, _isToken, _manacost, _cmc, _cardtype, _powtough, _text, _colors, _relatedCards,
                                 _reverseRelatedCards, _upsideDownArt, _loyalty, _cipt, _tableRow, _sets,
                                 _customPicURLs, std::move(_muIds), std::move(_uuIds), std::move(_collectorNumbers),
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

CardDatabase::CardDatabase(QObject *parent) : QObject(parent), loadStatus(NotLoaded)
{
    qRegisterMetaType<CardInfoPtr>("CardInfoPtr");
    qRegisterMetaType<CardInfoPtr>("CardSetPtr");

    // add new parsers here
    availableParsers << new CockatriceXml3Parser;

    for (auto &parser : availableParsers) {
        connect(parser, SIGNAL(addCard(CardInfoPtr)), this, SLOT(addCard(CardInfoPtr)), Qt::DirectConnection);
        connect(parser, SIGNAL(addSet(CardSetPtr)), this, SLOT(addSet(CardSetPtr)), Qt::DirectConnection);
    }

    connect(settingsCache, SIGNAL(cardDatabasePathChanged()), this, SLOT(loadCardDatabases()));
}

CardDatabase::~CardDatabase()
{
    clear();
    qDeleteAll(availableParsers);
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
    for (auto parser : availableParsers) {
        parser->clearSetlist();
    }

    loadStatus = NotLoaded;

    clearDatabaseMutex->unlock();
}

void CardDatabase::addCard(CardInfoPtr card)
{
    if (card == nullptr) {
        qDebug() << "addCard(nullptr)";
        return;
    }

    // if card already exists just add the new set property
    if (cards.contains(card->getName())) {
        CardInfoPtr sameCard = cards[card->getName()];
        for (auto set : card->getSets()) {
            QString setName = set->getCorrectedShortName();
            sameCard->setSet(set);
            sameCard->setMuId(setName, card->getMuId(setName));
            sameCard->setUuId(setName, card->getUuId(setName));
            sameCard->setRarity(setName, card->getRarity(setName));
            sameCard->setSetNumber(setName, card->getCollectorNumber(setName));
        }
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

    for (auto *cardRelation : card->getRelatedCards())
        cardRelation->deleteLater();

    for (auto *cardRelation : card->getReverseRelatedCards())
        cardRelation->deleteLater();

    for (auto *cardRelation : card->getReverseRelatedCards2Me())
        cardRelation->deleteLater();

    removeCardMutex->lock();
    cards.remove(card->getName());
    simpleNameCards.remove(card->getSimpleName());
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

void CardDatabase::addSet(CardSetPtr set)
{
    sets.insert(set->getShortName(), set);
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

    for (auto parser : availableParsers) {
        file.reset();
        if (parser->getCanParseFile(fileName, file)) {
            file.reset();
            parser->parseFile(file);
            return Ok;
        }
    }

    return Invalid;
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
    for (QString fileName :
         dir.entryList(QStringList("*.xml"), QDir::Files | QDir::Readable, QDir::Name | QDir::IgnoreCase)) {
        loadCardDatabase(dir.absoluteFilePath(fileName));
    }

    // AFTER all the cards have been loaded

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
        // if some sets are first found on this run, ask the user
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
    QString fileName = settingsCache->getCustomCardDatabasePath() + "/" + CardDatabase::TOKENS_SETNAME + ".xml";

    SetNameMap tmpSets;
    CardSetPtr customTokensSet = getSet(CardDatabase::TOKENS_SETNAME);
    tmpSets.insert(CardDatabase::TOKENS_SETNAME, customTokensSet);

    CardNameMap tmpCards;
    for (CardInfoPtr card : cards) {
        if (card->getSets().contains(customTokensSet)) {
            tmpCards.insert(card->getName(), card);
        }
    }

    availableParsers.first()->saveToFile(tmpSets, tmpCards, fileName);
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