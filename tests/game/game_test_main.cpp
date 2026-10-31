/**
 * @file game_test_main.cpp
 *
 * Test entry point for the headless game-logic suite. The game library has no
 * graphics dependency, so a QCoreApplication is enough; the only process-wide
 * setup left is the card database, which the game logic still reaches through
 * CardDatabaseManager and which therefore has to exist before the first zone is
 * constructed.
 */

#include "gtest/gtest.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QLoggingCategory>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <cstdio>
#include <cstdlib>
#include <libcockatrice/card/database/card_database.h>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/interfaces/interface_card_database_path_provider.h>

namespace
{

class GameTestPathProvider : public ICardDatabasePathProvider
{
public:
    explicit GameTestPathProvider(QString baseDir) : baseDir_(std::move(baseDir))
    {
    }

    QString getCardDatabasePath() const override
    {
        return baseDir_ + QStringLiteral("/cards.xml");
    }

    QString getCustomCardDatabasePath() const override
    {
        return baseDir_ + QStringLiteral("/customsets/");
    }

    QString getTokenDatabasePath() const override
    {
        return baseDir_ + QStringLiteral("/tokens.xml");
    }

    QString getSpoilerCardDatabasePath() const override
    {
        return baseDir_ + QStringLiteral("/spoilers.xml");
    }

private:
    QString baseDir_;
};

bool copyFixture(const QString &fileName, const QString &targetDir)
{
    const QString source = QString::fromUtf8(GAME_TEST_DATA_DIR) + QLatin1Char('/') + fileName;
    const QString target = targetDir + QLatin1Char('/') + fileName;
    if (QFile::copy(source, target)) {
        return true;
    }

    std::fprintf(stderr, "Failed to copy test fixture %s to %s\n", qPrintable(source), qPrintable(target));
    return false;
}

} // namespace

int main(int argc, char **argv)
{
    QStandardPaths::setTestModeEnabled(true);

    QTemporaryDir home;
    if (!home.isValid()) {
        std::fprintf(stderr, "Failed to create a temporary home directory\n");
        return EXIT_FAILURE;
    }
    qputenv("HOME", home.path().toLocal8Bit());

    QCoreApplication app(argc, argv);
    QLoggingCategory::setFilterRules(QStringLiteral("card_database.loading=false\n"
                                                    "card_database.loading.success_or_failure=false\n"
                                                    "settings_cache.*=false"));

    // Copy the fixtures so the database loader never writes its .cache file into
    // the source tree, then point the singleton at the copy.
    QTemporaryDir dataDir;
    if (!dataDir.isValid() || !QDir().mkpath(dataDir.filePath(QStringLiteral("customsets")))) {
        std::fprintf(stderr, "Failed to create a temporary card database directory\n");
        return EXIT_FAILURE;
    }
    if (!copyFixture(QStringLiteral("cards.xml"), dataDir.path()) ||
        !copyFixture(QStringLiteral("tokens.xml"), dataDir.path()) ||
        !copyFixture(QStringLiteral("spoilers.xml"), dataDir.path())) {
        return EXIT_FAILURE;
    }

    // Function-local static: constructed before the CardDatabase singleton below,
    // so it is destroyed after it and the raw pointer stored in the manager stays
    // valid for the database's lifetime.
    static GameTestPathProvider pathProvider(dataDir.path());
    CardDatabaseManager::setCardDatabasePathProvider(&pathProvider);

    CardDatabase *database = CardDatabaseManager::getInstance();
    database->loadCardDatabases();
    if (database->getLoadStatus() != Ok) {
        std::fprintf(stderr, "Failed to load the game test card database (status %d)\n",
                     static_cast<int>(database->getLoadStatus()));
        return EXIT_FAILURE;
    }

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
