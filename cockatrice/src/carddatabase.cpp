#include "carddatabase.h"
#include "carddbparser/cockatricexml3.h"
#include "carddbparser/cockatricexml4.h"
#include "game_specific_terms.h"
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

CardInfoPerSet::CardInfoPerSet(const CardSetPtr &_set) : set(_set)
{
}

CardInfo::CardInfo(const QString &_name,
                   const QString &_text,
                   bool _isToken,
                   QVariantHash _properties,
                   const QList<CardRelation *> &_relatedCards,
                   const QList<CardRelation *> &_reverseRelatedCards,
                   CardInfoPerSetMap _sets,
                   bool _cipt,
                   int _tableRow,
                   bool _upsideDownArt)
    : name(_name), text(_text), isToken(_isToken), properties(std::move(_properties)), relatedCards(_relatedCards),
      reverseRelatedCards(_reverseRelatedCards), sets(std::move(_sets)), cipt(_cipt), tableRow(_tableRow),
      upsideDownArt(_upsideDownArt)
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
                                  const QString &_text,
                                  bool _isToken,
                                  QVariantHash _properties,
                                  const QList<CardRelation *> &_relatedCards,
                                  const QList<CardRelation *> &_reverseRelatedCards,
                                  CardInfoPerSetMap _sets,
                                  bool _cipt,
                                  int _tableRow,
                                  bool _upsideDownArt)
{
    CardInfoPtr ptr(new CardInfo(_name, _text, _isToken, std::move(_properties), _relatedCards, _reverseRelatedCards,
                                 _sets, _cipt, _tableRow, _upsideDownArt));
    ptr->setSmartPointer(ptr);

    for (const CardInfoPerSet &set : _sets) {
        set.getPtr()->append(ptr);
    }

    return ptr;
}

QString CardInfo::getCorrectedName() const
{
    QString result = name;
    // Fire // Ice, Circle of Protection: Red, "Ach! Hans, Run!", Who/What/When/Where/Why, Question Elemental?
    return result.remove(" // ").remove(':').remove('"').remove('?').replace('/', ' ');
}

void CardInfo::addToSet(const CardSetPtr &_set, const CardInfoPerSet _info)
{
    _set->append(smartThis);
    sets.insert(_set->getShortName(), _info);

    refreshCachedSetNames();
}

void CardInfo::refreshCachedSetNames()
{
    QStringList setList;
    // update the cached list of set names
    for (const auto &set : sets) {
        if (set.getPtr()->getEnabled()) {
            setList << set.getPtr()->getShortName();
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
    QString colors = getColors();
    switch (colors.size()) {
        case 0:
            return QChar();
        case 1:
            return colors.at(0);
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
    availableParsers << new CockatriceXml4Parser;

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
    ICardDatabaseParser::clearSetlist();

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
        for (const CardInfoPerSet &set : card->getSets()) {
            sameCard->addToSet(set.getPtr(), set);
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
    for (const QString &fileName :
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

        foreach (CardRelation *cardRelation, card->getReverseRelatedCards()) {
            const QString &targetCard = cardRelation->getName();
            if (!cards.contains(targetCard)) {
                continue;
            }

            auto *newCardRelation = new CardRelation(card->getName(), cardRelation->getDoesAttach(),
                                                     cardRelation->getIsCreateAllExclusion(),
                                                     cardRelation->getIsVariable(), cardRelation->getDefaultCount());
            cards.value(targetCard)->addReverseRelatedCards2Me(newCardRelation);
        }
    }
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
    for (const CardInfoPtr &card : cards) {
        if (card->getSets().contains(CardDatabase::TOKENS_SETNAME)) {
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

// Back-compatibility methods. Remove ASAP
const QString CardInfo::getCardType() const
{
    return getProperty(Mtg::CardType);
}
void CardInfo::setCardType(const QString &value)
{
    setProperty(Mtg::CardType, value);
}
const QString CardInfo::getCmc() const
{
    return getProperty(Mtg::ConvertedManaCost);
}
const QString CardInfo::getColors() const
{
    return getProperty(Mtg::Colors);
}
void CardInfo::setColors(const QString &value)
{
    setProperty(Mtg::Colors, value);
}
const QString CardInfo::getLoyalty() const
{
    return getProperty(Mtg::Loyalty);
}
const QString CardInfo::getMainCardType() const
{
    return getProperty(Mtg::MainCardType);
}
const QString CardInfo::getManaCost() const
{
    return getProperty(Mtg::ManaCost);
}
const QString CardInfo::getPowTough() const
{
    return getProperty(Mtg::PowTough);
}
void CardInfo::setPowTough(const QString &value)
{
    setProperty(Mtg::PowTough, value);
}