/*
 * Picture loader benchmark / regression suite against the real card image hosts.
 *
 * Deliberately not registered with ctest: it hits live Scryfall / Gatherer
 * endpoints at ~10 requests per second and takes minutes. Run it by hand.
 *
 *   picture_loader_benchmark_test --carddb /path/to/cards.xml [options]
 *
 * Modes
 * -----
 * default : for every URL template in the configured download list (or for each
 *           --url given), load #count pictures twice: once cold (network) and
 *           once cached (served from the QNetworkDiskCache). Both passes must
 *           load every card with zero failures. The cached pass must complete
 *           well under the cold time, which is the regression gate for serving
 *           cached pictures instead of re-fetching them. The cold pass must stay
 *           above a pacing lower bound, the regression gate for burst-free
 *           request throttling.
 * --stress: two CardPictureLoaderWorker instances loading the same cards
 *           concurrently against one host (~20 req/s aggregate), which forces
 *           real 429 responses. Both workers must still complete 100% of their
 *           cards via the shared backoff logic.
 */

#include "client/settings/cache_settings.h"
#include "interface/card_picture_loader/card_picture_loader_worker.h"
#include "interface/card_picture_loader/card_picture_to_load.h"

#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QFile>
#include <QImage>
#include <QList>
#include <QMessageLogContext>
#include <QMetaType>
#include <QMutex>
#include <QTemporaryDir>
#include <QTimer>
#include <QUrl>
#include <cstdio>
#include <libcockatrice/card/database/card_database.h>
#include <libcockatrice/card/printing/exact_card.h>
#include <libcockatrice/card/printing/printing_info.h>
#include <libcockatrice/interfaces/interface_card_database_path_provider.h>
#include <libcockatrice/interfaces/noop_card_preference_provider.h>
#include <libcockatrice/interfaces/noop_card_set_priority_controller.h>
#include <libcockatrice/settings/download_settings.h>
#include <optional>

class BenchmarkCardDatabasePathProvider : public ICardDatabasePathProvider
{
public:
    BenchmarkCardDatabasePathProvider(QString _cardsXml, QString _customSetsDir)
        : cardsXml(std::move(_cardsXml)), customSetsDir(std::move(_customSetsDir))
    {
    }

    QString getCardDatabasePath() const override
    {
        return cardsXml;
    }

    QString getCustomCardDatabasePath() const override
    {
        return customSetsDir;
    }

    QString getTokenDatabasePath() const override
    {
        return QString();
    }

    QString getSpoilerCardDatabasePath() const override
    {
        return QString();
    }

private:
    QString cardsXml;
    QString customSetsDir;
};

struct PassResult
{
    int enqueued = 0;
    int finished = 0;
    int failed = 0;
    qint64 elapsedMs = 0;
    QStringList failedCards;
};

static QList<ExactCard> selectCardsForTemplate(CardDatabase &db, const QString &urlTemplate, int maxCards)
{
    QList<ExactCard> selected;
    const QList<CardInfoPtr> cards = db.getCardList().values();
    for (const CardInfoPtr &card : cards) {
        if (selected.size() >= maxCards) {
            break;
        }
        const SetToPrintingsMap &sets = card->getSets();
        if (sets.isEmpty()) {
            continue;
        }
        const QList<PrintingInfo> printings = sets.first();
        if (printings.isEmpty()) {
            continue;
        }
        const ExactCard cardToLoad(card, printings.first());
        if (CardPictureToLoad(cardToLoad).transformUrl(urlTemplate).isEmpty()) {
            continue;
        }
        selected.append(cardToLoad);
    }
    return selected;
}

static PassResult runPass(CardPictureLoaderWorker *worker, const QList<ExactCard> &cards, int timeoutMs)
{
    PassResult result;
    result.enqueued = cards.size();

    QEventLoop loop;
    QTimer watchdog;
    watchdog.setSingleShot(true);
    watchdog.setInterval(timeoutMs);
    QObject::connect(&watchdog, &QTimer::timeout, &loop, &QEventLoop::quit);

    QElapsedTimer clock;
    QObject::connect(worker, &CardPictureLoaderWorker::imageLoaded, &loop,
                     [&](const ExactCard &card, const QImage &image) {
                         ++result.finished;
                         if (image.isNull()) {
                             ++result.failed;
                             if (result.failedCards.size() < 10) {
                                 result.failedCards.append(card.getName());
                             }
                         }
                         if (result.finished >= result.enqueued) {
                             loop.quit();
                         }
                     });

    clock.start();
    for (const ExactCard &card : cards) {
        worker->enqueueImageLoad(card);
    }
    watchdog.start();
    loop.exec();
    result.elapsedMs = clock.elapsed();
    return result;
}

struct StressResult
{
    PassResult a;
    PassResult b;
    int http429Count = 0;
};

