#include "../../oracle/src/tag_database.h"

#include "gtest/gtest.h"
#include <QByteArray>
#include <QString>
#include <QStringList>
#include <memory>

#ifdef HAS_ZLIB
#include <zlib.h>
#endif

namespace
{
const QString ORACLE_A = QStringLiteral("aaaaaaaa-aaaa-aaaa-aaaa-aaaaaaaaaaaa");
const QString ORACLE_B = QStringLiteral("bbbbbbbb-bbbb-bbbb-bbbb-bbbbbbbbbbbb");

std::shared_ptr<TagDatabase> load(const QByteArray &data,
                                  const TagDatabase::LoadOptions &options = TagDatabase::LoadOptions{},
                                  QString *errorMessage = nullptr)
{
    return TagDatabase::loadFromData(data, options, errorMessage);
}

QString keyedByOracleId(const QString &oracleId, const QString &slugs)
{
    return QStringLiteral("{\"%1\": [%2]}").arg(oracleId, slugs);
}

#ifdef HAS_ZLIB
QByteArray gzipCompress(const QByteArray &input)
{
    z_stream stream{};
    if (deflateInit2(&stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 16 + MAX_WBITS, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
        return {};
    }

    stream.next_in = reinterpret_cast<Bytef *>(const_cast<char *>(input.constData()));
    stream.avail_in = static_cast<uInt>(input.size());

    QByteArray output;
    char buffer[4096];
    int result = Z_OK;
    while (result == Z_OK) {
        stream.next_out = reinterpret_cast<Bytef *>(buffer);
        stream.avail_out = sizeof(buffer);
        result = deflate(&stream, Z_FINISH);
        output.append(buffer, static_cast<int>(sizeof(buffer) - stream.avail_out));
    }

    deflateEnd(&stream);
    return result == Z_STREAM_END ? output : QByteArray();
}
#endif
} // namespace

TEST(TagDatabaseTest, ParsesScryfallJsonLines)
{
    const QByteArray data = R"json(
{"object":"tag","id":"11111111-1111-1111-1111-111111111111","slug":"removal","type":"oracle","parent_ids":[],"taggings":[{"oracle_id":"aaaaaaaa-aaaa-aaaa-aaaa-aaaaaaaaaaaa","weight":"median"}]}
{"object":"tag","id":"22222222-2222-2222-2222-222222222222","slug":"ramp","type":"oracle","parent_ids":[],"taggings":[{"oracle_id":"aaaaaaaa-aaaa-aaaa-aaaa-aaaaaaaaaaaa","weight":"median"},{"oracle_id":"bbbbbbbb-bbbb-bbbb-bbbb-bbbbbbbbbbbb","weight":"strong"}]}
)json";

    QString error;
    const auto database = load(data, TagDatabase::LoadOptions{}, &error);
    ASSERT_TRUE(database) << error.toStdString();

    EXPECT_EQ(database->tagCount(), 2);
    EXPECT_EQ(database->taggedCardCount(), 2);
    EXPECT_EQ(database->tagsForCard(ORACLE_A, QString()), QStringList({"ramp", "removal"}));
    EXPECT_EQ(database->tagsForCard(ORACLE_B, QString()), QStringList({"ramp"}));
}

TEST(TagDatabaseTest, ParsesScryfallJsonArray)
{
    const QByteArray data = R"json([
{"object":"tag","id":"11111111-1111-1111-1111-111111111111","slug":"removal","type":"oracle","taggings":[{"oracle_id":"aaaaaaaa-aaaa-aaaa-aaaa-aaaaaaaaaaaa"}]}
])json";

    QString error;
    const auto database = load(data, TagDatabase::LoadOptions{}, &error);
    ASSERT_TRUE(database) << error.toStdString();

    EXPECT_EQ(database->tagsForCard(ORACLE_A, QString()), QStringList({"removal"}));
}

