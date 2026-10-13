#include "gtest/gtest.h"
#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <libcockatrice/utility/user_data_backup.h>

namespace
{

TEST(UserDataBackup, CopyDirectoryKeepsNestedStructure)
{
    QTemporaryDir sourceDir;
    QTemporaryDir destParent;
    ASSERT_TRUE(sourceDir.isValid());
    ASSERT_TRUE(destParent.isValid());
    QDir source(sourceDir.path());
    ASSERT_TRUE(source.mkpath("decks/sub"));
    QFile deck(source.absoluteFilePath("decks/sub/mydeck.cod"));
    ASSERT_TRUE(deck.open(QIODevice::WriteOnly | QIODevice::Text));
    deck.write("<cockatrice_deck version=\"1\"/>\n");
    deck.close();

    QString dest = QDir(destParent.path()).absoluteFilePath("copy");
    EXPECT_TRUE(UserDataBackup::copyDirectoryRecursively(sourceDir.path(), dest));
    EXPECT_TRUE(QFile::exists(dest + "/decks/sub/mydeck.cod"));
}

TEST(UserDataBackup, CopyDirectoryKeepsHiddenFiles)
{
    QTemporaryDir sourceDir;
    QTemporaryDir destParent;
    ASSERT_TRUE(sourceDir.isValid());
    ASSERT_TRUE(destParent.isValid());
    QFile hidden(QDir(sourceDir.path()).absoluteFilePath(".hidden.ini"));
    ASSERT_TRUE(hidden.open(QIODevice::WriteOnly | QIODevice::Text));
    hidden.write("[general]\n");
    hidden.close();

    QString dest = QDir(destParent.path()).absoluteFilePath("copy");
    EXPECT_TRUE(UserDataBackup::copyDirectoryRecursively(sourceDir.path(), dest));
    EXPECT_TRUE(QFile::exists(dest + "/.hidden.ini"));
}

TEST(UserDataBackup, CopyDirectoryOverwritesExistingTarget)
{
    QTemporaryDir sourceDir;
    QTemporaryDir destParent;
    ASSERT_TRUE(sourceDir.isValid());
    ASSERT_TRUE(destParent.isValid());
    const QString sourceFile = QDir(sourceDir.path()).absoluteFilePath("settings.ini");
    QFile first(sourceFile);
    ASSERT_TRUE(first.open(QIODevice::WriteOnly | QIODevice::Text));
    first.write("version=1\n");
    first.close();

    QString dest = QDir(destParent.path()).absoluteFilePath("copy");
    ASSERT_TRUE(UserDataBackup::copyDirectoryRecursively(sourceDir.path(), dest));

    QFile second(sourceFile);
    ASSERT_TRUE(second.open(QIODevice::WriteOnly | QIODevice::Text));
    second.write("version=2\n");
    second.close();

    EXPECT_TRUE(UserDataBackup::copyDirectoryRecursively(sourceDir.path(), dest));
    QFile result(dest + "/settings.ini");
    ASSERT_TRUE(result.open(QIODevice::ReadOnly | QIODevice::Text));
    EXPECT_EQ(result.readAll(), QByteArray("version=2\n"));
}

TEST(UserDataBackup, CopyEmptyDirectorySucceeds)
{
    QTemporaryDir sourceDir;
    QTemporaryDir destParent;
    ASSERT_TRUE(sourceDir.isValid());
    ASSERT_TRUE(destParent.isValid());
    ASSERT_TRUE(QDir(sourceDir.path()).mkpath("empty"));

    QString dest = QDir(destParent.path()).absoluteFilePath("copy");
    EXPECT_TRUE(UserDataBackup::copyDirectoryRecursively(sourceDir.path(), dest));
    EXPECT_TRUE(QDir(dest + "/empty").exists());
}
TEST(UserDataBackup, CopyMissingDirectoryFails)
{
    QTemporaryDir destParent;
    ASSERT_TRUE(destParent.isValid());
    EXPECT_FALSE(
        UserDataBackup::copyDirectoryRecursively("/nonexistent-cockatrice-path-xyz", destParent.path() + "/copy"));
}

TEST(UserDataBackup, BackupDirectoryCreatesTimestampedCopy)
{
    QTemporaryDir sourceDir;
    QTemporaryDir backupRoot;
    ASSERT_TRUE(sourceDir.isValid());
    ASSERT_TRUE(backupRoot.isValid());
    QFile deck(QDir(sourceDir.path()).absoluteFilePath("mydeck.cod"));
    ASSERT_TRUE(deck.open(QIODevice::WriteOnly | QIODevice::Text));
    deck.write("<cockatrice_deck version=\"1\"/>\n");
    deck.close();

    QString backupLocation;
    EXPECT_TRUE(UserDataBackup::backupAtRiskPath(sourceDir.path(), backupRoot.path(), &backupLocation));
    EXPECT_TRUE(QFile::exists(backupLocation + "/mydeck.cod"));
    EXPECT_TRUE(backupLocation.startsWith(QDir(backupRoot.path()).absolutePath()));
}

TEST(UserDataBackup, BackupSingleFile)
{
    QTemporaryDir sourceDir;
    QTemporaryDir backupRoot;
    ASSERT_TRUE(sourceDir.isValid());
    ASSERT_TRUE(backupRoot.isValid());
    QString cardDb = QDir(sourceDir.path()).absoluteFilePath("cards.xml");
    QFile db(cardDb);
    ASSERT_TRUE(db.open(QIODevice::WriteOnly | QIODevice::Text));
    db.write("<cockatrice_card_database/>\n");
    db.close();

    QString backupLocation;
    EXPECT_TRUE(UserDataBackup::backupAtRiskPath(cardDb, backupRoot.path(), &backupLocation));
    EXPECT_TRUE(QFile::exists(backupLocation));
    EXPECT_TRUE(backupLocation.endsWith("cards.xml"));
}

TEST(UserDataBackup, BackupMissingSourceFails)
{
    QTemporaryDir backupRoot;
    ASSERT_TRUE(backupRoot.isValid());
    EXPECT_FALSE(UserDataBackup::backupAtRiskPath("/nonexistent-cockatrice-path-xyz", backupRoot.path(), nullptr));
}

TEST(UserDataBackup, BackupPathsShareSingleSessionDir)
{
    QTemporaryDir sourceDir;
    QTemporaryDir backupRoot;
    ASSERT_TRUE(sourceDir.isValid());
    ASSERT_TRUE(backupRoot.isValid());
    QDir source(sourceDir.path());
    ASSERT_TRUE(source.mkpath("decks"));
    ASSERT_TRUE(source.mkpath("settings"));
    QFile deck(source.absoluteFilePath("decks/mydeck.cod"));
    ASSERT_TRUE(deck.open(QIODevice::WriteOnly | QIODevice::Text));
    deck.write("<cockatrice_deck version=\"1\"/>\n");
    deck.close();
    QFile settings(source.absoluteFilePath("settings/settings.ini"));
    ASSERT_TRUE(settings.open(QIODevice::WriteOnly | QIODevice::Text));
    settings.write("[general]\n");
    settings.close();

    QStringList locations;
    QString sessionDir;
    EXPECT_TRUE(
        UserDataBackup::backupAtRiskPaths({source.absoluteFilePath("decks"), source.absoluteFilePath("settings")},
                                          backupRoot.path(), &locations, &sessionDir));
    ASSERT_EQ(locations.size(), 2);
    // Both backups live in the same timestamped session, even across a second boundary.
    for (const QString &location : locations) {
        EXPECT_TRUE(location.startsWith(QDir(sessionDir).absolutePath()));
    }
    EXPECT_TRUE(QFile::exists(QDir(sessionDir).absoluteFilePath("decks/mydeck.cod")));
    EXPECT_TRUE(QFile::exists(QDir(sessionDir).absoluteFilePath("settings/settings.ini")));
}

TEST(UserDataBackup, BackupPathsWithSameNameDoNotCollide)
{
    QTemporaryDir firstDir;
    QTemporaryDir secondDir;
    QTemporaryDir backupRoot;
    ASSERT_TRUE(firstDir.isValid());
    ASSERT_TRUE(secondDir.isValid());
    ASSERT_TRUE(backupRoot.isValid());
    for (const QString &dir : {firstDir.path(), secondDir.path()}) {
        QFile deck(QDir(dir).absoluteFilePath("cards.xml"));
        ASSERT_TRUE(deck.open(QIODevice::WriteOnly | QIODevice::Text));
        deck.write("<cockatrice_card_database/>\n");
        deck.close();
    }

    QStringList locations;
    EXPECT_TRUE(UserDataBackup::backupAtRiskPaths(
        {QDir(firstDir.path()).absoluteFilePath("cards.xml"), QDir(secondDir.path()).absoluteFilePath("cards.xml")},
        backupRoot.path(), &locations, nullptr));
    ASSERT_EQ(locations.size(), 2);
    EXPECT_NE(locations.at(0), locations.at(1));
    EXPECT_TRUE(QFile::exists(locations.at(0)));
    EXPECT_TRUE(QFile::exists(locations.at(1)));
}

TEST(UserDataBackup, RapidBackupsGetUniqueSessionDirs)
{
    QTemporaryDir sourceDir;
    QTemporaryDir backupRoot;
    ASSERT_TRUE(sourceDir.isValid());
    ASSERT_TRUE(backupRoot.isValid());
    QFile deck(QDir(sourceDir.path()).absoluteFilePath("mydeck.cod"));
    ASSERT_TRUE(deck.open(QIODevice::WriteOnly | QIODevice::Text));
    deck.write("<cockatrice_deck version=\"1\"/>\n");
    deck.close();

    QString firstSession;
    QString secondSession;
    QStringList locations;
    EXPECT_TRUE(UserDataBackup::backupAtRiskPaths({sourceDir.path()}, backupRoot.path(), &locations, &firstSession));
    EXPECT_TRUE(UserDataBackup::backupAtRiskPaths({sourceDir.path()}, backupRoot.path(), &locations, &secondSession));
    EXPECT_NE(firstSession, secondSession);
}

TEST(UserDataBackup, DefaultBackupRootIsUsable)
{
    QString root = UserDataBackup::defaultBackupRoot();
    EXPECT_FALSE(root.isEmpty());
    EXPECT_TRUE(QDir(root).isAbsolute());
}

} // namespace

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
