#include "client/crash/crash_reporter.h"

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QString>
#include <QTemporaryDir>
#include <gtest/gtest.h>
#include <libcockatrice/settings/personal_settings.h>
#include <string>

namespace
{

TEST(CrashReporterTest, SanitizeLogLineStripsAnsiColorCodes)
{
    ASSERT_EQ(CrashReporter::sanitizeLogLine(QStringLiteral("\033[32mhello\033[0m")), QStringLiteral("hello"));
    ASSERT_EQ(CrashReporter::sanitizeLogLine(QStringLiteral("\033[1;31mfatal\033[0m")), QStringLiteral("fatal"));
    ASSERT_EQ(CrashReporter::sanitizeLogLine(QStringLiteral("plain line")), QStringLiteral("plain line"));
}

TEST(CrashReporterTest, RedactCockatriceUrlsKeepsSchemeAndHostOnly)
{
    ASSERT_EQ(
        CrashReporter::redactCockatriceUrls(QStringLiteral("open cockatrice://opendeck?token=secret&user=me now")),
        QStringLiteral("open cockatrice://opendeck now"));
}

TEST(CrashReporterTest, RedactCockatriceUrlsIsIdempotent)
{
    // A secret-free URL must not match again after redaction (regression: this hung forever).
    ASSERT_EQ(CrashReporter::redactCockatriceUrls(QStringLiteral("open cockatrice://opendeck now")),
              QStringLiteral("open cockatrice://opendeck now"));
}

TEST(CrashReporterTest, RedactCockatriceUrlsLeavesOtherTextAlone)
{
    ASSERT_EQ(CrashReporter::redactCockatriceUrls(QStringLiteral("https://cockatrice.github.io/?token=secret")),
              QStringLiteral("https://cockatrice.github.io/?token=secret"));
    ASSERT_EQ(CrashReporter::sanitizeLogLine(QStringLiteral("")), QStringLiteral(""));
}

TEST(CrashReporterTest, InitSkippedWithoutDsn)
{
    CrashReporter reporter;
    reporter.init(QString(), QStringLiteral("3.1.2"), QDir::tempPath() + QStringLiteral("/cockatrice-crash-test"),
                  PersonalSettings::CrashReportingMode::Ask);
    ASSERT_FALSE(reporter.isReady());
    reporter.shutdown();
    ASSERT_FALSE(reporter.isReady());
}

TEST(CrashReporterTest, InitSkippedWhenNeverSend)
{
    CrashReporter reporter;
    reporter.init(QStringLiteral("https://key@sentry.io/1"), QStringLiteral("3.1.2"),
                  QDir::tempPath() + QStringLiteral("/cockatrice-crash-test"),
                  PersonalSettings::CrashReportingMode::NeverSend);
    ASSERT_FALSE(reporter.isReady());
    reporter.shutdown();
}

// The init() tests above compile to no-ops when the SDK is off, so they pass
// regardless of the gating; the gating decision itself is covered here, in both
// build modes, through the SDK-independent helper init() consults.
TEST(CrashReporterTest, ReportingEnabledGating)
{
    using Mode = PersonalSettings::CrashReportingMode;
    const QString dsn = QStringLiteral("https://key@sentry.io/1");
    ASSERT_FALSE(CrashReporter::isReportingEnabled(QString(), Mode::Ask));
    ASSERT_FALSE(CrashReporter::isReportingEnabled(QString(), Mode::AlwaysSend));
    ASSERT_FALSE(CrashReporter::isReportingEnabled(QString(), Mode::NeverSend));
    ASSERT_TRUE(CrashReporter::isReportingEnabled(dsn, Mode::Ask));
    ASSERT_TRUE(CrashReporter::isReportingEnabled(dsn, Mode::AlwaysSend));
    ASSERT_FALSE(CrashReporter::isReportingEnabled(dsn, Mode::NeverSend));
}

TEST(CrashReporterTest, ShutdownWithoutInitIsSafe)
{
    CrashReporter reporter;
    ASSERT_FALSE(reporter.isReady());
    reporter.shutdown();
    ASSERT_FALSE(reporter.isReady());
}

TEST(CrashReporterTest, ConsentAndSendWithoutDsnAreSafeNoOps)
{
    CrashReporter reporter;
    reporter.init(QString(), QStringLiteral("3.1.2"), QDir::tempPath() + QStringLiteral("/cockatrice-crash-test"),
                  PersonalSettings::CrashReportingMode::Ask);
    ASSERT_FALSE(reporter.isReady());
    reporter.giveConsent();
    reporter.revokeConsent();
    ASSERT_FALSE(reporter.isReady());
    ASSERT_FALSE(reporter.sendReport(QStringLiteral("preview")));
    ASSERT_FALSE(reporter.sendReport(QString()));
    reporter.shutdown();
    ASSERT_FALSE(reporter.isReady());
}

namespace
{

QByteArray readFileBytes(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }
    return file.readAll();
}

bool writeFileBytes(const QString &path, const QByteArray &content)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }
    return file.write(content) == content.size();
}

