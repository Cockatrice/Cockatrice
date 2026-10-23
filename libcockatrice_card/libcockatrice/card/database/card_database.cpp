#include "card_database.h"

#include "../relation/card_relation.h"
#include "libcockatrice/card/database/card_database_data.h"
#include "libcockatrice/card/database/card_database_loader.h"
#include "libcockatrice/card/database/card_database_querier.h"
#include "libcockatrice/card/database/parser/card_database_parser.h"
#include "libcockatrice/card/format/format_legality_rules.h"
#include "libcockatrice/card/printing/printing_info.h"
#include "libcockatrice/card/set/card_set.h"
#include "libcockatrice/card/set/card_set_list.h"

#include <QDebug>
#include <QList>
#include <QMessageLogger>
#include <QMetaObject>
#include <QMetaType>
#include <QMutexLocker>
#include <QSharedPointer>
#include <utility>

class ICardDatabasePathProvider;
class ICardPreferenceProvider;
class ICardSetPriorityController;

CardDatabase::CardDatabase(QObject *parent,
                           ICardPreferenceProvider *prefs,
                           ICardDatabasePathProvider *pathProvider,
                           ICardSetPriorityController *_setPriorityController)
    : QObject(parent), setPriorityController(_setPriorityController), loadStatus(NotLoaded)
{
    qRegisterMetaType<CardInfoPtr>("CardInfoPtr");
    qRegisterMetaType<CardInfoPtr>("CardSetPtr");
    qRegisterMetaType<CardDatabaseData>("CardDatabaseData");

    // create loader and wire it up
    loader = new CardDatabaseLoader(this, this, pathProvider, setPriorityController);
    // re-emit loader signals (so other code doesn't need to know about internals)
    connect(loader, &CardDatabaseLoader::loadingFinished, this, &CardDatabase::cardDatabaseLoadingFinished);
    connect(loader, &CardDatabaseLoader::loadingFailed, this, &CardDatabase::cardDatabaseLoadingFailed);
    connect(loader, &CardDatabaseLoader::newSetsFound, this, &CardDatabase::cardDatabaseNewSetsFound);
    connect(loader, &CardDatabaseLoader::allNewSetsEnabled, this, &CardDatabase::cardDatabaseAllNewSetsEnabled);
    // swap the finished snapshot into the live database. Uses AutoConnection so
    // that cross-thread loads are delivered via the event loop while same-thread/
    // synchronous callers get the swap immediately.
    connect(loader, &CardDatabaseLoader::databaseDataReady, this, &CardDatabase::swapInDatabaseData);

    querier = new CardDatabaseQuerier(this, this, prefs);
}

CardDatabase::~CardDatabase()
{
    clear();
}