TEST(TagDatabaseTest, IgnoresNonOracleTagTypes)
{
    const QByteArray data = R"json(
{"object":"tag","id":"11111111-1111-1111-1111-111111111111","slug":"removal","type":"oracle","taggings":[{"oracle_id":"aaaaaaaa-aaaa-aaaa-aaaa-aaaaaaaaaaaa"}]}
{"object":"tag","id":"22222222-2222-2222-2222-222222222222","slug":"art-only","type":"art","taggings":[{"oracle_id":"aaaaaaaa-aaaa-aaaa-aaaa-aaaaaaaaaaaa"}]}
)json";

    QString error;
    const auto database = load(data, TagDatabase::LoadOptions{}, &error);
    ASSERT_TRUE(database) << error.toStdString();

    EXPECT_EQ(database->tagsForCard(ORACLE_A, QString()), QStringList({"removal"}));
}

TEST(TagDatabaseTest, ParsesFlatOracleIdMap)
{
    const QByteArray data = keyedByOracleId(ORACLE_A, R"("Removal", "removal", " ramp ")").toUtf8();

    QString error;
    const auto database = load(data, TagDatabase::LoadOptions{}, &error);
    ASSERT_TRUE(database) << error.toStdString();

    // Mixed case, duplicates and surrounding whitespace are normalized.
    EXPECT_EQ(database->tagsForCard(ORACLE_A, QString()), QStringList({"ramp", "removal"}));
    EXPECT_EQ(database->tagsForCard(ORACLE_A.toUpper(), QString()), QStringList({"ramp", "removal"}));
    EXPECT_EQ(database->tagsForCard(ORACLE_B, QString()), QStringList());
}

TEST(TagDatabaseTest, ParsesFlatNameMap)
{
    const QByteArray data = R"json({"Lightning Bolt": ["removal", "burn"]})json";

    QString error;
    const auto database = load(data, TagDatabase::LoadOptions{}, &error);
    ASSERT_TRUE(database) << error.toStdString();

    // Name lookups are case-insensitive; an absent oracle id does not block them.
    EXPECT_EQ(database->tagsForCard(QString(), QStringLiteral("lightning bolt")), QStringList({"burn", "removal"}));
    EXPECT_EQ(database->tagsForCard(ORACLE_A, QStringLiteral("Lightning Bolt")), QStringList({"burn", "removal"}));
}

TEST(TagDatabaseTest, ParsesManifestWithNameKey)
{
    const QByteArray data =
        R"json({"version": 2, "key": "name", "tags": {"Lightning Bolt": ["removal"], "zzz-not-a-uuid": ["burn"]}})json";

    QString error;
    const auto database = load(data, TagDatabase::LoadOptions{}, &error);
    ASSERT_TRUE(database) << error.toStdString();

    EXPECT_EQ(database->tagsForCard(QString(), QStringLiteral("Lightning Bolt")), QStringList({"removal"}));
    EXPECT_EQ(database->tagsForCard(QString(), QStringLiteral("ZZZ-not-a-uuid")), QStringList({"burn"}));
}

TEST(TagDatabaseTest, ParsesJsonLinesRows)
{
    const QByteArray data = R"json(
{"oracle_id": "aaaaaaaa-aaaa-aaaa-aaaa-aaaaaaaaaaaa", "tags": ["removal"]}
{"name": "Lightning Bolt", "tags": ["burn", "removal"]}
)json";

    QString error;
    const auto database = load(data, TagDatabase::LoadOptions{}, &error);
    ASSERT_TRUE(database) << error.toStdString();

    EXPECT_EQ(database->tagsForCard(ORACLE_A, QString()), QStringList({"removal"}));
    EXPECT_EQ(database->tagsForCard(QString(), QStringLiteral("Lightning Bolt")), QStringList({"burn", "removal"}));
}

TEST(TagDatabaseTest, MergesOracleIdAndNameMatches)
{
    const QByteArray data =
        R"json({"aaaaaaaa-aaaa-aaaa-aaaa-aaaaaaaaaaaa": ["removal"], "Lightning Bolt": ["burn", "removal"]})json";

    QString error;
    const auto database = load(data, TagDatabase::LoadOptions{}, &error);
    ASSERT_TRUE(database) << error.toStdString();

    EXPECT_EQ(database->tagsForCard(ORACLE_A, QStringLiteral("Lightning Bolt")), QStringList({"burn", "removal"}));
}