QString uuidOf(char marker)
{
    // 36 chars, 4 hyphens — structurally a UUID, content irrelevant for these tests.
    return QStringLiteral("00000000-0000-4000-8000-00000000000%1").arg(marker);
}

} // namespace

TEST(CrashReporterTest, AppendAttachmentAppendsWellFormedItemAfterExistingContent)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("event.envelope"));
    // Existing envelope content deliberately WITHOUT a trailing newline.
    const QByteArray original = QByteArrayLiteral("{\"event_id\":\"x\"}\n{\"type\":\"event\",\"length\":2}\n{}");
    ASSERT_TRUE(writeFileBytes(path, original));

    ASSERT_TRUE(
        CrashReporter::appendAttachmentToEnvelope(path, QStringLiteral("crash-log.txt"), QByteArrayLiteral("hello")));
    const QByteArray expected = original + '\n' +
                                QByteArrayLiteral("{\"type\":\"attachment\",\"length\":5,\"attachment_length\":5,"
                                                  "\"filename\":\"crash-log.txt\",\"content_type\":\"text/plain\"}\n") +
                                QByteArrayLiteral("hello");
    ASSERT_EQ(readFileBytes(path), expected);
}

TEST(CrashReporterTest, AppendAttachmentOnEmptyFileHasNoLeadingNewline)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("event.envelope"));
    ASSERT_TRUE(writeFileBytes(path, QByteArray()));

    ASSERT_TRUE(
        CrashReporter::appendAttachmentToEnvelope(path, QStringLiteral("user-note.txt"), QByteArrayLiteral("note")));
    const QByteArray expected = QByteArrayLiteral("{\"type\":\"attachment\",\"length\":4,\"attachment_length\":4,"
                                                  "\"filename\":\"user-note.txt\",\"content_type\":\"text/plain\"}\n") +
                                QByteArrayLiteral("note");
    ASSERT_EQ(readFileBytes(path), expected);
}

TEST(CrashReporterTest, AppendAttachmentRejectsMissingFile)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    ASSERT_FALSE(CrashReporter::appendAttachmentToEnvelope(dir.filePath(QStringLiteral("missing.envelope")),
                                                           QStringLiteral("crash-log.txt"), QByteArrayLiteral("x")));
}

TEST(CrashReporterTest, AppendAttachmentLengthIsByteAccurate)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("event.envelope"));
    ASSERT_TRUE(writeFileBytes(path, QByteArrayLiteral("{}")));

    const QByteArray utf8Payload = QString::fromUtf8("héllo").toUtf8();
    ASSERT_EQ(utf8Payload.size(), 6); // 5 characters, "é" is 2 bytes
    ASSERT_TRUE(CrashReporter::appendAttachmentToEnvelope(path, QStringLiteral("crash-log.txt"), utf8Payload));
    const QByteArray content = readFileBytes(path);
    ASSERT_TRUE(content.contains(QByteArrayLiteral("\"length\":6")));
    ASSERT_TRUE(content.contains(QByteArrayLiteral("\"attachment_length\":6")));
    ASSERT_TRUE(content.endsWith(utf8Payload));
}

// The SDK's envelope re-parser frames each item's payload by its `length` header; a
// missing or wrong length ends the payload at the first newline, so the remaining log
// lines get parsed as item headers and the whole envelope is rejected (and deleted).
TEST(CrashReporterTest, AppendAttachmentKeepsMultiLinePayloadIntact)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("event.envelope"));
    ASSERT_TRUE(writeFileBytes(path, QByteArrayLiteral("{}")));

    const QByteArray payload = QByteArrayLiteral("first line\nsecond line\nthird line");
    ASSERT_TRUE(CrashReporter::appendAttachmentToEnvelope(path, QStringLiteral("crash-log.txt"), payload));
    const QByteArray content = readFileBytes(path);
    ASSERT_TRUE(content.contains("\"length\":" + QByteArray::number(payload.size())));
    ASSERT_TRUE(content.contains("\"attachment_length\":" + QByteArray::number(payload.size())));
    ASSERT_TRUE(content.endsWith(payload));
}