struct LogCounters
{
    int http429 = 0;
    QMutex mutex;
};

static LogCounters *s_activeCounters = nullptr;
static QtMessageHandler s_previousMessageHandler = nullptr;

static void stressLogHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    if (s_activeCounters && msg.contains(QStringLiteral("Too many requests from"))) {
        QMutexLocker locker(&s_activeCounters->mutex);
        ++s_activeCounters->http429;
    }
    if (s_previousMessageHandler) {
        s_previousMessageHandler(type, context, msg);
    }
}

static StressResult runStress(CardPictureLoaderWorker *workerA,
                              CardPictureLoaderWorker *workerB,
                              const QList<ExactCard> &cards,
                              int timeoutMs)
{
    StressResult result;
    result.a.enqueued = cards.size();
    result.b.enqueued = cards.size();

    int completed = 0;
    QMutex completedMutex;

    QEventLoop loop;
    QTimer watchdog;
    watchdog.setSingleShot(true);
    watchdog.setInterval(timeoutMs);
    QObject::connect(&watchdog, &QTimer::timeout, &loop, &QEventLoop::quit);

    const auto finishOne = [&](PassResult &pass, const ExactCard &card, const QImage &image) {
        ++pass.finished;
        if (image.isNull()) {
            ++pass.failed;
            if (pass.failedCards.size() < 10) {
                pass.failedCards.append(card.getName());
            }
        }
        QMutexLocker locker(&completedMutex);
        ++completed;
        if (completed >= result.a.enqueued + result.b.enqueued) {
            loop.quit();
        }
    };

    QElapsedTimer clock;
    QObject::connect(workerA, &CardPictureLoaderWorker::imageLoaded, &loop,
                     [&](const ExactCard &card, const QImage &image) { finishOne(result.a, card, image); });
    QObject::connect(workerB, &CardPictureLoaderWorker::imageLoaded, &loop,
                     [&](const ExactCard &card, const QImage &image) { finishOne(result.b, card, image); });

    // Count 429 responses as seen by the shared rate limiter.
    LogCounters counters;
    s_activeCounters = &counters;
    s_previousMessageHandler = qInstallMessageHandler(stressLogHandler);

    clock.start();
    for (const ExactCard &card : cards) {
        workerA->enqueueImageLoad(card);
        workerB->enqueueImageLoad(card);
    }
    watchdog.start();
    loop.exec();
    const qint64 elapsedMs = clock.elapsed();
    result.a.elapsedMs = elapsedMs;
    result.b.elapsedMs = elapsedMs;

    result.http429Count = counters.http429;
    s_activeCounters = nullptr;
    qInstallMessageHandler(s_previousMessageHandler);
    return result;
}

static QString formatDuration(qint64 ms)
{
    return QStringLiteral("%1.%2 s").arg(ms / 1000).arg((ms % 1000) / 100);
}

static bool likelyRedirects(const QString &urlTemplate)
{
    return urlTemplate.contains(QStringLiteral("api.scryfall.com"));
}

static QString hostOf(const QString &urlTemplate)
{
    return QUrl(urlTemplate).host();
}

static void printUsage()
{
    std::printf("usage: picture_loader_benchmark_test --carddb <path> [options]\n"
                "\n"
                "Loads card pictures from the real configured hosts (not a mock server) and\n"
                "verifies the picture loader's pacing / cache 429 behavior.\n"
                "\n"
                "options:\n"
                "  --carddb <path>       cards.xml to load card data from (required)\n"
                "  --count <N>           cards to load per template (default 300)\n"
                "  --url <template>      test only this URL template; repeatable\n"
                "  --stress              run the two-worker concurrency stress instead\n"
                "  --stress-url <t>      template used by --stress (default: scryfall uuid)\n"
                "  --timeout-min <N>     per-pass watchdog in minutes (default auto)\n"
                "  --cache-dir <dir>     reuse this directory as the sandbox root; non-empty\n"
                "                        cache skips the cold pacing lower bound\n"
                "  --help                show this help\n");
}

