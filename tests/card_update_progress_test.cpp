#include "interface/card_database_update/card_update_progress_parser.h"

#include <QByteArray>
#include <QString>
#include <gtest/gtest.h>

namespace
{
bool parseLine(const QByteArray &line, QString &stage, qint64 &done, qint64 &total)
{
    CardUpdateProgress progress;
    if (!parseCardUpdateProgressLine(line, progress)) {
        return false;
    }
    stage = progress.stage;
    done = progress.done;
    total = progress.total;
    return true;
}
} // namespace

TEST(CardUpdateProgressTest, ParsesAValidDownloadLine)
{
    QString stage;
    qint64 done = -1;
    qint64 total = -1;
    EXPECT_TRUE(parseLine("PROGRESS download 42 100", stage, done, total));
    EXPECT_EQ("download", stage);
    EXPECT_EQ(42, done);
    EXPECT_EQ(100, total);
}

TEST(CardUpdateProgressTest, ParsesTrailingNewline)
{
    QString stage;
    qint64 done = -1;
    qint64 total = -1;
    EXPECT_TRUE(parseLine("PROGRESS scan 7 99\n", stage, done, total));
    EXPECT_EQ("scan", stage);
    EXPECT_EQ(7, done);
    EXPECT_EQ(99, total);
}

TEST(CardUpdateProgressTest, ParsesWithSurroundingWhitespace)
{
    QString stage;
    qint64 done = -1;
    qint64 total = -1;
    EXPECT_TRUE(parseLine(QByteArray("  PROGRESS import 3 10  "), stage, done, total));
    EXPECT_EQ("import", stage);
    EXPECT_EQ(3, done);
    EXPECT_EQ(10, total);
}

TEST(CardUpdateProgressTest, AcceptsZeroProgress)
{
    QString stage;
    qint64 done = -1;
    qint64 total = -1;
    EXPECT_TRUE(parseLine("PROGRESS download 0 0", stage, done, total));
    EXPECT_EQ(0, done);
    EXPECT_EQ(0, total);
}

TEST(CardUpdateProgressTest, RejectsLinesWithoutProgressPrefix)
{
    CardUpdateProgress progress;
    EXPECT_FALSE(parseCardUpdateProgressLine("download 42 100", progress));
    EXPECT_FALSE(parseCardUpdateProgressLine("PROGRESSdownload 42 100", progress));
    EXPECT_FALSE(parseCardUpdateProgressLine("", progress));
}

TEST(CardUpdateProgressTest, RejectsMalformedLines)
{
    CardUpdateProgress progress;
    EXPECT_FALSE(parseCardUpdateProgressLine("PROGRESS", progress));
    EXPECT_FALSE(parseCardUpdateProgressLine("PROGRESS download", progress));
    EXPECT_FALSE(parseCardUpdateProgressLine("PROGRESS download 42 100 extra", progress));
    EXPECT_FALSE(parseCardUpdateProgressLine("PROGRESS download 42", progress));
    EXPECT_FALSE(parseCardUpdateProgressLine("PROGRESS download notANumber 100", progress));
}

TEST(CardUpdateProgressTest, RejectsNegativeValues)
{
    CardUpdateProgress progress;
    EXPECT_FALSE(parseCardUpdateProgressLine("PROGRESS download -1 100", progress));
    EXPECT_FALSE(parseCardUpdateProgressLine("PROGRESS download 42 -100", progress));
}

TEST(CardUpdateProgressTest, AcceptsUnknownStagesSchemalessly)
{
    QString stage;
    qint64 done = -1;
    qint64 total = -1;
    EXPECT_TRUE(parseLine("PROGRESS spoilers 1 2", stage, done, total));
    EXPECT_EQ("spoilers", stage);
    EXPECT_EQ(1, done);
    EXPECT_EQ(2, total);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}