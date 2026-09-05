#include "../../oracle/src/oracleimporter.h"

#include "gtest/gtest.h"
#include <QBuffer>
#include <QCoreApplication>
#include <QDebug>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QUrl>
#include <libcockatrice/interfaces/noop_card_set_priority_controller.h>

#if defined(HAS_LZMA)
#include "../../oracle/src/lzma/decompress.h"
#endif
#if defined(HAS_ZLIB)
#include "../../oracle/src/zip/unzip.h"
#endif
#if defined(Q_OS_MACOS)
#include <mach/mach.h>
#include <sys/resource.h>
#endif

// Helper: build a synthetic MTGJSON-style JSON with the given number of sets and cards per set
static QByteArray buildSyntheticData(int numSets, int cardsPerSet)
{
    QJsonObject dataObj;
    for (int s = 0; s < numSets; ++s) {
        QJsonArray cardsArray;
        for (int c = 0; c < cardsPerSet; ++c) {
            QJsonObject card;
            card["name"] = QString("Card %1").arg(s * cardsPerSet + c);
            card["text"] = "This is a test card with some rules text.";
            card["layout"] = "normal";
            card["manaCost"] = "{W}";
            card["type"] = "Creature — Human";
            card["power"] = "2";
            card["toughness"] = "2";
            card["colors"] = QJsonArray{"W"};
            card["colorIdentity"] = QJsonArray{"W"};
            card["types"] = QJsonArray{"Creature"};
            // Real MTGJSON types: floats and booleans, not strings. This
            // exercises the QVariant coercion in the property reader.
            card["convertedManaCost"] = 1.0;
            card["manaValue"] = 1.0;
            card["isOnlineOnly"] = false;
            card["isRebalanced"] = false;

            QJsonObject legalities;
            legalities["standard"] = "legal";
            legalities["modern"] = "legal";
            legalities["legacy"] = "legal";
            legalities["vintage"] = "legal";
            legalities["commander"] = "legal";
            card["legalities"] = legalities;

            QJsonObject identifiers;
            identifiers["scryfallId"] = QString("id-%1-%2").arg(s).arg(c);
            card["identifiers"] = identifiers;

            // In AllPrintings, number and rarity are flat fields on the card
            // object, exactly as set below.
            card["number"] = QString::number(c + 1);
            card["rarity"] = "common";

            cardsArray.append(card);
        }

        QJsonObject setObj;
        setObj["code"] = QString("T%1").arg(s, 2, 10, QChar('0'));
        setObj["name"] = QString("Test Set %1").arg(s);
        setObj["type"] = "expansion";
        setObj["releaseDate"] = "2024-01-01";
        setObj["cards"] = cardsArray;

        dataObj[QString("T%1").arg(s, 2, 10, QChar('0'))] = setObj;
    }

    QJsonObject root;
    root["data"] = dataObj;
    return QJsonDocument(root).toJson(QJsonDocument::Compact);
}

// ============================================================================
// Import throughput benchmark
// ============================================================================

TEST(OracleBenchmark, ImportThroughput)
{
    static constexpr int numSets = 10;
    static constexpr int cardsPerSet = 500;

    QByteArray data = buildSyntheticData(numSets, cardsPerSet);

    OracleImporter importer;

    // Phase 1: Parse JSON
    QElapsedTimer timer;
    timer.start();
    bool ok = importer.readSetsFromByteArray(data);
    ASSERT_TRUE(ok);
    qint64 parseMs = timer.elapsed();

    // Phase 2: Import cards
    timer.restart();
    int importedSets = importer.startImport();
    qint64 importMs = timer.elapsed();

    int totalImported = 0;
    for (const auto &card : importer.getCardList()) {
        Q_UNUSED(card);
        totalImported++;
    }

    // The fixture generates globally unique card names, so the expected
    // counts are exact: a regression here means cards were dropped.
    ASSERT_EQ(importedSets, numSets);
    ASSERT_EQ(totalImported, numSets * cardsPerSet);
    // Real-data probe: numeric convertedManaCost must be coerced to text
    // (regression for the QJsonValue::toString() reader in #7214).
    auto probeCard = importer.getCardList().value("Card 0");
    ASSERT_FALSE(probeCard.isNull());
    ASSERT_EQ(probeCard->getProperty("cmc"), "1");

    qDebug().noquote()
        << QString("Oracle Import Benchmark: %1 sets, %2 unique cards").arg(importedSets).arg(totalImported);
    qDebug().noquote() << QString("  JSON parse:  %1 ms").arg(parseMs);
    qDebug().noquote() << QString("  Card import: %1 ms").arg(importMs);
    qDebug().noquote() << QString("  Total:       %1 ms").arg(parseMs + importMs);
    if (importMs > 0) {
        qDebug().noquote() << QString("  Throughput:  %1 cards/sec")
                                  .arg(static_cast<double>(totalImported) / importMs * 1000.0, 0, 'f', 0);
    }
}

