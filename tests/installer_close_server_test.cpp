#include "installer_close_name.h"
#include "installer_close_server.h"

#include <QByteArray>
#include <QCoreApplication>
#include <QEventLoop>
#include <QLocalSocket>
#include <QString>
#include <QTimer>
#include <QtGlobal>
#include <gtest/gtest.h>
#include <libcockatrice/utility/installer_close_protocol.h>
#include <string>

namespace
{
constexpr int HANDSHAKE_TIMEOUT_MS = 5000;

/// The bytes both ends exchange, from the header they both include. The separate
/// WireCommandIsThePinnedBytes test is what catches an accidental change to the literal.
const QByteArray CLOSE_COMMAND = QByteArray::fromRawData(INSTALLER_CLOSE_COMMAND, sizeof(INSTALLER_CLOSE_COMMAND) - 1);

/// Scopes the socket to this test process, so a Cockatrice the developer happens to be
/// running cannot claim the name and the test cannot close it. Kept short on purpose:
/// QLocalServer on Unix puts the socket file under QDir::tempPath(), and macOS caps that
/// path at 104 bytes (sun_path). A longer name passes on Linux (/tmp) and fails to listen
/// on the macOS CI runners, whose temp directory is already ~48 characters.
QByteArray testUserName()
{
    return QByteArrayLiteral("t") + QByteArray::number(QCoreApplication::applicationPid());
}
} // namespace

// The probe builds the name from the process id and the account name it reads from the
// environment; the client builds it with scopedLocalServerName(). They have to produce the
// same string or the installer can never reach the instance it is trying to close.
TEST(InstallerCloseServerTest, SocketNameMatchesTheProbeForTheSameProcessAndUser)
{
    constexpr unsigned long PROCESS_ID = 4242;
    const QString userName = QStringLiteral("installer-test");
    const QString expected = QStringLiteral("CockatriceInstallerClose-4242-installer-test");

    EXPECT_EQ(expected, InstallerCloseServer::socketNameForUser(PROCESS_ID, userName));
    EXPECT_EQ(expected.toStdWString(), installerCloseSocketName(PROCESS_ID, userName.toStdWString()));
}

TEST(InstallerCloseServerTest, SocketNameScopesTheProcessIdByTheCurrentUser)
{
    qputenv("USER", testUserName());

    EXPECT_EQ(QStringLiteral("CockatriceInstallerClose-4242-") + QString::fromLatin1(testUserName()),
              InstallerCloseServer::socketName(4242));
}

// The line is a wire format shared with the Windows-only probe, which cannot be compiled
// into this test. Pinning the exact bytes here makes a change to the shared header a
// deliberate protocol change rather than a typo that only shows up in the installer.
TEST(InstallerCloseServerTest, WireCommandIsThePinnedBytes)
{
    EXPECT_EQ(QByteArrayLiteral("COCKATRICE_INSTALLER_CLOSE\n"), CLOSE_COMMAND);
    EXPECT_EQ(27, CLOSE_COMMAND.size());
}

// The installer only writes the command; the listening instance is what turns it into a
// close. Anything but that exact line must not be treated as a request.
TEST(InstallerCloseServerTest, CloseRequestIsEmittedForTheProbeCommand)
{
    qputenv("USER", testUserName());

    InstallerCloseServer server;
    ASSERT_TRUE(server.listen()) << "the installer close socket could not be started";

    int closeRequests = 0;
    QObject::connect(&server, &InstallerCloseServer::closeRequested, &server, [&closeRequests]() { ++closeRequests; });

    QLocalSocket socket;
    socket.connectToServer(InstallerCloseServer::socketName(QCoreApplication::applicationPid()));
    ASSERT_TRUE(socket.waitForConnected(HANDSHAKE_TIMEOUT_MS)) << "could not reach the installer close socket";
    ASSERT_EQ(CLOSE_COMMAND.size(), socket.write(CLOSE_COMMAND)) << "the close request was not accepted";

    // No waitForBytesWritten(): on Windows QLocalSocket completes the write through the
    // event dispatcher, and this test runs the server in the same thread, so a blocking
    // wait would hold the loop the server needs to accept and read. The loop below is the
    // real handshake: it flushes the write and delivers the server's readyRead.
    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    QObject::connect(&server, &InstallerCloseServer::closeRequested, &loop, &QEventLoop::quit);
    timeout.start(HANDSHAKE_TIMEOUT_MS);
    loop.exec();

    EXPECT_EQ(1, closeRequests) << "the probe's line must reach the close handler exactly once";
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
