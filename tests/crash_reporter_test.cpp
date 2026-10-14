#include "client/crash/crash_reporter.h"

#include <QDir>
#include <QString>
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

} // namespace

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