TEST(TagDatabaseTest, DropsSlugsContainingWhitespace)
{
    const QByteArray data = keyedByOracleId(ORACLE_A, R"("two words", "valid")").toUtf8();

    QString error;
    const auto database = load(data, TagDatabase::LoadOptions{}, &error);
    ASSERT_TRUE(database) << error.toStdString();

    // A slug with a space would corrupt the space-separated `tags` property.
    EXPECT_EQ(database->tagsForCard(ORACLE_A, QString()), QStringList({"valid"}));
}

TEST(TagDatabaseTest, ParentTagsAreOffByDefault)
{
    const QByteArray data = R"json(
{"object":"tag","id":"child","slug":"tutor-creature-giant","type":"oracle","parent_ids":["parent","missing"],"taggings":[{"oracle_id":"aaaaaaaa-aaaa-aaaa-aaaa-aaaaaaaaaaaa"}]}
{"object":"tag","id":"parent","slug":"tutor-creature","type":"oracle","parent_ids":["grandparent"],"taggings":[]}
{"object":"tag","id":"grandparent","slug":"tutor","type":"oracle","parent_ids":[],"taggings":[]}
)json";

    QString error;
    const auto database = load(data, TagDatabase::LoadOptions{}, &error);
    ASSERT_TRUE(database) << error.toStdString();

    EXPECT_EQ(database->tagsForCard(ORACLE_A, QString()), QStringList({"tutor-creature-giant"}));
}

TEST(TagDatabaseTest, ParentTagsExpandTransitivelyWhenEnabled)
{
    const QByteArray data = R"json(
{"object":"tag","id":"child","slug":"tutor-creature-giant","type":"oracle","parent_ids":["parent","missing"],"taggings":[{"oracle_id":"aaaaaaaa-aaaa-aaaa-aaaa-aaaaaaaaaaaa"}]}
{"object":"tag","id":"parent","slug":"tutor-creature","type":"oracle","parent_ids":["grandparent"],"taggings":[]}
{"object":"tag","id":"grandparent","slug":"tutor","type":"oracle","parent_ids":[],"taggings":[]}
)json";

    QString error;
    const auto database = load(data, TagDatabase::LoadOptions{.includeParentTags = true}, &error);
    ASSERT_TRUE(database) << error.toStdString();

    // The child precedes its parents in the file; expansion must not depend on order.
    EXPECT_EQ(database->tagsForCard(ORACLE_A, QString()),
              QStringList({"tutor", "tutor-creature", "tutor-creature-giant"}));
}

TEST(TagDatabaseTest, RejectsEmptyData)
{
    QString error;
    EXPECT_FALSE(load(QByteArray(), TagDatabase::LoadOptions{}, &error));
    EXPECT_FALSE(error.isEmpty());
}

TEST(TagDatabaseTest, RejectsMalformedJson)
{
    QString error;
    EXPECT_FALSE(load("not a tag database", TagDatabase::LoadOptions{}, &error));
    EXPECT_FALSE(error.isEmpty());
}

TEST(TagDatabaseTest, RejectsTagListsThatAreNotArrays)
{
    QString error;
    EXPECT_FALSE(load(R"json({"tags": {"Lightning Bolt": "removal"}})json", TagDatabase::LoadOptions{}, &error));
    EXPECT_FALSE(error.isEmpty());
}

TEST(TagDatabaseTest, RejectsDatabaseWithoutUsableTags)
{
    QString error;
    EXPECT_FALSE(load(R"json({"Lightning Bolt": []})json", TagDatabase::LoadOptions{}, &error));
    EXPECT_FALSE(error.isEmpty());
}

#ifdef HAS_ZLIB
TEST(TagDatabaseTest, DecompressesGzipPayloads)
{
    const QByteArray plain = keyedByOracleId(ORACLE_A, R"("removal")").toUtf8();
    const QByteArray compressed = gzipCompress(plain);
    ASSERT_FALSE(compressed.isEmpty());
    ASSERT_EQ(compressed.left(2), QByteArray::fromHex("1f8b"));

    QString error;
    const auto database = load(compressed, TagDatabase::LoadOptions{}, &error);
    ASSERT_TRUE(database) << error.toStdString();

    EXPECT_EQ(database->tagsForCard(ORACLE_A, QString()), QStringList({"removal"}));
}
#endif