// ============================================================================
// readSetsFromByteArray benchmark
// ============================================================================

TEST(OracleBenchmark, ParseJsonThroughput)
{
    static constexpr int numSets = 20;
    static constexpr int cardsPerSet = 1000;

    QByteArray data = buildSyntheticData(numSets, cardsPerSet);

    // Run 5 iterations and report average
    static constexpr int iterations = 5;
    qint64 totalMs = 0;

    for (int i = 0; i < iterations; ++i) {
        OracleImporter importer;
        QByteArray source = data;
        QElapsedTimer timer;
        timer.start();
        bool ok = importer.readSetsFromByteArray(std::move(source));
        ASSERT_TRUE(ok);
        totalMs += timer.elapsed();
    }

    qint64 avgMs = totalMs / iterations;
    qDebug().noquote() << QString("Parse Benchmark (%1 iterations): avg %2 ms for %3 sets x %4 cards")
                              .arg(iterations)
                              .arg(avgMs)
                              .arg(numSets)
                              .arg(cardsPerSet);
}

// ============================================================================
// Split card merging benchmark
// ============================================================================

TEST(OracleBenchmark, SplitCardMerging)
{
    static constexpr int numSplitCards = 1000;

    QJsonArray cardsList;
    for (int i = 0; i < numSplitCards; ++i) {
        QJsonObject face1;
        face1["name"] = QString("Fire %1 // Ice %1").arg(i);
        face1["text"] = "Fire side text.";
        face1["layout"] = "split";
        face1["side"] = "a";
        face1["faceName"] = QString("Fire %1").arg(i);
        face1["colors"] = QJsonArray{"R"};
        face1["colorIdentity"] = QJsonArray{"R"};
        face1["types"] = QJsonArray{"Instant"};
        face1["manaCost"] = "{R}";
        face1["legalities"] = QJsonObject{{"standard", "not_legal"}};
        face1["identifiers"] = QJsonObject{{"scryfallId", QString("f-%1").arg(i)}};
        face1["number"] = QString::number(i + 1);
        face1["rarity"] = "uncommon";

        QJsonObject face2;
        face2["name"] = QString("Fire %1 // Ice %1").arg(i);
        face2["text"] = "Ice side text.";
        face2["layout"] = "split";
        face2["side"] = "b";
        face2["faceName"] = QString("Ice %1").arg(i);
        face2["colors"] = QJsonArray{"U"};
        face2["colorIdentity"] = QJsonArray{"U"};
        face2["types"] = QJsonArray{"Instant"};
        face2["manaCost"] = "{U}";
        face2["legalities"] = QJsonObject{{"standard", "not_legal"}};
        face2["identifiers"] = QJsonObject{{"scryfallId", QString("i-%1").arg(i)}};
        face2["number"] = QString::number(i + 1);
        face2["rarity"] = "uncommon";

        cardsList.append(face1);
        cardsList.append(face2);
    }

    NoopCardSetPriorityController controller;
    OracleImporter importer;
    CardSetPtr set = CardSet::newInstance(&controller, "TST", "Split Test");

    QElapsedTimer timer;
    timer.start();
    int count = importer.importCardsFromSet(set, cardsList);
    qint64 ms = timer.elapsed();

    ASSERT_EQ(count, numSplitCards);
    qDebug().noquote() << QString("Split Card Merge Benchmark: %1 cards in %2 ms (%3 cards/sec)")
                              .arg(count)
                              .arg(ms)
                              .arg(ms > 0 ? static_cast<double>(count) / ms * 1000.0 : 0.0, 0, 'f', 0);
}

// ============================================================================
// sortAndReduceColors microbenchmark
// ============================================================================

// We can't call sortAndReduceColors directly (it's static), so we benchmark
// through importCardsFromSet with color properties.

