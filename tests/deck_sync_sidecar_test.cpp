#include "gtest/gtest.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QTemporaryDir>
#include <deck_sync_sidecar.h>

namespace
{
QString sidecarPath(QTemporaryDir &dir)
{
    return dir.filePath(".sync/profilekey.json");
}
} // namespace

TEST(DeckSyncSidecarTest, MissingFileLoadsEmpty)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    DeckSyncSidecar sidecar(sidecarPath(dir));
    EXPECT_TRUE(sidecar.load());
    EXPECT_TRUE(sidecar.records().isEmpty());
    EXPECT_FALSE(sidecar.contains("affinity.cod"));
}

TEST(DeckSyncSidecarTest, RoundTripsRecords)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    DeckSyncSidecar sidecar(sidecarPath(dir));

    DeckSyncRecord record;
    record.guid = "123e4567-e89b-12d3-a456-426614174000";
    record.remoteId = 42;
    record.baseHash = "5cac19qm";
    record.lastSyncSecs = 1791739200;
    record.deleted = true;
    sidecar.set("decks/affinity.cod", record);
    EXPECT_TRUE(sidecar.save());

    DeckSyncSidecar reloaded(sidecarPath(dir));
    EXPECT_TRUE(reloaded.load());
    EXPECT_TRUE(reloaded.contains("decks/affinity.cod"));
    DeckSyncRecord back = reloaded.get("decks/affinity.cod");
    EXPECT_EQ(back.guid, record.guid);
    EXPECT_EQ(back.remoteId, 42);
    EXPECT_EQ(back.baseHash, "5cac19qm");
    EXPECT_EQ(back.lastSyncSecs, 1791739200);
    EXPECT_TRUE(back.deleted);
}

TEST(DeckSyncSidecarTest, UnknownPathReturnsDefaultRecord)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    DeckSyncSidecar sidecar(sidecarPath(dir));
    DeckSyncRecord back = sidecar.get("missing.cod");
    EXPECT_EQ(back.remoteId, -1);
    EXPECT_TRUE(back.guid.isEmpty());
}

namespace
{
bool writeRaw(const QString &path, const QByteArray &bytes)
{
    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile file(path);
    return file.open(QIODevice::WriteOnly | QIODevice::Truncate) && file.write(bytes) == bytes.size();
}
} // namespace

TEST(DeckSyncSidecarTest, GarbageFileFailsToLoad)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    ASSERT_TRUE(writeRaw(sidecarPath(dir), "not json at all"));
    DeckSyncSidecar sidecar(sidecarPath(dir));
    EXPECT_FALSE(sidecar.load());
}

TEST(DeckSyncSidecarTest, WrongRecordsShapeFailsToLoad)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    ASSERT_TRUE(writeRaw(sidecarPath(dir), R"({"records":["a.cod"]})"));
    DeckSyncSidecar missing(sidecarPath(dir));
    EXPECT_FALSE(missing.load());

    ASSERT_TRUE(writeRaw(sidecarPath(dir), R"({"foo":1})"));
    DeckSyncSidecar other(sidecarPath(dir));
    EXPECT_FALSE(other.load());
}

TEST(DeckSyncSidecarTest, NonObjectEntriesAreSkipped)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    ASSERT_TRUE(writeRaw(sidecarPath(dir), R"({"records":{"a.cod":42,"b.cod":{"guid":"g"}}})"));
    DeckSyncSidecar sidecar(sidecarPath(dir));
    EXPECT_TRUE(sidecar.load());
    EXPECT_FALSE(sidecar.contains("a.cod"));
    EXPECT_TRUE(sidecar.contains("b.cod"));
    EXPECT_EQ(sidecar.get("b.cod").guid, "g");
}

TEST(DeckSyncSidecarTest, RemoveDropsRecord)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    DeckSyncSidecar sidecar(sidecarPath(dir));
    sidecar.set("a.cod", DeckSyncRecord());
    sidecar.remove("a.cod");
    EXPECT_FALSE(sidecar.contains("a.cod"));
    EXPECT_TRUE(sidecar.save());

    DeckSyncSidecar reloaded(sidecarPath(dir));
    EXPECT_TRUE(reloaded.load());
    EXPECT_FALSE(reloaded.contains("a.cod"));
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
