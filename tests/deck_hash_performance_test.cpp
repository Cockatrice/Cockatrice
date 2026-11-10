#include "gtest/gtest.h"
#include <QChar>
#include <QCryptographicHash>
#include <QDebug>
#include <QElapsedTimer>
#include <QFile>
#include <QList>
#include <QString>
#include <QStringList>
#include <libcockatrice/deck_list/deck_list.h>
#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/opensslv.h>
#include <string>
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
#include <openssl/provider.h>
#endif

static constexpr int amount = 1e5;
QString repeatDeck;
QString numberDeck;
QString uniquesDeck;
QString uniquesXorDeck;
QString duplicatesDeck;

TEST(DeckHashTest, RepeatTest)
{
    DeckList decklist(repeatDeck);
    QElapsedTimer timer;
    timer.start();
    for (int i = 0; i < amount; ++i) {
        decklist.getDeckHash();
        decklist.refreshDeckHash();
        if (i != 0 && i % 10000 == 0) {
            qWarning("RepeatTest: iterations=%d chunk=%lldms", i, timer.restart());
        }
    }
    auto hash = decklist.getDeckHash().toStdString();
    ASSERT_EQ(hash, "5cac19qm") << "The hash does not match!";
}

TEST(DeckHashTest, Sha1Baseline)
{
    // Bounded by wall clock, so the number survives even when a single hash takes milliseconds.
    const QByteArray input = QByteArrayLiteral("mountain;island;island;SB:forest;forest;forest");
    QElapsedTimer timer;
    timer.start();
    qint64 n = 0;
    while (timer.elapsed() < 2000) {
        (void)QCryptographicHash::hash(input, QCryptographicHash::Sha1);
        ++n;
    }
    FAIL() << "Sha1Baseline: fresh instance x" << n << " in 2000ms = " << (2000000000LL / n) / 1000 << "us each";
}

TEST(DeckHashTest, Sha1Reuse)
{
    const QByteArray input = QByteArrayLiteral("mountain;island;island;SB:forest;forest;forest");
    QCryptographicHash hasher(QCryptographicHash::Sha1);
    QElapsedTimer timer;
    timer.start();
    int sink = 0;
    for (int i = 0; i < amount; ++i) {
        hasher.addData(input);
        sink += hasher.result().size();
        hasher.reset();
        if (i != 0 && i % 10000 == 0) {
            qWarning("Sha1Reuse: hashes=%d chunk=%lldms", i, timer.restart());
        }
    }
    qWarning("Sha1Reuse: total=%d sink=%d", amount, sink);
}

TEST(DeckHashTest, QtCtorBreakdown)
{
    // Time-budgeted rather than a fixed count: a single construction costs milliseconds on
    // the Arch image, and a CTest timeout swallows whatever the test printed.
    QString report = QStringLiteral("QtCtorBreakdown: qt=%1").arg(QLatin1String(qVersion()));

    auto timeOps = [&report](const char *label, auto &&fn) {
        QElapsedTimer timer;
        timer.start();
        qint64 n = 0;
        while (timer.elapsed() < 500) {
            fn();
            ++n;
        }
        const qint64 us = n ? timer.nsecsElapsed() / 1000 / n : -1;
        report += QStringLiteral(" %1=%2us(n=%3)").arg(QLatin1String(label)).arg(us).arg(n);
    };

    QElapsedTimer first;
    first.start();
    {
        QCryptographicHash hasher(QCryptographicHash::Sha1);
        (void)hasher;
    }
    report += QStringLiteral(" firstCtor=%1us").arg(first.nsecsElapsed() / 1000);

    timeOps("ctor", [] {
        QCryptographicHash hasher(QCryptographicHash::Sha1);
        (void)hasher;
    });
    timeOps("ctorHash", [] {
        QCryptographicHash hasher(QCryptographicHash::Sha1);
        hasher.addData(QByteArrayLiteral("mountain"));
        (void)hasher.result();
    });
    timeOps("fetch", [] {
        EVP_MD *md = EVP_MD_fetch(nullptr, "SHA1", "-fips");
        EVP_MD_free(md);
    });
    timeOps("ctx", [] {
        EVP_MD_CTX *ctx = EVP_MD_CTX_new();
        EVP_MD_CTX_free(ctx);
    });
    EVP_MD *md = EVP_MD_fetch(nullptr, "SHA1", "-fips");
    timeOps("init", [md] {
        EVP_MD_CTX *ctx = EVP_MD_CTX_new();
        EVP_DigestInit_ex(ctx, md, nullptr);
        EVP_MD_CTX_free(ctx);
    });
    EVP_MD_free(md);
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
    timeOps("libctx", [] {
        OSSL_LIB_CTX *ctx = OSSL_LIB_CTX_new();
        OSSL_PROVIDER_load(ctx, "default");
        OSSL_LIB_CTX_free(ctx);
    });
#endif

#ifdef Q_OS_LINUX
    QString crypto;
    QFile maps(QStringLiteral("/proc/self/maps"));
    if (maps.open(QIODevice::ReadOnly)) {
        const QStringList lines = QString::fromLatin1(maps.readAll()).split(QLatin1Char('\n'));
        for (const QString &line : lines) {
            if (line.contains(QLatin1String("libcrypto")) || line.contains(QLatin1String("libssl"))) {
                crypto += QLatin1Char('\n') + line;
            }
        }
    }
#else
    const QString crypto = QStringLiteral("(not linux)");
#endif

    FAIL() << report.toStdString() << "\nLoaded openssl:" << crypto.toStdString();
}