int main(int argc, char **argv)
{
    QString cardDbArg;
    QStringList explicitUrls;
    int count = 300;
    bool stress = false;
    QString stressUrl(QStringLiteral("https://api.scryfall.com/cards/!set:uuid!?format=image"));
    QString cacheDirArg;
    std::optional<int> timeoutMin;

    QStringList args;
    for (int i = 1; i < argc; ++i) {
        args.append(QString::fromLocal8Bit(argv[i]));
    }
    for (int i = 0; i < args.size(); ++i) {
        const QString &arg = args.at(i);
        const auto value = [&]() -> QString { return i + 1 < args.size() ? args.at(++i) : QString(); };
        if (arg == QLatin1String("--carddb")) {
            cardDbArg = value();
        } else if (arg == QLatin1String("--count")) {
            count = value().toInt();
        } else if (arg == QLatin1String("--url")) {
            explicitUrls.append(value());
        } else if (arg == QLatin1String("--stress")) {
            stress = true;
        } else if (arg == QLatin1String("--stress-url")) {
            stressUrl = value();
        } else if (arg == QLatin1String("--timeout-min")) {
            timeoutMin = value().toInt();
        } else if (arg == QLatin1String("--cache-dir")) {
            cacheDirArg = value();
        } else if (arg == QLatin1String("--help") || arg == QLatin1String("-h")) {
            printUsage();
            return 0;
        } else {
            std::fprintf(stderr, "unknown argument: %s\n", qPrintable(arg));
            printUsage();
            return 2;
        }
    }

    if (cardDbArg.isEmpty()) {
        std::fprintf(stderr, "error: --carddb is required (a cards.xml generated by Oracle)\n");
        printUsage();
        return 2;
    }
    if (count <= 0) {
        std::fprintf(stderr, "error: --count must be positive\n");
        return 2;
    }

    QCoreApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("Cockatrice"));
    app.setOrganizationName(QStringLiteral("Cockatrice"));
    app.setApplicationVersion(QStringLiteral("9.0.0-benchmark"));

    // The ExactCard argument of imageLoaded crosses threads via a queued connection.
    qRegisterMetaType<ExactCard>();

    QTemporaryDir sandbox;
    const QString rootDir = cacheDirArg.isEmpty() ? sandbox.path() : cacheDirArg;
    QDir().mkpath(rootDir);
    QDir().mkpath(rootDir + "/config");
    QDir().mkpath(rootDir + "/data");
    QDir().mkpath(rootDir + "/cache");

#ifdef Q_OS_UNIX
    // Redirect every QStandardPaths lookup (and therefore SettingsCache paths)
    // into the sandbox so the benchmark never touches user config or caches.
    qputenv("XDG_CONFIG_HOME", (rootDir + "/config").toUtf8());
    qputenv("XDG_DATA_HOME", (rootDir + "/data").toUtf8());
    qputenv("XDG_CACHE_HOME", (rootDir + "/cache").toUtf8());
