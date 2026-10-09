#include "interface/card_database_update/card_update_progress_parser.h"

#include <QByteArray>
#include <QString>
#include <gtest/gtest.h>

namespace
{
std::optional<CardUpdateProgress> parseLine(const QByteArray &line)
{
    return CardUpdateProgress::fromProtocolLine(line);
}
} // namespace

TEST(CardUpdateProgressTest, ParsesAValidDownloadLine)
{
    const std::optional<CardUpdateProgress> progress = parseLine("PROGRESS download 42 100");
    ASSERT_TRUE(progress.has_value());
    EXPECT_EQ(CardUpdateStage::Download, progress->stage);
    EXPECT_EQ(42, progress->done);
    EXPECT_EQ(100, progress->total);
}

TEST(CardUpdateProgressTest, ParsesTrailingNewline)
{
    const std::optional<CardUpdateProgress> progress = parseLine("PROGRESS scan 7 99\n");
    ASSERT_TRUE(progress.has_value());
    EXPECT_EQ(CardUpdateStage::Scan, progress->stage);
    EXPECT_EQ(7, progress->done);
    EXPECT_EQ(99, progress->total);
}

TEST(CardUpdateProgressTest, ParsesWithSurroundingWhitespace)
{
    const std::optional<CardUpdateProgress> progress = parseLine(QByteArray("  PROGRESS import 3 10  "));
    ASSERT_TRUE(progress.has_value());
    EXPECT_EQ(CardUpdateStage::Import, progress->stage);
    EXPECT_EQ(3, progress->done);
    EXPECT_EQ(10, progress->total);
}

TEST(CardUpdateProgressTest, AcceptsZeroProgress)
{
    const std::optional<CardUpdateProgress> progress = parseLine("PROGRESS download 0 0");
    ASSERT_TRUE(progress.has_value());
    EXPECT_EQ(0, progress->done);
    EXPECT_EQ(0, progress->total);
}

TEST(CardUpdateProgressTest, RejectsLinesWithoutProgressPrefix)
{
    EXPECT_FALSE(parseLine("download 42 100").has_value());
    EXPECT_FALSE(parseLine("PROGRESSdownload 42 100").has_value());
    EXPECT_FALSE(parseLine("").has_value());
}

TEST(CardUpdateProgressTest, RejectsMalformedLines)
{
    EXPECT_FALSE(parseLine("PROGRESS").has_value());
    EXPECT_FALSE(parseLine("PROGRESS download").has_value());
    EXPECT_FALSE(parseLine("PROGRESS download 42 100 extra").has_value());
    EXPECT_FALSE(parseLine("PROGRESS download 42").has_value());
    EXPECT_FALSE(parseLine("PROGRESS download notANumber 100").has_value());
}

TEST(CardUpdateProgressTest, RejectsNegativeValues)
{
    EXPECT_FALSE(parseLine("PROGRESS download -1 100").has_value());
    EXPECT_FALSE(parseLine("PROGRESS download 42 -100").has_value());
}

TEST(CardUpdateProgressTest, AcceptsUnknownStagesSchemalessly)
{
    const std::optional<CardUpdateProgress> progress = parseLine("PROGRESS spoilers 1 2");
    ASSERT_TRUE(progress.has_value());
    EXPECT_EQ(CardUpdateStage::Unknown, progress->stage);
    EXPECT_EQ(1, progress->done);
    EXPECT_EQ(2, progress->total);
}

TEST(CardUpdateProgressTest, StageTokensRoundTrip)
{
    EXPECT_EQ("download", CardUpdateProgress{CardUpdateStage::Download}.stageToken());
    EXPECT_EQ("scan", CardUpdateProgress{CardUpdateStage::Scan}.stageToken());
    EXPECT_EQ("import", CardUpdateProgress{CardUpdateStage::Import}.stageToken());
    EXPECT_EQ("unknown", CardUpdateProgress{CardUpdateStage::Unknown}.stageToken());
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}