TEST(OracleBenchmark, ImportCardsWithColors)
{
    static constexpr int numCards = 10000;

    NoopCardSetPriorityController controller;
    OracleImporter importer;
    CardSetPtr set = CardSet::newInstance(&controller, "TST", "Color Test");

    QJsonArray cardsList;
    for (int i = 0; i < numCards; ++i) {
        QJsonObject card;
        card["name"] = QString("Color Card %1").arg(i);
        card["text"] = "Rules text.";
        card["layout"] = "normal";
        card["manaCost"] = "{W}";
        card["type"] = "Creature — Human";
        card["types"] = QJsonArray{"Creature"};
        card["colors"] = QJsonArray{"B", "R", "G", "W", "U"};
        card["colorIdentity"] = QJsonArray{"B", "R", "G", "W", "U"};
        card["number"] = QString::number(i + 1);
        card["rarity"] = "common";
        card["legalities"] = QJsonObject{{"standard", "legal"}};
        card["identifiers"] = QJsonObject{{"scryfallId", QString("c-%1").arg(i)}};
        cardsList.append(card);
    }

    QElapsedTimer timer;
    timer.start();
    int count = importer.importCardsFromSet(set, cardsList);
    qint64 ms = timer.elapsed();

    ASSERT_EQ(count, numCards);
    qDebug().noquote() << QString("Import with Colors Benchmark: %1 cards in %2 ms (%3 cards/sec)")
                              .arg(count)
                              .arg(ms)
                              .arg(ms > 0 ? static_cast<double>(count) / ms * 1000.0 : 0.0, 0, 'f', 0);
}

// ============================================================================
// RAM usage measurement
// ============================================================================

// Mirrors the default AllPrintings URL selection in oracle/src/pages.cpp.
#if defined(HAS_LZMA)
static const QUrl kDefaultAllPrintingsUrl("https://www.mtgjson.com/api/v5/AllPrintings.json.xz");
#elif defined(HAS_ZLIB)
static const QUrl kDefaultAllPrintingsUrl("https://www.mtgjson.com/api/v5/AllPrintings.json.zip");
#else
static const QUrl kDefaultAllPrintingsUrl("https://www.mtgjson.com/api/v5/AllPrintings.json");
#endif

// Magic bytes also from oracle/src/pages.cpp
static const QByteArray kXzSignature("\xFD\x37\x7A\x58\x5A", 6);
static const QByteArray kZipSignature("PK");

struct MemorySnapshot
{
    qint64 peakRssKb = -1; // process high-water mark (VmHWM on Linux, ru_maxrss on macOS)
    qint64 rssKb = -1;     // current resident set size
    bool available = false;

    static MemorySnapshot current()
    {
        MemorySnapshot snap;
#if defined(Q_OS_LINUX)
        QFile statusFile("/proc/self/status");
        if (statusFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            // /proc files report size() == 0, so atEnd() is immediately true: read everything first.
            const QList<QByteArray> lines = statusFile.readAll().split('\n');
            for (const QByteArray &line : lines) {
                if (line.startsWith("VmHWM:")) {
                    snap.peakRssKb = line.mid(6).trimmed().split(' ').value(0).toLongLong();
                } else if (line.startsWith("VmRSS:")) {
                    snap.rssKb = line.mid(6).trimmed().split(' ').value(0).toLongLong();
                }
            }
            snap.available = snap.peakRssKb >= 0;
        }
#elif defined(Q_OS_MACOS)
        struct rusage usage;
        if (getrusage(RUSAGE_SELF, &usage) == 0) {
            snap.peakRssKb = usage.ru_maxrss / 1024; // bytes -> kB
            snap.available = snap.peakRssKb >= 0;
        }
        // getrusage has no current-RSS equivalent; task_info's resident_size
        // is the closest macOS analog to Linux VmRSS.
        mach_task_basic_info info = {};
        mach_msg_type_number_t count = MACH_TASK_BASIC_INFO_COUNT;
        if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO, reinterpret_cast<task_info_t>(&info), &count) ==
            KERN_SUCCESS) {
            snap.rssKb = info.resident_size / 1024;
        }
#endif
        return snap;
    }
};

static QString formatKb(qint64 kb)
{
    if (kb < 0) {
        return "N/A";
    }
    return QString("%1 MB").arg(kb / 1024.0, 0, 'f', 1);
}

static void logRamPhase(const QString &phase, const MemorySnapshot &baseline, const MemorySnapshot &current)
{
    if (!baseline.available || !current.available) {
        qDebug().noquote() << QString("  %1: memory stats unavailable on this platform").arg(phase);
        return;
    }
    // VmHWM / ru_maxrss are monotonically non-decreasing high-water marks, so a
    // peak-based delta between phases is ~0.0 MB by construction once the
    // fixture build has set the process peak. The live signals are current RSS
    // and the process peak; the delta is meaningful only where the baseline was
    // taken immediately before the phase it measures (e.g. the import phase,
    // which compares afterParse against afterImport).
    QString rssDelta = "N/A";
    if (current.rssKb >= 0 && baseline.rssKb >= 0) {
        rssDelta = formatKb(current.rssKb - baseline.rssKb);
    }
    qDebug().noquote() << QString("  %1: current RSS %2 | delta vs baseline %3 | process peak %4")
                              .arg(phase)
                              .arg(formatKb(current.rssKb))
                              .arg(rssDelta)
                              .arg(formatKb(current.peakRssKb));
}

