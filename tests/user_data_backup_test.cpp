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

    QStringList locations;
    QString sessionDir;
    EXPECT_TRUE(UserDataBackup::backupAtRiskPaths({sourceDir.path()}, backupRoot.path(), &locations, &sessionDir));
    ASSERT_EQ(locations.size(), 1);
    EXPECT_TRUE(QFile::exists(locations.first() + "/mydeck.cod"));
    EXPECT_TRUE(locations.first().startsWith(QDir(backupRoot.path()).absolutePath()));
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

    QStringList locations;
    EXPECT_TRUE(UserDataBackup::backupAtRiskPaths({cardDb}, backupRoot.path(), &locations, nullptr));
    ASSERT_EQ(locations.size(), 1);
    EXPECT_TRUE(QFile::exists(locations.first()));
    EXPECT_TRUE(locations.first().endsWith("cards.xml"));
}

TEST(UserDataBackup, BackupMissingSourceFails)
{
    QTemporaryDir backupRoot;
    ASSERT_TRUE(backupRoot.isValid());
    QStringList locations;
    QString sessionDir;
    EXPECT_FALSE(UserDataBackup::backupAtRiskPaths({QDir(backupRoot.path()).absoluteFilePath("does-not-exist.cod")},
                                                   backupRoot.path(), &locations, &sessionDir));
    EXPECT_TRUE(locations.isEmpty());
    EXPECT_TRUE(sessionDir.isEmpty());
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

TEST(UserDataBackup, DefaultBackupRootHasNoDotDotSegments)
{
    QString root = UserDataBackup::defaultBackupRoot();
    EXPECT_FALSE(root.contains(QStringLiteral("..")));
    EXPECT_EQ(root, QDir::cleanPath(root));
}

TEST(UserDataBackup, ListSessionsFindsRecentAndPrunesOld)
{
    QTemporaryDir sourceDir;
    QTemporaryDir backupRoot;
    ASSERT_TRUE(sourceDir.isValid());
    ASSERT_TRUE(backupRoot.isValid());
    QFile deck(QDir(sourceDir.path()).absoluteFilePath("mydeck.cod"));
    ASSERT_TRUE(deck.open(QIODevice::WriteOnly | QIODevice::Text));
    deck.write("<cockatrice_deck version=\"1\"/>\n");
    deck.close();

    QStringList locations;
    QString firstSession;
    QString secondSession;
    ASSERT_TRUE(UserDataBackup::backupAtRiskPaths({sourceDir.path()}, backupRoot.path(), &locations, &firstSession));
    ASSERT_TRUE(UserDataBackup::backupAtRiskPaths({sourceDir.path()}, backupRoot.path(), &locations, &secondSession));

    const QStringList sessions = UserDataBackup::listBackupSessions(backupRoot.path());
    ASSERT_EQ(sessions.size(), 2);
    // Newest first
    EXPECT_EQ(sessions.first(), secondSession);

    const QString recent = UserDataBackup::findRecentUsableBackupSession(backupRoot.path(), {sourceDir.path()}, 24);
    EXPECT_EQ(recent, secondSession);

    const QStringList reusedLocations = UserDataBackup::listSessionBackupLocations(recent);
    EXPECT_FALSE(reusedLocations.isEmpty());
    for (const QString &location : reusedLocations) {
        EXPECT_TRUE(location.startsWith(QDir(recent).absolutePath()));
        EXPECT_TRUE(QFile::exists(location));
    }

    UserDataBackup::pruneOldBackupSessions(backupRoot.path(), 1);
    const QStringList pruned = UserDataBackup::listBackupSessions(backupRoot.path());
    ASSERT_EQ(pruned.size(), 1);
    EXPECT_EQ(pruned.first(), secondSession);
}

TEST(UserDataBackup, FindRecentReturnsEmptyWhenNone)
{
    QTemporaryDir backupRoot;
    ASSERT_TRUE(backupRoot.isValid());
    EXPECT_TRUE(UserDataBackup::listBackupSessions(backupRoot.path()).isEmpty());
    EXPECT_TRUE(UserDataBackup::findRecentUsableBackupSession(backupRoot.path(), {backupRoot.path() + "/missing"}, 24)
                    .isEmpty());
    EXPECT_TRUE(UserDataBackup::findRecentCoveringBackupSession(backupRoot.path(), {backupRoot.path() + "/missing"}, 24)
                    .isEmpty());
    EXPECT_TRUE(UserDataBackup::listSessionBackupLocations(backupRoot.path() + "/missing").isEmpty());
    EXPECT_FALSE(UserDataBackup::isBackupSessionComplete(backupRoot.path() + "/missing"));
    // Pruning an empty root must not fail
    UserDataBackup::pruneOldBackupSessions(backupRoot.path(), 5);
}

TEST(UserDataBackup, AllMissingSourcesCreateNoSession)
{
    QTemporaryDir backupRoot;
    ASSERT_TRUE(backupRoot.isValid());
    QStringList locations;
    QString sessionDir;
    EXPECT_FALSE(UserDataBackup::backupAtRiskPaths({backupRoot.path() + "/nope1", backupRoot.path() + "/nope2"},
                                                   backupRoot.path(), &locations, &sessionDir));
    EXPECT_TRUE(locations.isEmpty());
    EXPECT_TRUE(sessionDir.isEmpty());
    // No empty session left behind to block a retry or evict good sessions
    EXPECT_TRUE(UserDataBackup::listBackupSessions(backupRoot.path()).isEmpty());
}

TEST(UserDataBackup, EmptySessionsAreSkippedAndPruned)
{
    QTemporaryDir backupRoot;
    ASSERT_TRUE(backupRoot.isValid());
    QString emptySession;
    ASSERT_TRUE(UserDataBackup::createBackupSessionDir(backupRoot.path(), &emptySession));
    // An empty session from a failed attempt must never block a retry
    EXPECT_TRUE(UserDataBackup::listSessionBackupLocations(emptySession).isEmpty());
    EXPECT_FALSE(UserDataBackup::isBackupSessionComplete(emptySession));
    EXPECT_TRUE(UserDataBackup::findRecentUsableBackupSession(backupRoot.path(), {backupRoot.path() + "/missing"}, 24)
                    .isEmpty());
    EXPECT_TRUE(UserDataBackup::findRecentCoveringBackupSession(backupRoot.path(), {backupRoot.path() + "/missing"}, 24)
                    .isEmpty());
    UserDataBackup::pruneOldBackupSessions(backupRoot.path(), 5);
    EXPECT_FALSE(QDir(emptySession).exists());
}

TEST(UserDataBackup, SuccessfulBackupIsMarkedComplete)
{
    QTemporaryDir sourceDir;
    QTemporaryDir backupRoot;
    ASSERT_TRUE(sourceDir.isValid());
    ASSERT_TRUE(backupRoot.isValid());
    QFile deck(QDir(sourceDir.path()).absoluteFilePath("mydeck.cod"));
    ASSERT_TRUE(deck.open(QIODevice::WriteOnly | QIODevice::Text));
    deck.write("<cockatrice_deck version=\"1\"/>\n");
    deck.close();

    QStringList locations;
    QString sessionDir;
    EXPECT_TRUE(UserDataBackup::backupAtRiskPaths({sourceDir.path()}, backupRoot.path(), &locations, &sessionDir));
    EXPECT_TRUE(UserDataBackup::isBackupSessionComplete(sessionDir));
    // Marker files are bookkeeping and must not leak into the user-facing locations
    const QStringList listed = UserDataBackup::listSessionBackupLocations(sessionDir);
    EXPECT_FALSE(listed.isEmpty());
    for (const QString &location : listed) {
        EXPECT_FALSE(location.endsWith(QStringLiteral("backup.complete")));
        EXPECT_FALSE(location.endsWith(QStringLiteral("backup.incomplete")));
    }
}

TEST(UserDataBackup, IncompleteMarkerIsNotUsableButRemainsCovering)
{
    QTemporaryDir sourceDir;
    QTemporaryDir backupRoot;
    ASSERT_TRUE(sourceDir.isValid());
    ASSERT_TRUE(backupRoot.isValid());
    QFile deck(QDir(sourceDir.path()).absoluteFilePath("mydeck.cod"));
    ASSERT_TRUE(deck.open(QIODevice::WriteOnly | QIODevice::Text));
    deck.write("<cockatrice_deck version=\"1\"/>\n");
    deck.close();

    QStringList locations;
    QString sessionDir;
    ASSERT_TRUE(UserDataBackup::backupAtRiskPaths({sourceDir.path()}, backupRoot.path(), &locations, &sessionDir));
    ASSERT_TRUE(QFile::remove(QDir(sessionDir).absoluteFilePath("backup.complete")));
    QFile marker(QDir(sessionDir).absoluteFilePath("backup.incomplete"));
    ASSERT_TRUE(marker.open(QIODevice::WriteOnly | QIODevice::Text));
    marker.write("incomplete\n");
    marker.close();

    EXPECT_FALSE(UserDataBackup::isBackupSessionComplete(sessionDir));
    // Incomplete sessions are skipped by the usable search but still found by the
    // covering search, so a failed copy falls back to the salvaged content.
    EXPECT_TRUE(UserDataBackup::findRecentUsableBackupSession(backupRoot.path(), {sourceDir.path()}, 24).isEmpty());
    EXPECT_EQ(UserDataBackup::findRecentCoveringBackupSession(backupRoot.path(), {sourceDir.path()}, 24), sessionDir);
}

TEST(UserDataBackup, SessionWithoutMarkerCountsAsIncomplete)
{
    QTemporaryDir backupRoot;
    ASSERT_TRUE(backupRoot.isValid());
    QString sessionDir;
    ASSERT_TRUE(UserDataBackup::createBackupSessionDir(backupRoot.path(), &sessionDir));
    ASSERT_TRUE(QDir(sessionDir).mkpath("decks"));
    QFile deck(QDir(sessionDir).absoluteFilePath("decks/mydeck.cod"));
    ASSERT_TRUE(deck.open(QIODevice::WriteOnly | QIODevice::Text));
    deck.write("<cockatrice_deck version=\"1\"/>\n");
    deck.close();
    EXPECT_FALSE(UserDataBackup::isBackupSessionComplete(sessionDir));
    // No source manifest either, so neither search reuses it.
    EXPECT_TRUE(UserDataBackup::findRecentUsableBackupSession(backupRoot.path(), {backupRoot.path() + "/missing"}, 24)
                    .isEmpty());
    EXPECT_TRUE(UserDataBackup::findRecentCoveringBackupSession(backupRoot.path(), {backupRoot.path() + "/missing"}, 24)
                    .isEmpty());
}

TEST(UserDataBackup, FindRecentIgnoresExpiredSessions)
{
    QTemporaryDir sourceDir;
    QTemporaryDir backupRoot;
    ASSERT_TRUE(sourceDir.isValid());
    ASSERT_TRUE(backupRoot.isValid());
    QFile deck(QDir(sourceDir.path()).absoluteFilePath("mydeck.cod"));
    ASSERT_TRUE(deck.open(QIODevice::WriteOnly | QIODevice::Text));
    deck.write("<cockatrice_deck version=\"1\"/>\n");
    deck.close();

    QStringList locations;
    QString sessionDir;
    ASSERT_TRUE(UserDataBackup::backupAtRiskPaths({sourceDir.path()}, backupRoot.path(), &locations, &sessionDir));
    EXPECT_TRUE(UserDataBackup::findRecentUsableBackupSession(backupRoot.path(), {sourceDir.path()}, 0).isEmpty());
    EXPECT_TRUE(UserDataBackup::findRecentCoveringBackupSession(backupRoot.path(), {sourceDir.path()}, 0).isEmpty());
}

TEST(UserDataBackup, EstimateBackupSize)
{
    QTemporaryDir sourceDir;
    ASSERT_TRUE(sourceDir.isValid());
    const QString missing = QDir(sourceDir.path()).absoluteFilePath("missing");
    EXPECT_EQ(UserDataBackup::estimateBackupSize({missing}), 0);
    QFile deck(QDir(sourceDir.path()).absoluteFilePath("mydeck.cod"));
    ASSERT_TRUE(deck.open(QIODevice::WriteOnly | QIODevice::Text));
    deck.write("123456");
    deck.close();
    EXPECT_EQ(UserDataBackup::estimateBackupSize({sourceDir.path(), missing}), 6);
    // Early-out: once over the cap the exact total no longer matters
    EXPECT_GE(UserDataBackup::estimateBackupSize({sourceDir.path()}, 1), 1);
}

TEST(UserDataBackup, SessionCoversSourcesRequiresManifestMatch)
{
    QTemporaryDir sourceA;
    QTemporaryDir sourceB;
    QTemporaryDir backupRoot;
    ASSERT_TRUE(sourceA.isValid());
    ASSERT_TRUE(sourceB.isValid());
    ASSERT_TRUE(backupRoot.isValid());
    QFile fa(QDir(sourceA.path()).absoluteFilePath("a.cod"));
    ASSERT_TRUE(fa.open(QIODevice::WriteOnly | QIODevice::Text));
    fa.write("a");
    fa.close();

    QStringList locations;
    QString sessionDir;
    ASSERT_TRUE(UserDataBackup::backupAtRiskPaths({sourceA.path()}, backupRoot.path(), &locations, &sessionDir));
    EXPECT_TRUE(UserDataBackup::sessionCoversSources(sessionDir, {sourceA.path()}));
    EXPECT_FALSE(UserDataBackup::sessionCoversSources(sessionDir, {sourceA.path(), sourceB.path()}));
    EXPECT_FALSE(UserDataBackup::sessionCoversSources(sessionDir, {sourceB.path()}));
}

TEST(UserDataBackup, SessionWithoutManifestIsNotCovering)
{
    QTemporaryDir backupRoot;
    ASSERT_TRUE(backupRoot.isValid());
    QString sessionDir;
    ASSERT_TRUE(UserDataBackup::createBackupSessionDir(backupRoot.path(), &sessionDir));
    ASSERT_TRUE(QDir(sessionDir).mkpath("decks"));
    QFile deck(QDir(sessionDir).absoluteFilePath("decks/mydeck.cod"));
    ASSERT_TRUE(deck.open(QIODevice::WriteOnly | QIODevice::Text));
    deck.write("<cockatrice_deck version=\"1\"/>\n");
    deck.close();
    EXPECT_FALSE(UserDataBackup::sessionCoversSources(sessionDir, {QDir(sessionDir).absoluteFilePath("decks")}));
}

TEST(UserDataBackup, ManifestIsHiddenFromSessionLocations)
{
    QTemporaryDir sourceDir;
    QTemporaryDir backupRoot;
    ASSERT_TRUE(sourceDir.isValid());
    ASSERT_TRUE(backupRoot.isValid());
    QFile deck(QDir(sourceDir.path()).absoluteFilePath("mydeck.cod"));
    ASSERT_TRUE(deck.open(QIODevice::WriteOnly | QIODevice::Text));
    deck.write("x");
    deck.close();

    QStringList locations;
    QString sessionDir;
    ASSERT_TRUE(UserDataBackup::backupAtRiskPaths({sourceDir.path()}, backupRoot.path(), &locations, &sessionDir));
    for (const QString &location : UserDataBackup::listSessionBackupLocations(sessionDir)) {
        EXPECT_FALSE(location.endsWith(QStringLiteral("backup.sources")));
    }
}

TEST(UserDataBackup, FindRecentUsableSkipsIncompleteNewest)
{
    QTemporaryDir sourceDir;
    QTemporaryDir backupRoot;
    ASSERT_TRUE(sourceDir.isValid());
    ASSERT_TRUE(backupRoot.isValid());
    QFile deck(QDir(sourceDir.path()).absoluteFilePath("mydeck.cod"));
    ASSERT_TRUE(deck.open(QIODevice::WriteOnly | QIODevice::Text));
    deck.write("<cockatrice_deck version=\"1\"/>\n");
    deck.close();

    QStringList locations;
    QString oldSession;
    QString newSession;
    ASSERT_TRUE(UserDataBackup::backupAtRiskPaths({sourceDir.path()}, backupRoot.path(), &locations, &oldSession));
    ASSERT_TRUE(UserDataBackup::backupAtRiskPaths({sourceDir.path()}, backupRoot.path(), &locations, &newSession));
    ASSERT_NE(oldSession, newSession);

    // Newest session becomes incomplete: usable search must fall back to the older complete one.
    ASSERT_TRUE(QFile::remove(QDir(newSession).absoluteFilePath("backup.complete")));
    QFile marker(QDir(newSession).absoluteFilePath("backup.incomplete"));
    ASSERT_TRUE(marker.open(QIODevice::WriteOnly | QIODevice::Text));
    marker.write("incomplete\n");
    marker.close();

    EXPECT_EQ(UserDataBackup::findRecentUsableBackupSession(backupRoot.path(), {sourceDir.path()}), oldSession);
    // Covering search still finds the newest session even though it is incomplete.
    EXPECT_EQ(UserDataBackup::findRecentCoveringBackupSession(backupRoot.path(), {sourceDir.path()}), newSession);
}

TEST(UserDataBackup, FindRecentUsableRequiresCoveringSet)
{
    QTemporaryDir sourceA;
    QTemporaryDir sourceB;
    QTemporaryDir backupRoot;
    ASSERT_TRUE(sourceA.isValid());
    ASSERT_TRUE(sourceB.isValid());
    ASSERT_TRUE(backupRoot.isValid());
    QFile fa(QDir(sourceA.path()).absoluteFilePath("a.cod"));
    ASSERT_TRUE(fa.open(QIODevice::WriteOnly | QIODevice::Text));
    fa.write("a");
    fa.close();
    QFile fb(QDir(sourceB.path()).absoluteFilePath("b.cod"));
    ASSERT_TRUE(fb.open(QIODevice::WriteOnly | QIODevice::Text));
    fb.write("b");
    fb.close();

    QStringList locations;
    QString oldSession;
    QString newSession;
    ASSERT_TRUE(UserDataBackup::backupAtRiskPaths({sourceA.path()}, backupRoot.path(), &locations, &oldSession));
    ASSERT_TRUE(UserDataBackup::backupAtRiskPaths({sourceB.path()}, backupRoot.path(), &locations, &newSession));

    // Newest covers B only, so a query for A must reuse the older session.
    EXPECT_EQ(UserDataBackup::findRecentUsableBackupSession(backupRoot.path(), {sourceA.path()}), oldSession);
    EXPECT_EQ(UserDataBackup::findRecentUsableBackupSession(backupRoot.path(), {sourceB.path()}), newSession);
    EXPECT_TRUE(
        UserDataBackup::findRecentUsableBackupSession(backupRoot.path(), {sourceA.path(), sourceB.path()}).isEmpty());
    EXPECT_TRUE(UserDataBackup::findRecentUsableBackupSession(backupRoot.path(), {}).isEmpty());
}

TEST(UserDataBackup, CopyDirectoryDoesNotSilentlyEmptyDirSymlink)
{
    QTemporaryDir sourceDir;
    QTemporaryDir destParent;
    ASSERT_TRUE(sourceDir.isValid());
    ASSERT_TRUE(destParent.isValid());
    QDir source(sourceDir.path());
    ASSERT_TRUE(source.mkpath("real"));
    QFile inner(source.absoluteFilePath("real/card.cod"));
    ASSERT_TRUE(inner.open(QIODevice::WriteOnly | QIODevice::Text));
    inner.write("x");
    inner.close();
    if (!QFile::link(source.absoluteFilePath("real"), source.absoluteFilePath("link"))) {
        GTEST_SKIP() << "Symlinks not permitted in this environment";
    }

    QString dest = QDir(destParent.path()).absoluteFilePath("copy");
    const bool ok = UserDataBackup::copyDirectoryRecursively(sourceDir.path(), dest);
    const QFileInfo linkCopy(QDir(dest).absoluteFilePath("link"));
    // Old behavior created an empty plain dir and reported success, silently dropping contents.
    // New behavior copies the link best-effort; if that fails it must report failure.
    if (ok) {
        EXPECT_TRUE(linkCopy.exists() || linkCopy.isSymLink());
        if (linkCopy.isDir() && !linkCopy.isSymLink()) {
            EXPECT_FALSE(QDir(linkCopy.absoluteFilePath()).isEmpty());
        }
    } else {
        EXPECT_TRUE(linkCopy.isSymLink() || !linkCopy.exists() || !QDir(linkCopy.absoluteFilePath()).isEmpty());
    }
}

TEST(UserDataBackup, PluralBackupClearsOutParamsOnTotalFailure)
{
    QTemporaryDir backupRoot;
    ASSERT_TRUE(backupRoot.isValid());
    QStringList locations = {QStringLiteral("stale")};
    QString sessionDir = QStringLiteral("stale");
    const QString missing = QDir(backupRoot.path()).absoluteFilePath("does-not-exist.cod");
    EXPECT_FALSE(UserDataBackup::backupAtRiskPaths({missing}, backupRoot.path(), &locations, &sessionDir));
    EXPECT_TRUE(locations.isEmpty());
    EXPECT_TRUE(sessionDir.isEmpty());
}

} // namespace

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
