#include "card_database.h"

#include "../../client/ui/picture_loader/picture_loader.h"
#include "../../settings/cache_settings.h"
#include "../../utility/card_set_comparator.h"
#include "./card_database_parser/cockatrice_xml_3.h"
#include "./card_database_parser/cockatrice_xml_4.h"

#include <QCryptographicHash>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QMessageBox>
#include <QRegularExpression>
#include <algorithm>
#include <utility>

CardDatabase::CardDatabase(QObject *parent) : QObject(parent), loadStatus(NotLoaded)
{
    qRegisterMetaType<CardInfoPtr>("CardInfoPtr");
    qRegisterMetaType<CardInfoPtr>("CardSetPtr");

    // add new parsers here
    availableParsers << new CockatriceXml4Parser;
    availableParsers << new CockatriceXml3Parser;

    for (auto &parser : availableParsers) {
        connect(parser, &ICardDatabaseParser::addCard, this, &CardDatabase::addCard, Qt::DirectConnection);
        connect(parser, &ICardDatabaseParser::addSet, this, &CardDatabase::addSet, Qt::DirectConnection);
    }

    connect(&SettingsCache::instance(), &SettingsCache::cardDatabasePathChanged, this,
            &CardDatabase::loadCardDatabases);
}

CardDatabase::~CardDatabase()
{
    clear();
    qDeleteAll(availableParsers);
}