// Decompresses the download payload when the default URL is a compressed build,
// mirroring the wizard's magic-byte handling in oracle/src/pages.cpp.
static QByteArray decompressSetsData(const QByteArray &payload)
{
    if (payload.startsWith(kXzSignature)) {
#if defined(HAS_LZMA)
        QBuffer inBuffer(const_cast<QByteArray *>(&payload));
        QByteArray out;
        QBuffer outBuffer(&out);
        inBuffer.open(QIODevice::ReadOnly);
        outBuffer.open(QIODevice::WriteOnly);
        XzDecompressor xz;
        if (!xz.decompress(&inBuffer, &outBuffer)) {
            qDebug() << "RAM benchmark: xz decompression failed";
            return {};
        }
        return out;
#else
        qDebug() << "RAM benchmark: download is xz-compressed but this build has no LZMA support";
        return {};
#endif
    }
    if (payload.startsWith(kZipSignature)) {
#if defined(HAS_ZLIB)
        QBuffer inBuffer(const_cast<QByteArray *>(&payload));
        inBuffer.open(QIODevice::ReadOnly);
        UnZip unzip;
        if (unzip.openArchive(&inBuffer) != UnZip::Ok) {
            qDebug() << "RAM benchmark: zip archive open failed";
            return {};
        }
        if (unzip.fileList().size() != 1) {
            qDebug() << "RAM benchmark: zip archive doesn't contain exactly one file";
            return {};
        }
        QByteArray out;
        QBuffer outBuffer(&out);
        outBuffer.open(QIODevice::WriteOnly);
        const auto errorCode = unzip.extractFile(unzip.fileList().value(0), &outBuffer);
        unzip.closeArchive();
        if (errorCode != UnZip::Ok) {
            qDebug() << "RAM benchmark: zip extraction failed";
            return {};
        }
        return out;
#else
        qDebug() << "RAM benchmark: download is zip-compressed but this build has no zlib support";
        return {};
#endif
    }
    return payload;
}

TEST(OracleBenchmark, ImportRamUsage)
{
    static constexpr int numSets = 30;
    static constexpr int cardsPerSet = 2000; // ~60k cards, roughly AllPrintings scale

    // Baseline must precede the fixture build: a high-water mark set while
    // generating the synthetic JSON would otherwise mask the importer phases.
    // Where memory stats are unavailable (Windows), skip before doing the
    // 60k-card fixture build, which would otherwise be pure wasted work.
    const MemorySnapshot baseline = MemorySnapshot::current();
    if (!baseline.available) {
        GTEST_SKIP() << "Memory stats unavailable on this platform";
    }

    const QByteArray data = buildSyntheticData(numSets, cardsPerSet);

    // The fixture build leaves freed-but-unreturned arenas behind (current RSS
    // rarely falls once glibc allocates). Baseline immediately after it so the
    // parse phase measures only the importer's own growth (~40 MB) rather than
    // swallowing the fixture builder's spike.
    const MemorySnapshot afterFixture = MemorySnapshot::current();
    logRamPhase("fixture build", baseline, afterFixture);

    NoopCardSetPriorityController controller;
    OracleImporter importer;

    QElapsedTimer timer;
    timer.start();
    ASSERT_TRUE(importer.readSetsFromByteArray(std::move(data)));
    const qint64 parseMs = timer.elapsed();
    const MemorySnapshot afterParse = MemorySnapshot::current();

    timer.restart();
    const int importedSets = importer.startImport();
    const qint64 importMs = timer.elapsed();
    const MemorySnapshot afterImport = MemorySnapshot::current();

    importer.releaseSetData();
    const MemorySnapshot afterRelease = MemorySnapshot::current();

    const int totalCards = importer.getCardList().size();
    qDebug().noquote() << QString("Oracle RAM Benchmark (synthetic): %1 sets, %2 cards, %3 MB JSON")
                              .arg(importedSets)
                              .arg(totalCards)
                              .arg(data.size() / (1024.0 * 1024.0), 0, 'f', 1);
    qDebug().noquote() << QString("  JSON parse: %1 ms").arg(parseMs);
    qDebug().noquote() << QString("  Card import: %1 ms").arg(importMs);
    logRamPhase("parse", afterFixture, afterParse);
    logRamPhase("import", afterParse, afterImport);
    logRamPhase("after releaseSetData()", afterImport, afterRelease);

    // Freeing the parsed tree rarely moves current RSS (allocator reuse), so the
    // meaningful signal that release actually dropped the buffers is emptiness,
    // not an RSS delta.
    ASSERT_TRUE(importer.getSets().isEmpty());
}

