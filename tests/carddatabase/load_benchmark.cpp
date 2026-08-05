/*
 * Standalone benchmark for card database loading.
 *
 * Measures wall-clock cost and model churn:
 *   - Cold load (XML -> write cache)
 *   - Warm load (read cache)
 *
 * Run:
 *   load_benchmark [--carddb PATH] [--tokens PATH] [--spoilers PATH] [--custom DIR]
 *
 * All arguments are optional; they default to the XDG Cockatrice data location
 * (~/.local/share/Cockatrice/Cockatrice/...).
 */

#include "mocks.h"

#include "gtest/gtest.h"
#include <QAbstractItemModel>
#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QFile>
#include <QLoggingCategory>
#include <QStringList>
#include <QTimer>
#include <libcockatrice/card/database/card_database.h>
#include <libcockatrice/card/database/card_database_loader.h>
#include <libcockatrice/interfaces/interface_card_database_path_provider.h>
#include <libcockatrice/interfaces/noop_card_preference_provider.h>
#include <libcockatrice/interfaces/noop_card_set_priority_controller.h>
#include <libcockatrice/models/database/card_database_model.h>

static QString defaultXdgPath(const QString &file)
{
    QString base = QDir::homePath() + "/.local/share/Cockatrice/Cockatrice/";
    return base + file;
}

class CliCardDatabasePathProvider : public ICardDatabasePathProvider
{
public:
    QString cardDb;
    QString tokens;
    QString spoilers;
    QString custom;

    QString getCardDatabasePath() const override
    {
        return cardDb;
    }
    QString getTokenDatabasePath() const override
    {
        return tokens;
    }
    QString getSpoilerCardDatabasePath() const override
    {
        return spoilers;
    }
    QString getCustomCardDatabasePath() const override
    {
        return custom;
    }
};

