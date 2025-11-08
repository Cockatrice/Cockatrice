#include "card_database.h"

#include "../relation/card_relation.h"
#include "parser/cockatrice_xml_3.h"
#include "parser/cockatrice_xml_4.h"

#include <QCryptographicHash>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QRegularExpression>
#include <algorithm>
#include <utility>

CardDatabase::CardDatabase(QObject *parent,
                           ICardPreferenceProvider *prefs,
                           ICardDatabasePathProvider *pathProvider,
                           ICardSetPriorityController *_setPriorityController)
    : QObject(parent), setPriorityController(_setPriorityController), loadStatus(NotLoaded)
{
    qRegisterMetaType<CardInfoPtr>("CardInfoPtr");
    qRegisterMetaType<CardInfoPtr>("CardSetPtr");

    // create loader and wire it up
    loader = new CardDatabaseLoader(this, this, pathProvider, prefs);
    // re-emit loader signals (so other code doesn't need to know about internals)
    connect(loader, &CardDatabaseLoader::loadingFinished, this, &CardDatabase::cardDatabaseLoadingFinished);
    connect(loader, &CardDatabaseLoader::loadingFailed, this, &CardDatabase::cardDatabaseLoadingFailed);
    connect(loader, &CardDatabaseLoader::newSetsFound, this, &CardDatabase::cardDatabaseNewSetsFound);
    connect(loader, &CardDatabaseLoader::allNewSetsEnabled, this, &CardDatabase::cardDatabaseAllNewSetsEnabled);

    querier = new CardDatabaseQuerier(this, this, prefs);
}

CardDatabase::~CardDatabase()
{
    clear();
}

void CardDatabase::clear()
{
    QMutexLocker locker(clearDatabaseMutex);

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
}

void CardDatabase::loadCardDatabases()
{
    loadStatus = loader->loadCardDatabases();
}

bool CardDatabase::saveCustomTokensToFile()
{
    return loader->saveCustomTokensToFile();
}

void CardDatabase::refreshCachedReverseRelatedCards()
{
    for (const auto &card : cards) {
        card->resetReverseRelatedCards2Me();
    }

    for (const auto &card : cards) {
        for (auto *rel : card->getReverseRelatedCards()) {
            if (auto target = cards.value(rel->getName())) {
                auto *newRel = new CardRelation(card->getName(), rel->getAttachType(), rel->getIsCreateAllExclusion(),
                                                rel->getIsVariable(), rel->getDefaultCount(), rel->getIsPersistent());
                target->addReverseRelatedCards2Me(newRel);
            }
        }
    }
}

void CardDatabase::addCard(CardInfoPtr card)
{
    if (card == nullptr) {
        qCWarning(CardDatabaseLog) << "CardDatabase::addCard(nullptr)";
        return;
    }

    auto name = card->getName();

    // If a card already exists, just add the new set property.
    if (auto existing = cards.value(name)) {
        for (const auto &printings : card->getSets())
            for (const auto &printing : printings)
                existing->addToSet(printing.getSet(), printing);
        return;
    }

    QMutexLocker locker(addCardMutex);
    cards.insert(name, card);
    simpleNameCards.insert(card->getSimpleName(), card);

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

    QMutexLocker locker(removeCardMutex);
    cards.remove(card->getName());
    simpleNameCards.remove(card->getSimpleName());
    emit cardRemoved(card);
}

void CardDatabase::addSet(CardSetPtr set)
{
    sets.insert(set->getShortName(), set);
}

CardSetPtr CardDatabase::getSet(const QString &setName)
{
    if (sets.contains(setName)) {
        return sets.value(setName);
    } else {
        CardSetPtr newSet = CardSet::newInstance(setPriorityController, setName);
        sets.insert(setName, newSet);
        return newSet;
    }
}

CardSetList CardDatabase::getSetList() const
{
    CardSetList result;
    for (auto set : sets.values()) {
        result << set;
    }
    return result;
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