TEST(OracleBenchmark, ImportRamUsageAllPrintings)
{
    // Only "1" enables the download: unset (the default and the CI setup) and
    // an explicit "0" both disable it.
    bool envOk = false;
    const int enabled = qEnvironmentVariableIntValue("COCKATRICE_ORACLE_RAM_BENCHMARK", &envOk);
    if (!envOk || enabled == 0) {
        GTEST_SKIP() << "Set COCKATRICE_ORACLE_RAM_BENCHMARK=1 to download the real AllPrintings dataset for this "
                        "RAM benchmark. Default URL: "
                     << kDefaultAllPrintingsUrl.toDisplayString().toStdString();
    }

    // Baseline must precede the request so the phase covers the download +
    // decompress step, including the payload materialized by readAll().
    const MemorySnapshot baseline = MemorySnapshot::current();
    if (!baseline.available) {
        GTEST_SKIP() << "Memory stats unavailable on this platform";
    }

    QNetworkAccessManager nam;
    QNetworkRequest request(kDefaultAllPrintingsUrl);
    request.setHeader(QNetworkRequest::UserAgentHeader, "Cockatrice Oracle RAM benchmark");
    QNetworkReply *reply = nam.get(request);

    QEventLoop loop;
    QTimer timeoutTimer;
    timeoutTimer.setSingleShot(true);
    bool timedOut = false;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(&timeoutTimer, &QTimer::timeout, &loop, [&] {
        timedOut = true;
        reply->abort();
    });
    timeoutTimer.start(10 * 60 * 1000);
    loop.exec();
    timeoutTimer.stop();

    // abort() leaves reply->error() as OperationCanceledError, so a timed-out
    // download takes the same GTEST_SKIP path as any other network error
    // instead of reading a truncated body and failing the parse below.
    if (timedOut || reply->error() != QNetworkReply::NoError) {
        GTEST_SKIP() << "Download failed: " << reply->errorString().toStdString();
    }
    const QByteArray payload = reply->readAll();
    reply->deleteLater();

    // mtgjson can answer 200 with an HTML page (mirrors the wizard's '<' check
    // in pages.cpp); reject it before trying to decompress/parse.
    if (payload.startsWith("<")) {
        GTEST_SKIP() << "Download returned a non-JSON body (HTML page instead of data), skipping";
    }

    const QByteArray setsData = decompressSetsData(payload);
    const MemorySnapshot afterDownload = MemorySnapshot::current();
    if (setsData.isEmpty()) {
        GTEST_SKIP() << "No data to import (download or decompression failed)";
    }

    NoopCardSetPriorityController controller;
    OracleImporter importer;

    QElapsedTimer timer;
    timer.start();
    ASSERT_TRUE(importer.readSetsFromByteArray(std::move(setsData)));
    const qint64 parseMs = timer.elapsed();
    const MemorySnapshot afterParse = MemorySnapshot::current();

    timer.restart();
    const int importedSets = importer.startImport();
    const qint64 importMs = timer.elapsed();
    const MemorySnapshot afterImport = MemorySnapshot::current();

    importer.releaseSetData();
    const MemorySnapshot afterRelease = MemorySnapshot::current();

    const int totalCards = importer.getCardList().size();
    qDebug().noquote() << QString("Oracle RAM Benchmark (real AllPrintings): %1 sets, %2 unique cards")
                              .arg(importedSets)
                              .arg(totalCards);
    qDebug().noquote() << QString("  URL: %1").arg(kDefaultAllPrintingsUrl.toDisplayString());
    qDebug().noquote() << QString("  Downloaded: %1 MB, decompressed: %2 MB")
                              .arg(payload.size() / (1024.0 * 1024.0), 0, 'f', 1)
                              .arg(setsData.size() / (1024.0 * 1024.0), 0, 'f', 1);
    qDebug().noquote() << QString("  JSON parse: %1 ms").arg(parseMs);
    qDebug().noquote() << QString("  Card import: %1 ms").arg(importMs);
    logRamPhase("download+decompress", baseline, afterDownload);
    logRamPhase("parse", afterDownload, afterParse);
    logRamPhase("import", afterParse, afterImport);
    logRamPhase("after releaseSetData()", afterImport, afterRelease);
}

int main(int argc, char **argv)
{
    // Required for the event loop used by the real-AllPrintings download benchmark
    QCoreApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
