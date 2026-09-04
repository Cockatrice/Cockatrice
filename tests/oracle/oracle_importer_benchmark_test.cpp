#include "../../oracle/src/oracleimporter.h"

#include "gtest/gtest.h"
#include <QDebug>
#include <QElapsedTimer>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <libcockatrice/interfaces/noop_card_set_priority_controller.h>

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
        QElapsedTimer timer;
        timer.start();
        bool ok = importer.readSetsFromByteArray(data);
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

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