#endif

    const bool warmStart = QDir(rootDir + "/cache/Cockatrice/downloaded")
                               .entryList(QDir::Files | QDir::AllDirs | QDir::NoDotAndDotDot, QDir::Name)
                               .size() > 0;

    // Copied into the sandbox so the loader's binary cache ("cards.xml.cache")
    // is written next to it instead of next to the user's file, and so a
    // --cache-dir rerun can pick it up again.
    const QString dataPath = rootDir + "/data/Cockatrice";
    QDir().mkpath(dataPath);
    const QString cardsXml = dataPath + "/cards.xml";
    if (!QFile::exists(cardsXml)) {
        if (!QFile::copy(cardDbArg, cardsXml)) {
            std::fprintf(stderr, "error: could not copy %s to sandbox\n", qPrintable(cardDbArg));
            return 2;
        }
    }
    const QString customSetsDir = dataPath + "/customsets";
    QDir().mkpath(customSetsDir);

    auto *prefs = new NoopCardPreferenceProvider();
    auto *priorityController = new NoopCardSetPriorityController();
    auto *pathProvider = new BenchmarkCardDatabasePathProvider(cardsXml, customSetsDir);
    CardDatabase db(nullptr, prefs, pathProvider, priorityController);
    db.loadCardDatabases();
    if (db.getLoadStatus() != Ok || db.getCardList().isEmpty()) {
        std::fprintf(stderr, "error: failed to load card database from %s\n", qPrintable(cardsXml));
        return 1;
    }
    const int availableCards = db.getCardList().size();

    const QStringList urlsToTest =
        explicitUrls.isEmpty() ? SettingsCache::instance().downloads().getAllURLs() : explicitUrls;

    if (stress) {
        const QList<ExactCard> cards = selectCardsForTemplate(db, stressUrl, count);
        if (cards.isEmpty()) {
            std::fprintf(stderr, "error: no cards satisfy template %s\n", qPrintable(stressUrl));
            return 1;
        }
        SettingsCache::instance().downloads().setDownloadUrls({stressUrl});

        std::printf("=== STRESS: 2 workers x %d cards, host %s ===\n", static_cast<int>(cards.size()),
                    qPrintable(hostOf(stressUrl)));
        auto *workerA = new CardPictureLoaderWorker();
        auto *workerB = new CardPictureLoaderWorker();
        const int timeoutMs = (timeoutMin.has_value() ? timeoutMin.value() : 15) * 60 * 1000;
        const StressResult result = runStress(workerA, workerB, cards, timeoutMs);

        const bool completeA = result.a.finished >= result.a.enqueued;
        const bool completeB = result.b.finished >= result.b.enqueued;
        const bool zeroFailures = result.a.failed == 0 && result.b.failed == 0;
        const bool pass = completeA && completeB && zeroFailures;

        std::printf("worker A: %d/%d loaded, %d failed (%.2f images/s)\n", result.a.finished, result.a.enqueued,
                    result.a.failed,
                    result.a.elapsedMs > 0
                        ? static_cast<double>(result.a.finished) * 1000.0 / static_cast<double>(result.a.elapsedMs)
                        : 0.0);
        std::printf("worker B: %d/%d loaded, %d failed (%.2f images/s)\n", result.b.finished, result.b.enqueued,
                    result.b.failed,
                    result.b.elapsedMs > 0
                        ? static_cast<double>(result.b.finished) * 1000.0 / static_cast<double>(result.b.elapsedMs)
                        : 0.0);
        std::printf("429 responses observed: %d\n", result.http429Count);
        std::printf("elapsed: %s\n", qPrintable(formatDuration(result.a.elapsedMs)));
        if (result.a.failed > 0) {
            std::printf("  worker A failures: %s\n", qPrintable(result.a.failedCards.join(QStringLiteral(", "))));
        }
        if (result.b.failed > 0) {
            std::printf("  worker B failures: %s\n", qPrintable(result.b.failedCards.join(QStringLiteral(", "))));
        }
        std::printf("RESULT: %s\n", pass ? "PASS" : "FAIL");
        return pass ? 0 : 1;
    }

    bool allPass = true;
    std::printf("=== PICTURE LOADER BENCHMARK (%d cards available, %d per template)%s ===\n", availableCards, count,
                warmStart ? ", WARM cache from previous run" : "");

    auto *worker = new CardPictureLoaderWorker();
    for (const QString &urlTemplate : urlsToTest) {
        const QList<ExactCard> cards = selectCardsForTemplate(db, urlTemplate, count);
        if (cards.isEmpty()) {
            std::printf("SKIP  %-22s %-52s (no cards satisfy the template)\n", qPrintable(hostOf(urlTemplate)),
                        qPrintable(urlTemplate));
            continue;
        }
        SettingsCache::instance().downloads().setDownloadUrls({urlTemplate});

        const bool redirects = likelyRedirects(urlTemplate);
        const qint64 perCardMs = redirects ? 200 : 100;
        const int timeoutMs =
            (timeoutMin.has_value() ? timeoutMin.value() : (cards.size() * perCardMs * 8 + 60000) / 60000) * 60 * 1000;
        const qint64 coldLowerMs = static_cast<qint64>(cards.size()) * perCardMs / 2;
        const qint64 cachedUpperMs = 8000;

        const PassResult cold = runPass(worker, cards, timeoutMs);
        const PassResult cached = runPass(worker, cards, timeoutMs);

        const bool coldComplete = cold.finished >= cold.enqueued;
        const bool coldZeroFailures = cold.failed == 0;
        const bool coldPaced = warmStart || cold.elapsedMs >= coldLowerMs;
        const bool cachedComplete = cached.finished >= cached.enqueued;
        const bool cachedZeroFailures = cached.failed == 0;
        const bool cachedFast = cached.elapsedMs < cachedUpperMs;

        const bool pass =
            coldComplete && coldZeroFailures && coldPaced && cachedComplete && cachedZeroFailures && cachedFast;
        allPass = allPass && pass;

        std::printf("%s  %-22s %-52s cold %s (paced>=%s) cached %s (must be <%.0fs) | %d/%d %d/%d loaded, "
                    "%d/%d failed\n",
                    pass ? "PASS " : "FAIL ", qPrintable(hostOf(urlTemplate)), qPrintable(urlTemplate),
                    qPrintable(formatDuration(cold.elapsedMs)), qPrintable(formatDuration(coldLowerMs)),
                    qPrintable(formatDuration(cached.elapsedMs)), static_cast<double>(cachedUpperMs) / 1000.0,
                    cold.finished, cold.enqueued, cached.finished, cached.enqueued, cold.failed, cached.failed);
        if (cold.failed > 0) {
            std::printf("  cold failures: %s\n", qPrintable(cold.failedCards.join(QStringLiteral(", "))));
        }
        if (cached.failed > 0) {
            std::printf("  cached failures: %s\n", qPrintable(cached.failedCards.join(QStringLiteral(", "))));
        }
    }

    std::printf("cache root: %s%s\n", qPrintable(rootDir),
                cacheDirArg.isEmpty() ? " (reuse with --cache-dir to warm on the next run)" : "");
    std::printf("RESULT: %s\n", allPass ? "PASS" : "FAIL");
    return allPass ? 0 : 1;
}