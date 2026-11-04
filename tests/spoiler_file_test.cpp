#include "client/network/update/card_spoiler/spoiler_file.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <gtest/gtest.h>

#ifdef Q_OS_UNIX
#include <csignal>
#include <sys/resource.h>
#include <unistd.h>
#endif

namespace
{
class SpoilerFileTest : public ::testing::Test
{
protected:
    QTemporaryDir directory;
    QString path;
    QString error;
    const QByteArray original = "<cockatrice_carddatabase version=\"4\"><cards/></cockatrice_carddatabase>";

    void SetUp() override
    {
        ASSERT_TRUE(directory.isValid());
        path = directory.filePath("spoiler.xml");
        QFile file(path);
        ASSERT_TRUE(file.open(QIODevice::WriteOnly));
        ASSERT_EQ(file.write(original), original.size());
    }

    QByteArray read()
    {
        QFile file(path);
        EXPECT_TRUE(file.open(QIODevice::ReadOnly));
        return file.readAll();
    }

    SpoilerFile::SaveResult save(const QByteArray &data)
    {
        return SpoilerFile::save(path, data, [] { return true; }, error);
    }
};

TEST_F(SpoilerFileTest, InvalidDocumentsPreserveExistingFile)
{
    for (const QByteArray &invalid :
         {QByteArray(), QByteArray("<html><cards/></html>"),
          QByteArray("<cockatrice_carddatabase version=\"2\"><cards/></cockatrice_carddatabase>"),
          QByteArray("<cockatrice_carddatabase><cards/></cockatrice_carddatabase>"),
          QByteArray("<cockatrice_carddatabase version=\"4\"><sets/></cockatrice_carddatabase>"),
          QByteArray("<cockatrice_carddatabase version=\"4\"><cards>"),
          QByteArray("<cockatrice_carddatabase version=\"4\"><cards/></cockatrice_carddatabase><extra/>")}) {
        EXPECT_EQ(save(invalid), SpoilerFile::SaveResult::Failed);
        EXPECT_FALSE(error.isEmpty());
        EXPECT_EQ(read(), original);
    }
}

TEST_F(SpoilerFileTest, SupportedVersionsAreAccepted)
{
    EXPECT_TRUE(SpoilerFile::validate(original).isEmpty());
    EXPECT_TRUE(
        SpoilerFile::validate("<cockatrice_carddatabase version=\"3\"><cards/></cockatrice_carddatabase>").isEmpty());
}

TEST_F(SpoilerFileTest, IdenticalFileIsUnchanged)
{
    EXPECT_EQ(save(original), SpoilerFile::SaveResult::Unchanged);
    EXPECT_TRUE(error.isEmpty());
    EXPECT_EQ(read(), original);
}

TEST_F(SpoilerFileTest, CompleteFileHashDetectsChangesAfterLargeCommonPrefix)
{
    QByteArray changed = "<cockatrice_carddatabase version=\"4\"><!--";
    changed += QByteArray(100000, 'x');
    changed += "--><cards/></cockatrice_carddatabase>";
    ASSERT_EQ(save(changed), SpoilerFile::SaveResult::Written);
    changed.replace("<cards/>", "<cards></cards>");
    EXPECT_EQ(save(changed), SpoilerFile::SaveResult::Written);
    EXPECT_EQ(read(), changed);
    EXPECT_TRUE(error.isEmpty());
}

TEST_F(SpoilerFileTest, CancellationBeforeWritePreservesExistingFile)
{
    EXPECT_EQ(SpoilerFile::save(path, original, [] { return false; }, error), SpoilerFile::SaveResult::Failed);
    EXPECT_FALSE(error.isEmpty());
    EXPECT_EQ(read(), original);
}

TEST_F(SpoilerFileTest, CancellationBeforeCommitPreservesExistingFile)
{
    int checks = 0;
    EXPECT_EQ(SpoilerFile::save(
                  path, original + "\n", [&checks] { return ++checks == 1; }, error),
              SpoilerFile::SaveResult::Failed);
    EXPECT_EQ(checks, 2);
    EXPECT_FALSE(error.isEmpty());
    EXPECT_EQ(read(), original);
    EXPECT_EQ(QDir(directory.path()).entryList(QDir::Files), QStringList{"spoiler.xml"});
}

TEST_F(SpoilerFileTest, MissingParentIsCreated)
{
    path = directory.filePath("nested/spoiler.xml");
    EXPECT_EQ(save(original), SpoilerFile::SaveResult::Written);
    EXPECT_EQ(read(), original);
}

TEST_F(SpoilerFileTest, FileInParentPathReportsFailureWithoutModifyingIt)
{
    EXPECT_EQ(SpoilerFile::save(
                  QDir(path).filePath("spoiler.xml"), original, [] { return true; }, error),
              SpoilerFile::SaveResult::Failed);
    EXPECT_FALSE(error.isEmpty());
    EXPECT_EQ(read(), original);
}

#ifdef Q_OS_UNIX
TEST_F(SpoilerFileTest, ShortWriteLeavesExistingFileIntact)
{
    // Limit only the child process: a real short write exercises QSaveFile's
    // buffered flush failure without depending on disk capacity or file ownership.
    const QByteArray changed = "<cockatrice_carddatabase version=\"4\"><!--" + QByteArray(100000, 'x') +
                               "--><cards/></cockatrice_carddatabase>";
    EXPECT_EXIT(
        {
            struct rlimit limit;
            if (getrlimit(RLIMIT_FSIZE, &limit) != 0) {
                _exit(2);
            }
            limit.rlim_cur = 128;
            if (setrlimit(RLIMIT_FSIZE, &limit) != 0) {
                _exit(3);
            }
            std::signal(SIGXFSZ, SIG_IGN);
            const auto result = save(changed);
            _exit(result == SpoilerFile::SaveResult::Failed && !error.isEmpty() ? 0 : 1);
        },
        ::testing::ExitedWithCode(0), "");
    EXPECT_EQ(read(), original);
    EXPECT_EQ(QDir(directory.path()).entryList(QDir::Files), QStringList{"spoiler.xml"});
}
#endif
} // namespace
