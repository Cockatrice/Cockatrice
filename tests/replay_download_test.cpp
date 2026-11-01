#include "interface/widgets/replay/replay_download.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <gtest/gtest.h>
#include <libcockatrice/protocol/pb/game_replay.pb.h>

namespace
{
class ReplayDownloadTest : public ::testing::Test
{
protected:
    QTemporaryDir directory;
    QString error;

    void SetUp() override
    {
        ASSERT_TRUE(directory.isValid());
    }

    void write(const QString &path, const QByteArray &data)
    {
        QFile file(path);
        ASSERT_TRUE(file.open(QIODevice::WriteOnly));
        ASSERT_EQ(file.write(data), data.size());
    }

    QByteArray read(const QString &path)
    {
        QFile file(path);
        EXPECT_TRUE(file.open(QIODevice::ReadOnly));
        return file.readAll();
    }

    QByteArray payload()
    {
        GameReplay replay;
        replay.set_replay_id(17);
        replay.set_duration_seconds(42);
        return QByteArray::fromStdString(replay.SerializeAsString());
    }
};

TEST_F(ReplayDownloadTest, MatchNamesStayInsideSelectedDirectory)
{
    const QString path = ReplayDownload::prepareMatchDirectory(directory.path(), 17, "a/b\\c:?. ", error);
    EXPECT_EQ(path, directory.filePath("17_a_b_c__"));
    EXPECT_TRUE(QFileInfo(path).isDir());
    EXPECT_TRUE(error.isEmpty());
}

TEST_F(ReplayDownloadTest, DotAndEmptyNamesUseGameId)
{
    for (const QString &name : {QString(), QString("."), QString(".."), QString(".. ")}) {
        EXPECT_EQ(ReplayDownload::prepareMatchDirectory(directory.path(), 17, name, error), directory.filePath("17"));
        EXPECT_TRUE(error.isEmpty());
    }
}

TEST_F(ReplayDownloadTest, ExistingDirectoryIsReusedWithoutRemovingContents)
{
    const QString path = directory.filePath("17_match");
    ASSERT_TRUE(QDir().mkdir(path));
    const QString replayPath = QDir(path).filePath("keep.cor");
    write(replayPath, "existing replay");
    EXPECT_EQ(ReplayDownload::prepareMatchDirectory(directory.path(), 17, "match", error), path);
    EXPECT_EQ(read(replayPath), "existing replay");
}

TEST_F(ReplayDownloadTest, ExistingFileBlocksMatchDirectory)
{
    const QString path = directory.filePath("17_match");
    write(path, "existing replay");
    EXPECT_TRUE(ReplayDownload::prepareMatchDirectory(directory.path(), 17, "match", error).isEmpty());
    EXPECT_FALSE(error.isEmpty());
    EXPECT_EQ(read(path), "existing replay");
}

#ifdef Q_OS_UNIX
TEST_F(ReplayDownloadTest, MatchDirectoryCannotFollowExistingSymlink)
{
    QTemporaryDir outside;
    ASSERT_TRUE(outside.isValid());
    ASSERT_TRUE(QFile::link(outside.path(), directory.filePath("17_match")));
    EXPECT_TRUE(ReplayDownload::prepareMatchDirectory(directory.path(), 17, "match", error).isEmpty());
    EXPECT_FALSE(error.isEmpty());
    EXPECT_TRUE(QDir(outside.path()).isEmpty());
}

TEST_F(ReplayDownloadTest, DanglingSymlinkBlocksMatchDirectory)
{
    ASSERT_TRUE(QFile::link(directory.filePath("missing"), directory.filePath("17_match")));
    EXPECT_TRUE(ReplayDownload::prepareMatchDirectory(directory.path(), 17, "match", error).isEmpty());
    EXPECT_FALSE(error.isEmpty());
    EXPECT_FALSE(QFileInfo::exists(directory.filePath("missing")));
}
#endif

TEST_F(ReplayDownloadTest, MissingParentReportsFailure)
{
    EXPECT_TRUE(ReplayDownload::prepareMatchDirectory(directory.filePath("missing"), 17, "match", error).isEmpty());
    EXPECT_FALSE(error.isEmpty());
}

TEST_F(ReplayDownloadTest, CorruptAndEmptyPayloadsPreservePreviousReplay)
{
    const QString path = directory.filePath("replay.cor");
    const QByteArray previous = payload();
    write(path, previous);
    for (const QByteArray &invalid : {QByteArray(), QByteArray::fromHex("ff"), QByteArray::fromHex("08011210")}) {
        EXPECT_FALSE(ReplayDownload::save(path, invalid, error));
        EXPECT_FALSE(error.isEmpty());
        EXPECT_EQ(read(path), previous);
    }
}

TEST_F(ReplayDownloadTest, ValidPayloadReplacesExistingFile)
{
    const QString path = directory.filePath("replay.cor");
    write(path, "old replay");
    const QByteArray data = payload();
    ASSERT_TRUE(ReplayDownload::save(path, data, error));
    EXPECT_TRUE(error.isEmpty());
    EXPECT_EQ(read(path), data);
    EXPECT_EQ(QDir(directory.path()).entryList(QDir::Files), QStringList{"replay.cor"});
}

TEST_F(ReplayDownloadTest, FailedDestinationLeavesOtherReplaysIntact)
{
    const QString previous = directory.filePath("keep.cor");
    write(previous, "existing replay");
    EXPECT_FALSE(ReplayDownload::save(directory.filePath("missing/replay.cor"), payload(), error));
    EXPECT_FALSE(error.isEmpty());
    EXPECT_EQ(read(previous), "existing replay");
    EXPECT_FALSE(QFileInfo::exists(directory.filePath("missing")));
}
} // namespace