namespace
{
struct ChurnCounters
{
    qint64 cardAddedEmissions = 0;
    qint64 cardRemovedEmissions = 0;
    qint64 modelRowsInserted = 0;
    qint64 modelRowsRemoved = 0;
    qint64 loadingFinishedSignals = 0;
    qint64 databaseResetSignals = 0;
    qint64 modelResets = 0;
};

struct RunResult
{
    qint64 elapsedMs = 0;
    qint64 cardCount = 0;
    qint64 setCount = 0;
    LoadStatus status = NotLoaded;
    ChurnCounters churn;
    qint64 propsQueried = 0;
    qint64 propTotal = 0;
};

void deleteCache(const QString &cardDbPath)
{
    const QString cachePath = cardDbPath + ".cache";
    if (QFile::exists(cachePath)) {
        QFile::remove(cachePath);
    }
}

RunResult runLoad(CardDatabase *db, ChurnCounters &counters)
{
    const ChurnCounters before = counters;

    QElapsedTimer timer;
    timer.start();

    db->loadCardDatabases();

    RunResult result;
    result.elapsedMs = timer.elapsed();
    result.cardCount = db->getCardList().size();
    result.setCount = db->getSetList().size();
    result.status = db->getLoadStatus();

    result.churn.cardAddedEmissions = counters.cardAddedEmissions - before.cardAddedEmissions;
    result.churn.cardRemovedEmissions = counters.cardRemovedEmissions - before.cardRemovedEmissions;
    result.churn.modelRowsInserted = counters.modelRowsInserted - before.modelRowsInserted;
    result.churn.modelRowsRemoved = counters.modelRowsRemoved - before.modelRowsRemoved;
    result.churn.loadingFinishedSignals = counters.loadingFinishedSignals - before.loadingFinishedSignals;
    result.churn.databaseResetSignals = counters.databaseResetSignals - before.databaseResetSignals;
    result.churn.modelResets = counters.modelResets - before.modelResets;

    for (const CardInfoPtr &card : db->getCardList()) {
        result.propTotal += card->getPropertiesHash().size();
        if (!card->getProperty("colors").isEmpty()) {
            ++result.propsQueried;
        }
    }

    return result;
}

void printResult(const QString &label, const RunResult &r)
{
    qInfo() << "----------------------------------------";
    qInfo() << label;
    qInfo() << "  Load wall-clock time :" << r.elapsedMs << "ms";
    qInfo() << "  Cards loaded        :" << r.cardCount;
    qInfo() << "  Sets loaded         :" << r.setCount;
    qInfo() << "  Load status         :" << r.status;
    qInfo() << "  Properties queried  :" << r.propsQueried << "/" << r.cardCount
            << " cards, total props:" << r.propTotal;
    qInfo() << "  CHURN during load:";
    qInfo() << "    cardAdded emissions        :" << r.churn.cardAddedEmissions;
    qInfo() << "    cardRemoved emissions      :" << r.churn.cardRemovedEmissions;
    qInfo() << "    model rowsInserted         :" << r.churn.modelRowsInserted;
    qInfo() << "    model rowsRemoved          :" << r.churn.modelRowsRemoved;
    qInfo() << "    loadingFinished signals    :" << r.churn.loadingFinishedSignals;
    qInfo() << "    databaseReset signals      :" << r.churn.databaseResetSignals;
    qInfo() << "    model resets (batch)       :" << r.churn.modelResets;
}
} // namespace

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    QStringList args = app.arguments();
    CliCardDatabasePathProvider pathProvider;
    pathProvider.cardDb = defaultXdgPath("cards.xml");
    pathProvider.tokens = defaultXdgPath("tokens.xml");
    pathProvider.spoilers = defaultXdgPath("spoiler.xml");
    pathProvider.custom = defaultXdgPath("customsets/");

    for (int i = 1; i < args.size(); ++i) {
        const QString &a = args.at(i);
        if (a == "--carddb" && i + 1 < args.size()) {
            pathProvider.cardDb = args.at(++i);
        } else if (a == "--tokens" && i + 1 < args.size()) {
            pathProvider.tokens = args.at(++i);
        } else if (a == "--spoilers" && i + 1 < args.size()) {
            pathProvider.spoilers = args.at(++i);
        } else if (a == "--custom" && i + 1 < args.size()) {
            pathProvider.custom = args.at(++i);
        } else if (a == "--help" || a == "-h") {
            qInfo() << "Usage: load_benchmark [--carddb PATH] [--tokens PATH] [--spoilers PATH] [--custom DIR]";
            return 0;
        }
    }

    QLoggingCategory::setFilterRules("card_database.loading=true\n"
                                     "card_database.loading.success=true\n");

    qInfo() << "=== Card Database Load Benchmark ===";
    qInfo() << "carddb      :" << pathProvider.cardDb;
    qInfo() << "tokens      :" << pathProvider.tokens;
    qInfo() << "spoilers    :" << pathProvider.spoilers;
    qInfo() << "custom      :" << pathProvider.custom;
    qInfo() << "";

    ChurnCounters counters;

    auto runScenario = [&](bool deleteCacheBefore) -> RunResult {
        if (deleteCacheBefore) {
            deleteCache(pathProvider.cardDb);
        }

        // Fresh database each scenario so signals don't accumulate across scenarios
        // for the churn counters, but we keep the same counters object to get a
        // running total if desired.
        CardDatabase db(nullptr, new NoopCardPreferenceProvider(), &pathProvider, new NoopCardSetPriorityController());

        // Re-wire churn counters for this db instance
        QObject::connect(&db, &CardDatabase::cardAdded, &app,
                         [&counters](const CardInfoPtr &) { ++counters.cardAddedEmissions; });
        QObject::connect(&db, &CardDatabase::cardRemoved, &app,
                         [&counters](CardInfoPtr) { ++counters.cardRemovedEmissions; });
        QObject::connect(&db, &CardDatabase::cardDatabaseLoadingFinished, &app,
                         [&counters]() { ++counters.loadingFinishedSignals; });
        QObject::connect(&db, &CardDatabase::cardDatabaseReset, &app,
                         [&counters]() { ++counters.databaseResetSignals; });

        CardDatabaseModel model(&db, false);
        QObject::connect(&model, &QAbstractItemModel::rowsInserted, &app,
                         [&counters](const QModelIndex &, int, int) { ++counters.modelRowsInserted; });
        QObject::connect(&model, &QAbstractItemModel::rowsRemoved, &app,
                         [&counters](const QModelIndex &, int, int) { ++counters.modelRowsRemoved; });
        QObject::connect(&model, &QAbstractItemModel::modelReset, &app, [&counters]() { ++counters.modelResets; });

        return runLoad(&db, counters);
    };

    // ---- Scenario 1: Cold load (XML -> write cache) ----
    printResult("Scenario 1: Cold load (XML -> cache)", runScenario(true));

    // ---- Scenario 2: Warm load (read cache) ----
    printResult("Scenario 2: Warm load (read cache)", runScenario(false));

    // Teardown
    deleteCache(pathProvider.cardDb);

    qInfo() << "----------------------------------------";
    qInfo() << "Done.";

    return 0;
}
