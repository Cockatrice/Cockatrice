#include "client/crash/crash_reporter.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QLockFile>
#include <QString>
#include <QSysInfo>
#include <QTemporaryDir>
#include <QTextStream>
#include <gtest/gtest.h>
#include <libcockatrice/settings/debug_settings.h>
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
                  static_cast<int>(DebugSettings::CrashReportingMode::Ask));
    ASSERT_FALSE(reporter.isReady());
    reporter.shutdown();
    ASSERT_FALSE(reporter.isReady());
}

TEST(CrashReporterTest, InitSkippedWhenNeverSend)
{
    CrashReporter reporter;
    reporter.init(QStringLiteral("https://key@sentry.io/1"), QStringLiteral("3.1.2"),
                  QDir::tempPath() + QStringLiteral("/cockatrice-crash-test"),
                  static_cast<int>(DebugSettings::CrashReportingMode::NeverSend));
    ASSERT_FALSE(reporter.isReady());
    reporter.shutdown();
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
                  static_cast<int>(DebugSettings::CrashReportingMode::Ask));
    ASSERT_FALSE(reporter.isReady());
    reporter.giveConsent();
    reporter.revokeConsent();
    ASSERT_FALSE(reporter.isReady());
    ASSERT_FALSE(reporter.sendReport(QStringLiteral("preview")));
    ASSERT_FALSE(reporter.sendReport(QString()));
    reporter.shutdown();
    ASSERT_FALSE(reporter.isReady());
}

TEST(CrashReporterTest, BeginSessionOnEmptyDirReportsClean)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    CrashReporter reporter;
    ASSERT_FALSE(reporter.beginSession(dir.path()));
    reporter.endSession();
}

TEST(CrashReporterTest, BeginSessionIgnoresLiveConcurrentInstance)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    // A second QLockFile held in-process stands in for a live concurrent instance: same
    // pid, alive, so the scan must leave it alone and report a clean start.
    QLockFile liveInstance(dir.filePath(QStringLiteral("session-424242")));
    ASSERT_TRUE(liveInstance.tryLock(0));
    CrashReporter reporter;
    ASSERT_FALSE(reporter.beginSession(dir.path()));
    ASSERT_TRUE(QFile::exists(liveInstance.fileName()));
    reporter.endSession();
    liveInstance.unlock();
}

TEST(CrashReporterTest, BeginSessionDetectsStaleDeadOwnerLock)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    // A well-formed lock whose owner is dead: our own pid (so the host matches) but a
    // process name that cannot match, which is how QLockFile spots pid reuse. Must be
    // reported unclean and cleaned up, even though the file is brand new (this is why
    // the scan probes with staleLockTime 0 rather than the 30 s default).
    const QString stalePath = dir.filePath(QStringLiteral("session-stale-owner"));
    QFile stale(stalePath);
    ASSERT_TRUE(stale.open(QIODevice::WriteOnly | QIODevice::Text));
    QTextStream out(&stale);
    out << QCoreApplication::applicationPid() << '\n'
        << QStringLiteral("definitely-not-the-test-binary") << '\n'
        << QSysInfo::machineHostName() << '\n';
    stale.close();

    CrashReporter reporter;
    ASSERT_TRUE(reporter.beginSession(dir.path()));
    ASSERT_FALSE(QFile::exists(stalePath));
    reporter.endSession();
}

TEST(CrashReporterTest, BeginSessionCleansGarbageFileAndReportsUnclean)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    // An unreadable lock file has no live owner by definition: report unclean and remove it.
    const QString garbagePath = dir.filePath(QStringLiteral("session-garbage"));
    QFile garbage(garbagePath);
    ASSERT_TRUE(garbage.open(QIODevice::WriteOnly));
    garbage.write("not-a-lock-file\x00\xff", 17);
    garbage.close();

    CrashReporter reporter;
    ASSERT_TRUE(reporter.beginSession(dir.path()));
    ASSERT_FALSE(QFile::exists(garbagePath));
    reporter.endSession();
}

TEST(CrashReporterTest, BeginSessionEndSessionRoundTripReportsClean)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    CrashReporter reporter;
    ASSERT_FALSE(reporter.beginSession(dir.path()));
    reporter.endSession();
    ASSERT_FALSE(reporter.beginSession(dir.path()));
    reporter.endSession();
}

} // namespace

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
