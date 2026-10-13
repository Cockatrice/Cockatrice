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