TEST(DeckHashTest, OpenSslInfo)
{
    qWarning("OpenSslInfo: runtime=%s num=%08lx", OpenSSL_version(OPENSSL_VERSION),
             static_cast<unsigned long>(OpenSSL_version_num()));
    qWarning("OpenSslInfo: OPENSSL_CONF=%s OPENSSL_MODULES=%s", qgetenv("OPENSSL_CONF").constData(),
             qgetenv("OPENSSL_MODULES").constData());
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
    qWarning("OpenSslInfo: provider default=%d fips=%d legacy=%d", OSSL_PROVIDER_available(nullptr, "default"),
             OSSL_PROVIDER_available(nullptr, "fips"), OSSL_PROVIDER_available(nullptr, "legacy"));
#endif
    EVP_MD *plain = EVP_MD_fetch(nullptr, "SHA1", nullptr);
    EVP_MD *fips = EVP_MD_fetch(nullptr, "SHA1", "-fips");
    qWarning("OpenSslInfo: EVP_MD_fetch SHA1=%s SHA1/-fips=%s", plain ? "ok" : "null", fips ? "ok" : "null");
    EVP_MD_free(plain);
    EVP_MD_free(fips);
}

TEST(DeckHashTest, EvpFetchBaseline)
{
    const QByteArray input = QByteArrayLiteral("mountain;island;island;SB:forest;forest;forest");
    QElapsedTimer timer;
    int ok = 0;

    timer.start();
    for (int i = 0; i < 1000; ++i) {
        EVP_MD *md = EVP_MD_fetch(nullptr, "SHA1", "-fips");
        ok += md ? 1 : 0;
        EVP_MD_free(md);
    }
    qWarning("EvpFetchBaseline: fetch x1000 elapsed=%lldms ok=%d", timer.elapsed(), ok);

    timer.restart();
    ok = 0;
    for (int i = 0; i < 1000; ++i) {
        EVP_MD *md = EVP_MD_fetch(nullptr, "SHA1", "-fips");
        EVP_MD_CTX *ctx = EVP_MD_CTX_new();
        unsigned char digest[EVP_MAX_MD_SIZE];
        unsigned int len = 0;
        if (md && ctx && EVP_DigestInit_ex(ctx, md, nullptr) == 1 &&
            EVP_DigestUpdate(ctx, input.constData(), size_t(input.size())) == 1 &&
            EVP_DigestFinal_ex(ctx, digest, &len) == 1) {
            ok += static_cast<int>(len);
        }
        EVP_MD_CTX_free(ctx);
        EVP_MD_free(md);
    }
    qWarning("EvpFetchBaseline: fetch+init+digest x1000 elapsed=%lldms ok=%d", timer.elapsed(), ok);
}

TEST(DeckHashTest, NumberTest)
{
    DeckList decklist(numberDeck);
    auto hash = decklist.getDeckHash().toStdString();
    ASSERT_EQ(hash, "e0m38p19") << "The hash does not match!";
}

TEST(DeckHashTest, UniquesTest)
{
    DeckList decklist(uniquesDeck);
    auto hash = decklist.getDeckHash().toStdString();
    ASSERT_EQ(hash, "88prk025") << "The hash does not match!";
}

TEST(DeckHashTest, UniquesTestXor)
{
    DeckList decklist(uniquesXorDeck);
    auto hash = decklist.getDeckHash().toStdString();
    ASSERT_EQ(hash, "hkn6q4pf") << "The hash does not match!";
}

TEST(DeckHashTest, DuplicatesTest)
{
    DeckList decklist(duplicatesDeck);
    auto hash = decklist.getDeckHash().toStdString();
    ASSERT_EQ(hash, "ekt6tg1h") << "The hash does not match!";
}

int main(int argc, char **argv)
{
    const QString deckStart =
        R"(<?xml version="1.0"?><cockatrice_deck version="1"><deckname></deckname><comments></comments><zone name="main">)";
    const QString deckEnd = R"(</zone></cockatrice_deck>)";

    repeatDeck =
        deckStart +
        R"(<card number="1" name="Mountain"/><card number="2" name="Island"/></zone><zone name="side"><card number="3" name="Forest"/>)" +
        deckEnd;
    numberDeck = deckStart + QString(R"(<card number="%1" name="Island"/>)").arg(amount) + deckEnd;

    QStringList deckString{deckStart};
    QStringList deckStringXor = deckString;
    int len = QString::number(amount).length();
    for (int i = 0; i < amount; ++i) {
        // creates already sorted list
        deckString << R"(<card number="1" name="card )" << QString::number(i).rightJustified(len, '0') << R"("/>)";
        // xor in order to mess with sorting
        deckStringXor << R"(<card number="1" name="card )" << QString::number(i ^ amount) << R"("/>)";
    }
    deckString << deckEnd;
    deckStringXor << deckEnd;
    uniquesDeck = deckString.join("");
    uniquesXorDeck = deckStringXor.join("");

    duplicatesDeck = deckStart + QString(R"(<card number="1" name="card"/>)").repeated(amount) + deckEnd;

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