TEST(CrashReporterTest, PendingEnvelopePathsMatchesSdkNamingInCacheAndRunDirs)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString dbPath = dir.path();
    ASSERT_TRUE(QDir().mkpath(dbPath + QStringLiteral("/cache")));
    ASSERT_TRUE(QDir().mkpath(dbPath + QStringLiteral("/aaaa.run")));
    ASSERT_TRUE(QDir().mkpath(dbPath + QStringLiteral("/other")));

    const QString retryName = QStringLiteral("1700000000000-0-") + uuidOf('a') + QStringLiteral(".envelope");
    const QString bareName = uuidOf('b') + QStringLiteral(".envelope");
    // Valid: retry-style cache file + bare run-dir file.
    ASSERT_TRUE(writeFileBytes(dbPath + QStringLiteral("/cache/") + retryName, QByteArrayLiteral("{}")));
    ASSERT_TRUE(writeFileBytes(dbPath + QStringLiteral("/aaaa.run/") + bareName, QByteArrayLiteral("{}")));
    // Rejected: junk names and the SDK's own marker file.
    ASSERT_TRUE(writeFileBytes(dbPath + QStringLiteral("/cache/") + uuidOf('c') + QStringLiteral(".envelope.bak"),
                               QByteArrayLiteral("{}")));
    ASSERT_TRUE(writeFileBytes(dbPath + QStringLiteral("/cache/") + QStringLiteral("random.envelope"),
                               QByteArrayLiteral("{}")));
    ASSERT_TRUE(writeFileBytes(dbPath + QStringLiteral("/aaaa.run/") + QStringLiteral("__sentry-crash.envelope"),
                               QByteArrayLiteral("{}")));
    ASSERT_TRUE(writeFileBytes(dbPath + QStringLiteral("/other/") + bareName, QByteArrayLiteral("{}")));

    const QStringList found = CrashReporter::pendingEnvelopePaths(dbPath);
    ASSERT_EQ(found.size(), 2);
    ASSERT_TRUE(found.contains(dbPath + QStringLiteral("/cache/") + retryName));
    ASSERT_TRUE(found.contains(dbPath + QStringLiteral("/aaaa.run/") + bareName));
}

TEST(CrashReporterTest, PendingEnvelopePathsWithoutDatabaseIsEmpty)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    ASSERT_TRUE(CrashReporter::pendingEnvelopePaths(dir.filePath(QStringLiteral("no-such-db"))).isEmpty());
    ASSERT_TRUE(CrashReporter::pendingEnvelopePaths(QString()).isEmpty());
}

TEST(CrashReporterTest, AttachFoldsLogAndNoteIntoPendingEnvelopes)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString dbPath = dir.path();
    ASSERT_TRUE(QDir().mkpath(dbPath + QStringLiteral("/cache")));
    const QString envelopePath = dbPath + QStringLiteral("/cache/") + uuidOf('d') + QStringLiteral(".envelope");
    ASSERT_TRUE(writeFileBytes(envelopePath, QByteArrayLiteral("{\"event_id\":\"x\"}\n{}\n")));

    CrashReporter reporter;
    // Empty DSN: no SDK, but init() still records the database path the dialog uses.
    reporter.init(QString(), QStringLiteral("3.1.2"), dbPath, PersonalSettings::CrashReportingMode::Ask);
    ASSERT_EQ(reporter.attachToPendingCrashEnvelopes(QStringLiteral("log line"), QStringLiteral("my note")), 1);

    const QByteArray content = readFileBytes(envelopePath);
    ASSERT_TRUE(content.startsWith(QByteArrayLiteral("{\"event_id\":\"x\"}\n{}\n")));
    ASSERT_TRUE(content.contains(QByteArrayLiteral("\"filename\":\"crash-log.txt\"")));
    ASSERT_TRUE(content.contains(QByteArrayLiteral("\nlog line")));
    ASSERT_TRUE(content.contains(QByteArrayLiteral("\"filename\":\"user-note.txt\"")));
    ASSERT_TRUE(content.endsWith(QByteArrayLiteral("\nmy note")));
}

TEST(CrashReporterTest, AttachSkipsNoteWhenEmptyAndFindsRunDirEnvelopes)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString dbPath = dir.path();
    ASSERT_TRUE(QDir().mkpath(dbPath + QStringLiteral("/bbbb.run")));
    const QString envelopePath = dbPath + QStringLiteral("/bbbb.run/") + uuidOf('e') + QStringLiteral(".envelope");
    ASSERT_TRUE(writeFileBytes(envelopePath, QByteArrayLiteral("{}")));

    // Static overload (the pre-init AlwaysSend path) with an empty note.
    ASSERT_EQ(CrashReporter::attachToPendingCrashEnvelopes(dbPath, QStringLiteral("log only"), QString()), 1);
    const QByteArray content = readFileBytes(envelopePath);
    ASSERT_TRUE(content.contains(QByteArrayLiteral("\"filename\":\"crash-log.txt\"")));
    ASSERT_FALSE(content.contains(QByteArrayLiteral("user-note.txt")));
}

} // namespace

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