void CardDatabase::clear()
{
    QMutexLocker locker(clearDatabaseMutex);

    // CardInfo keeps its CardSetPtr in setsToPrintings while CardSet keeps strong CardInfoPtrs in its
    // QList base, so the two form a reference cycle. Empty the sets before dropping the cards;
    // otherwise a reload leaves the whole previous database alive.
    for (const auto &set : sets) {
        if (set) {
            set->clear();
        }
    }

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

void CardDatabase::reloadCardDatabasesAndNotify()
{
    // An explicit reload bypasses the binary cache so the files on disk are
    // always re-read, even when their size/mtime hash still matches the cache.
    // Models refresh from the swap's cardDatabaseReset in a single batch, so
    // no cardDatabaseEnabledSetsChanged is emitted here: that rebuilt the
    // 36k-card models a second time on the GUI thread, which froze startup
    // after enabling new/custom sets. The follow-up unknown-sets check is
    // queued to the GUI thread so it sees the swapped-in data (this may run
    // on a worker via QtConcurrent::run); it is silent unless new sets
    // appeared mid-reload.
    loadStatus = loader->loadCardDatabases(/* ignoreCache */ true);

    if (loadStatus == Ok) {
        QMetaObject::invokeMethod(this, &CardDatabase::checkUnknownSets, Qt::AutoConnection);
    }
}

bool CardDatabase::saveCustomTokensToFile()
{
    return loader->saveCustomTokensToFile();
}

void CardDatabase::refreshCachedReverseRelatedCards()
{
    refreshCachedReverseRelatedCards(cards);
}

void CardDatabase::refreshCachedReverseRelatedCards(CardNameMap &cardMap)
{
    for (const auto &card : cardMap) {
        card->resetReverseRelatedCards2Me();
    }

    for (const auto &card : cardMap) {
        for (auto *rel : card->getReverseRelatedCards()) {
            if (auto target = cardMap.value(rel->getName())) {
                auto *newRel = new CardRelation(card->getName(), rel->getAttachType(), rel->getIsCreateAllExclusion(),
                                                rel->getIsVariable(), rel->getDefaultCount(), rel->getIsPersistent(),
                                                rel->getIsFaceDown());
                target->addReverseRelatedCards2Me(newRel);
            }
        }
    }
}

void CardDatabase::addCard(const CardInfoPtr &card)
{
    if (card == nullptr) {
        qCWarning(CardDatabaseLog) << "CardDatabase::addCard(nullptr)";
        return;
    }

    auto name = card->getName();

    // If a card already exists, just add the new set property.
    if (auto existing = cards.value(name)) {
        for (const auto &printings : card->getSets()) {
            for (const auto &printing : printings) {
                existing->addToSet(printing.getSet(), printing);
            }
        }
        return;
    }

    QMutexLocker locker(addCardMutex);
    cards.insert(name, card);
    simpleNameCards.insert(card->getSimpleName(), card);

    emit cardAdded(card);
}

void CardDatabase::removeCard(const CardInfoPtr &card)
{
    if (card.isNull()) {
        qCWarning(CardDatabaseLog) << "CardDatabase::removeCard(nullptr)";
        return;
    }

    for (auto *cardRelation : card->getRelatedCards()) {
        cardRelation->deleteLater();
    }

    for (auto *cardRelation : card->getReverseRelatedCards()) {
        cardRelation->deleteLater();
    }

    for (auto *cardRelation : card->getReverseRelatedCards2Me()) {
        cardRelation->deleteLater();
    }

    // Detach the card from its sets as well: the set holds a strong reference back to the card,
    // so leaving it there keeps both objects alive after the database drops the card.
    for (const auto &printings : card->getSets()) {
        for (const PrintingInfo &printing : printings) {
            if (const CardSetPtr set = printing.getSet()) {
                set->removeAll(card);
            }
            break;
        }
    }

    QMutexLocker locker(removeCardMutex);
    cards.remove(card->getName());
    simpleNameCards.remove(card->getSimpleName());
    emit cardRemoved(card);
}

void CardDatabase::addSet(const CardSetPtr &set)
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
            _sets.markAllAsKnownBatched(setPriorityController);
        }
    } else {
        // No set enabled. Probably this is the first time running trice
        _sets.enableAllBatched(setPriorityController);
        _sets.sortByKey();

        // No cardDatabaseEnabledSetsChanged here: the reload chained off
        // cardDatabaseAllNewSetsEnabled re-parses with the sets enabled and its
        // swap's cardDatabaseReset rebuilds the models in one batch. Emitting
        // both back-to-back rebuilt the 36k-card models twice on the GUI thread,
        // stalling startup. (When the card list is empty the slot returns early
        // without reloading, but there is nothing to display either.)
        emit cardDatabaseAllNewSetsEnabled();
    }
}

void CardDatabase::enableAllUnknownSets()
{
    auto _sets = getSetList();
    _sets.enableAllUnknownBatched(setPriorityController);
}

void CardDatabase::markAllSetsAsKnown()
{
    auto _sets = getSetList();
    _sets.markAllAsKnownBatched(setPriorityController);
}

void CardDatabase::notifyEnabledSetsChanged(bool recomputeCachedSets)
{
    if (recomputeCachedSets) {
        // refresh the list of cached set names / alt names for every card; this is
        // only needed when the enabled-set state actually changed (e.g. the user
        // toggled a set in the settings UI).
        for (const CardInfoPtr &card : cards) {
            card->refreshCachedSets();
        }
    }

    // inform the carddatabasemodels that they need to re-check their list of cards
    emit cardDatabaseEnabledSetsChanged();
}

void CardDatabase::addFormat(const FormatRulesPtr &format)
{
    formats.insert(format->formatName.toLower(), format);
}

void CardDatabase::swapInDatabaseData(CardDatabaseData data)
{
    for (const auto &card : cards) {
        for (auto *rel : card->getRelatedCards()) {
            rel->deleteLater();
        }
        for (auto *rel : card->getReverseRelatedCards()) {
            rel->deleteLater();
        }
        for (auto *rel : card->getReverseRelatedCards2Me()) {
            rel->deleteLater();
        }
    }

    // Break the CardSet <-> CardInfo cycle before replacing the containers, otherwise the previous
    // snapshot survives the reload through the sets' strong card lists.
    for (const auto &set : sets) {
        if (set) {
            set->clear();
        }
    }

    cards = std::move(data.cards);
    simpleNameCards = std::move(data.simpleNameCards);
    sets = std::move(data.sets);
    formats = std::move(data.formats);

    loadStatus = cards.isEmpty() ? NotLoaded : Ok;

    // inform listeners that the whole database was replaced; they should
    // rebuild from the live containers in a single batch instead of reacting
    // to individual card additions.
    emit cardDatabaseReset();
}
