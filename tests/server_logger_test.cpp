#include "server_logger.h"

#include "gtest/gtest.h"
#include <QFile>
#include <QMetaObject>
#include <QTemporaryDir>
#include <iostream>
#include <sstream>
#include <streambuf>

namespace
{

/**
 * @brief Redirects std::cout for the lifetime of the object so the logger's console
 *        sink can be asserted on.
 */
class CoutCapture
{
public:
    CoutCapture() : previousBuffer(std::cout.rdbuf(captured.rdbuf()))
    {
    }
    ~CoutCapture()
    {
        std::cout.rdbuf(previousBuffer);
    }

    QString text() const
    {
        return QString::fromStdString(captured.str());
    }

private:
    std::ostringstream captured;
    std::streambuf *previousBuffer;
};

// The console sink is written by the logger thread; drive it directly here, as the event
// loop would in the server.
void flushLogger(ServerLogger &logger)
{
    QMetaObject::invokeMethod(&logger, "flushBuffer", Qt::DirectConnection);
}

// Regression: --log-to-console used to be gated behind a successfully opened logfile, so an
// empty server/logfile silenced every diagnostic, including critical startup errors.
TEST(ServerLoggerTest, ConsoleSinkDoesNotDependOnLogFile)
{
    ServerLogger logger(true, true, QString());
    logger.startLog(QString());

    CoutCapture capture;
    logger.logMessage("could not open the database");
    flushLogger(logger);

    EXPECT_TRUE(capture.text().contains("could not open the database")) << capture.text().toStdString();
}

// writelog only describes the logfile; disabling it must not silence the console.
TEST(ServerLoggerTest, ConsoleSinkSurvivesDisabledFileLog)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());

    ServerLogger logger(true, false, QString());
    logger.startLog(dir.filePath("server.log"));

    CoutCapture capture;
    logger.logMessage("startup failed");
    flushLogger(logger);

    EXPECT_TRUE(capture.text().contains("startup failed")) << capture.text().toStdString();
}

// logfilters selects what goes into the logfile; the console is not filtered.
TEST(ServerLoggerTest, ConsoleSinkIgnoresLogFilters)
{
    ServerLogger logger(true, true, "database");
    logger.startLog(QString());

    CoutCapture capture;
    logger.logMessage("unrelated message");
    flushLogger(logger);

    EXPECT_TRUE(capture.text().contains("unrelated message")) << capture.text().toStdString();
}

TEST(ServerLoggerTest, StaysQuietWhenConsoleIsDisabled)
{
    ServerLogger logger(false, false, QString());
    logger.startLog(QString());

    CoutCapture capture;
    logger.logMessage("not for the console");
    flushLogger(logger);

    EXPECT_TRUE(capture.text().isEmpty()) << capture.text().toStdString();
}

TEST(ServerLoggerTest, WritesMatchingFilteredMessagesToTheLogFile)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString logPath = dir.filePath("server.log");

    {
        ServerLogger logger(false, true, "database");
        logger.startLog(logPath);
        logger.logMessage("database is unavailable");
        logger.logMessage("unrelated message");
    }

    QFile logFile(logPath);
    ASSERT_TRUE(logFile.open(QIODevice::ReadOnly));
    const QString contents = QString::fromUtf8(logFile.readAll());
    EXPECT_TRUE(contents.contains("database is unavailable")) << contents.toStdString();
    EXPECT_FALSE(contents.contains("unrelated message")) << contents.toStdString();
}

// SIGHUP reloads the configuration; the logger must pick up the new filter.
TEST(ServerLoggerTest, ReloadConfigAppliesANewFilter)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString logPath = dir.filePath("server.log");

    {
        ServerLogger logger(false, true, "database");
        logger.startLog(logPath);
        logger.logMessage("filtered out");
        logger.reloadConfig(true, QString());
        logger.logMessage("kept after reload");
    }

    QFile logFile(logPath);
    ASSERT_TRUE(logFile.open(QIODevice::ReadOnly));
    const QString contents = QString::fromUtf8(logFile.readAll());
    EXPECT_FALSE(contents.contains("filtered out")) << contents.toStdString();
    EXPECT_TRUE(contents.contains("kept after reload")) << contents.toStdString();
}

TEST(ServerLoggerTest, DisabledFileLogLeavesTheLogFileEmpty)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString logPath = dir.filePath("server.log");

    {
        ServerLogger logger(false, false, QString());
        logger.startLog(logPath);
        logger.logMessage("not for the logfile");
    }

    QFile logFile(logPath);
    ASSERT_TRUE(logFile.open(QIODevice::ReadOnly));
    EXPECT_TRUE(logFile.readAll().isEmpty());
}

// A filter list consisting only of separators activates filtering with no entries; the old
// trimmed-empty check suppressed the logfile in that case and this preserves it.
TEST(ServerLoggerTest, DegenerateFilterSuppressesTheLogFile)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString logPath = dir.filePath("server.log");

    {
        ServerLogger logger(false, true, ",");
        logger.startLog(logPath);
        logger.logMessage("anything");
    }

    QFile logFile(logPath);
    ASSERT_TRUE(logFile.open(QIODevice::ReadOnly));
    EXPECT_TRUE(logFile.readAll().isEmpty());
}

} // namespace