void CardDatabase::clear()
{
    clearDatabaseMutex->lock();

    for (const auto &card : cards.values()) {
        if (card) {
            removeCard(card);
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
        qCWarning(CardDatabaseLog) << "CardDatabase::addCard(nullptr)";
        return;
    }

    // if card already exists just add the new set property
    if (cards.contains(card->getName())) {
        CardInfoPtr sameCard = cards[card->getName()];
        for (const auto &printings : card->getSets()) {
            for (const PrintingInfo &printing : printings) {
                sameCard->addToSet(printing.getSet(), printing);
            }
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
        qCWarning(CardDatabaseLog) << "CardDatabase::removeCard(nullptr)";
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
    return cards.value(cardName);
}

QList<CardInfoPtr> CardDatabase::getCards(const QStringList &cardNames) const
{
    QList<CardInfoPtr> cardInfos;
    for (const QString &cardName : cardNames) {
        CardInfoPtr ptr = cards.value(cardName);
        if (ptr)
            cardInfos.append(ptr);
    }

    return cardInfos;
}

QList<CardInfoPtr> CardDatabase::getCardsByNameAndProviderId(const QMap<QString, QString> &cardNames) const
{
    QList<CardInfoPtr> cardInfos;
    for (const QString &cardName : cardNames) {
        CardInfoPtr ptr = getCardByNameAndProviderId(cardName, cardNames[cardName]);
        if (ptr)
            cardInfos.append(ptr);
    }

    return cardInfos;
}

CardInfoPtr CardDatabase::getCardByNameAndProviderId(const QString &cardName, const QString &providerId) const
{
    auto info = getCard(cardName);
    if (providerId.isNull() || providerId.isEmpty() || info.isNull()) {
        return info;
    }

    for (const auto &printings : info->getSets()) {
        for (const auto &printing : printings) {
            if (printing.getProperty("uuid") == providerId) {
                CardInfoPtr cardFromSpecificSet = info->clone();
                cardFromSpecificSet->setPixmapCacheKey(QLatin1String("card_") + QString(info->getName()) +
                                                       QString("_") + QString(printing.getProperty("uuid")));
                return cardFromSpecificSet;
            }
        }
    }
    return {};
}

CardInfoPtr CardDatabase::getCardBySimpleName(const QString &cardName) const
{
    return simpleNameCards.value(CardInfo::simplifyName(cardName));
}

CardInfoPtr CardDatabase::guessCard(const QString &cardName, const QString &providerId) const
{
    CardInfoPtr temp = providerId.isEmpty() ? getCard(cardName) : getCardByNameAndProviderId(cardName, providerId);

    if (temp == nullptr) { // get card by simple name instead
        temp = getCardBySimpleName(cardName);
        if (temp == nullptr) { // still could not find the card, so simplify the cardName too
            const auto &simpleCardName = CardInfo::simplifyName(cardName);
            temp = getCardBySimpleName(simpleCardName);
        }
    }
    return temp; // returns nullptr if not found
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
    for (auto set : sets.values()) {
        result << set;
    }
    return result;
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
    auto startTime = QTime::currentTime();
    LoadStatus tempLoadStatus = NotLoaded;
    if (!path.isEmpty()) {
        loadFromFileMutex->lock();
        tempLoadStatus = loadFromFile(path);
        loadFromFileMutex->unlock();
    }

    int msecs = startTime.msecsTo(QTime::currentTime());
    qCInfo(CardDatabaseLoadingLog) << "Loaded card database: Path =" << path << "Status =" << tempLoadStatus
                                   << "Cards =" << cards.size() << "Sets =" << sets.size()
                                   << QString("%1ms").arg(msecs);

    return tempLoadStatus;
}

LoadStatus CardDatabase::loadCardDatabases()
{
    reloadDatabaseMutex->lock();

    qCInfo(CardDatabaseLoadingLog) << "Card Database Loading Started";

    clear(); // remove old db

    loadStatus = loadCardDatabase(SettingsCache::instance().getCardDatabasePath()); // load main card database
    loadCardDatabase(SettingsCache::instance().getTokenDatabasePath());             // load tokens database
    loadCardDatabase(SettingsCache::instance().getSpoilerCardDatabasePath());       // load spoilers database

    // find all custom card databases, recursively & following symlinks
    // then load them alphabetically
    QDirIterator customDatabaseIterator(SettingsCache::instance().getCustomCardDatabasePath(), QStringList() << "*.xml",
                                        QDir::Files, QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);
    QStringList databasePaths;
    while (customDatabaseIterator.hasNext()) {
        customDatabaseIterator.next();
        databasePaths.push_back(customDatabaseIterator.filePath());
    }
    databasePaths.sort();

    for (auto i = 0; i < databasePaths.size(); ++i) {
        const auto &databasePath = databasePaths.at(i);
        qCInfo(CardDatabaseLoadingLog) << "Loading Custom Set" << i << "(" << databasePath << ")";
        loadCardDatabase(databasePath);
    }

    // AFTER all the cards have been loaded

    // Refresh the pixmap cache keys for all cards by setting them to the UUID of the preferred printing
    refreshPreferredPrintings();
    // resolve the reverse-related tags
    refreshCachedReverseRelatedCards();

    if (loadStatus == Ok) {
        checkUnknownSets(); // update deck editors, etc
        qCInfo(CardDatabaseLoadingSuccessOrFailureLog) << "Card Database Loading Success";
        emit cardDatabaseLoadingFinished();
    } else {
        qCInfo(CardDatabaseLoadingSuccessOrFailureLog) << "Card Database Loading Failed";
        emit cardDatabaseLoadingFailed(); // bring up the settings dialog
    }

    reloadDatabaseMutex->unlock();
    return loadStatus;
}

void CardDatabase::refreshPreferredPrintings()
{
    for (const CardInfoPtr &card : cards) {
        card->setPixmapCacheKey(QLatin1String("card_") + QString(card->getName()) + QString("_") +
                                QString(getPreferredPrintingProviderIdForCard(card->getName())));
    }
}

PrintingInfo CardDatabase::getPreferredPrinting(const QString &cardName) const
{
    CardInfoPtr cardInfo = getCard(cardName);
    return getPreferredPrinting(cardInfo);
}

PrintingInfo CardDatabase::getPreferredPrinting(const CardInfoPtr &cardInfo) const
{
    if (!cardInfo) {
        return PrintingInfo(nullptr);
    }

    SetToPrintingsMap setMap = cardInfo->getSets();
    if (setMap.empty()) {
        return PrintingInfo(nullptr);
    }

    CardSetPtr preferredSet = nullptr;
    PrintingInfo preferredPrinting;
    SetPriorityComparator comparator;

    for (const auto &printings : setMap) {
        for (auto &printing : printings) {
            CardSetPtr currentSet = printing.getSet();
            if (!preferredSet || comparator(currentSet, preferredSet)) {
                preferredSet = currentSet;
                preferredPrinting = printing;
            }
        }
    }

    if (preferredSet) {
        return preferredPrinting;
    }

    return PrintingInfo(nullptr);
}

PrintingInfo CardDatabase::getSpecificPrinting(const QString &cardName, const QString &providerId) const
{
    CardInfoPtr cardInfo = getCard(cardName);
    if (!cardInfo) {
        return PrintingInfo(nullptr);
    }

    SetToPrintingsMap setMap = cardInfo->getSets();
    if (setMap.empty()) {
        return PrintingInfo(nullptr);
    }

    for (const auto &printings : setMap) {
        for (auto &cardInfoForSet : printings) {
            if (cardInfoForSet.getProperty("uuid") == providerId) {
                return cardInfoForSet;
            }
        }
    }

    if (providerId.isNull()) {
        return getPreferredPrinting(cardName);
    }

    return PrintingInfo(nullptr);
}

PrintingInfo CardDatabase::getSpecificPrinting(const QString &cardName,
                                               const QString &setShortName,
                                               const QString &collectorNumber) const
{
    CardInfoPtr cardInfo = getCard(cardName);
    if (!cardInfo) {
        return PrintingInfo(nullptr);
    }

    SetToPrintingsMap setMap = cardInfo->getSets();
    if (setMap.empty()) {
        return PrintingInfo(nullptr);
    }

    for (const auto &printings : setMap) {
        for (auto &cardInfoForSet : printings) {
            if (collectorNumber != nullptr) {
                if (cardInfoForSet.getSet()->getShortName() == setShortName &&
                    cardInfoForSet.getProperty("num") == collectorNumber) {
                    return cardInfoForSet;
                }
            } else {
                if (cardInfoForSet.getSet()->getShortName() == setShortName) {
                    return cardInfoForSet;
                }
            }
        }
    }

    return PrintingInfo(nullptr);
}

QString CardDatabase::getPreferredPrintingProviderIdForCard(const QString &cardName)
{
    PrintingInfo preferredPrinting = getPreferredPrinting(cardName);
    QString preferredPrintingProviderId = preferredPrinting.getProperty(QString("uuid"));
    if (preferredPrintingProviderId.isEmpty()) {
        CardInfoPtr defaultCardInfo = getCard(cardName);
        if (defaultCardInfo.isNull()) {
            return cardName;
        }
        return defaultCardInfo->getName();
    }
    return preferredPrintingProviderId;
}

bool CardDatabase::isProviderIdForPreferredPrinting(const QString &cardName, const QString &providerId)
{
    if (providerId.startsWith("card_")) {
        return providerId ==
               QLatin1String("card_") + cardName + QString("_") + getPreferredPrintingProviderIdForCard(cardName);
    }
    return providerId == getPreferredPrintingProviderIdForCard(cardName);
}

PrintingInfo CardDatabase::getSetInfoForCard(const CardInfoPtr &_card)
{
    const SetToPrintingsMap &setMap = _card->getSets();
    if (setMap.empty()) {
        return PrintingInfo(nullptr);
    }

    for (const auto &printings : setMap) {
        for (const auto &cardInfoForSet : printings) {
            if (QLatin1String("card_") + _card->getName() + QString("_") + cardInfoForSet.getProperty("uuid") ==
                _card->getPixmapCacheKey()) {
                return cardInfoForSet;
            }
        }
    }

    return PrintingInfo(nullptr);
}

void CardDatabase::refreshCachedReverseRelatedCards()
{
    for (const CardInfoPtr &card : cards)
        card->resetReverseRelatedCards2Me();

    for (const CardInfoPtr &card : cards) {
        if (card->getReverseRelatedCards().isEmpty()) {
            continue;
        }

        for (CardRelation *cardRelation : card->getReverseRelatedCards()) {
            const QString &targetCard = cardRelation->getName();
            if (!cards.contains(targetCard)) {
                continue;
            }

            auto *newCardRelation = new CardRelation(
                card->getName(), cardRelation->getAttachType(), cardRelation->getIsCreateAllExclusion(),
                cardRelation->getIsVariable(), cardRelation->getDefaultCount(), cardRelation->getIsPersistent());
            cards.value(targetCard)->addReverseRelatedCards2Me(newCardRelation);
        }
    }
}

QStringList CardDatabase::getAllMainCardTypes() const
{
    QSet<QString> types;
    for (const auto &card : cards.values()) {
        types.insert(card->getMainCardType());
    }
    return types.values();
}

QMap<QString, int> CardDatabase::getAllMainCardTypesWithCount() const
{
    QMap<QString, int> typeCounts;

    for (const auto &card : cards.values()) {
        QString type = card->getMainCardType();
        typeCounts[type]++;
    }

    return typeCounts;
}

QMap<QString, int> CardDatabase::getAllSubCardTypesWithCount() const
{
    QMap<QString, int> typeCounts;

    for (const auto &card : cards.values()) {
        QString type = card->getCardType();

        QStringList parts = type.split(" — ");

        if (parts.size() > 1) { // Ensure there are subtypes
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
            QStringList subtypes = parts[1].split(" ", Qt::SkipEmptyParts);
#else
            QStringList subtypes = parts[1].split(" ", QString::SkipEmptyParts);
#endif

            for (const QString &subtype : subtypes) {
                typeCounts[subtype]++;
            }
        }
    }

    return typeCounts;
}

void CardDatabase::checkUnknownSets()
{
    auto _sets = getSetList();

    if (_sets.getEnabledSetsNum()) {
        // if some sets are first found on this run, ask the user
        int numUnknownSets = _sets.getUnknownSetsNum();
        QStringList unknownSetNames = _sets.getUnknownSetsNames();
        if (numUnknownSets > 0) {
            emit cardDatabaseNewSetsFound(numUnknownSets, unknownSetNames);
        } else {
            _sets.markAllAsKnown();
        }
    } else {
        // No set enabled. Probably this is the first time running trice
        _sets.guessSortKeys();
        _sets.sortByKey();
        _sets.enableAll();
        notifyEnabledSetsChanged();

        emit cardDatabaseAllNewSetsEnabled();
    }
}

void CardDatabase::enableAllUnknownSets()
{
    auto _sets = getSetList();
    _sets.enableAllUnknown();
}

void CardDatabase::markAllSetsAsKnown()
{
    auto _sets = getSetList();
    _sets.markAllAsKnown();
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
    QString fileName = SettingsCache::instance().getCustomCardDatabasePath() + "/" + CardSet::TOKENS_SETNAME + ".xml";

    SetNameMap tmpSets;
    CardSetPtr customTokensSet = getSet(CardSet::TOKENS_SETNAME);
    tmpSets.insert(CardSet::TOKENS_SETNAME, customTokensSet);

    CardNameMap tmpCards;
    for (const CardInfoPtr &card : cards) {
        if (card->getSets().contains(CardSet::TOKENS_SETNAME)) {
            tmpCards.insert(card->getName(), card);
        }
    }

    availableParsers.first()->saveToFile(tmpSets, tmpCards, fileName);
    return true;
}
