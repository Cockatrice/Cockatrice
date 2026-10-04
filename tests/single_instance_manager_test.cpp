#include "single_instance_manager.h"

#include <QByteArray>
#include <QCoreApplication>
#include <QDataStream>
#include <QEventLoop>
#include <QIODevice>
#include <QList>
#include <QLocalServer>
#include <QLocalSocket>
#include <QStringLiteral>
#include <QTimer>
#include <QtEnvironmentVariables>
#include <gtest/gtest.h>
#include <qtypes.h>
#include <string>

namespace
{
constexpr int HANDSHAKE_TIMEOUT_MS = 5000;
constexpr int MODAL_DIALOG_MS = 50;

const QStringList PAYLOAD{QStringLiteral("cockatrice://joingame?hostname=127.0.0.1&port=4747&roomid=0&gameid=1")};

/// Serializes a payload the way SingleInstanceManager::forwardToPrimary() does.
QByteArray encodePayload(const QStringList &files)
{
    QByteArray payload;
    QDataStream out(&payload, QIODevice::WriteOnly);
    out << files;

    QByteArray message;
    QDataStream msgStream(&message, QIODevice::WriteOnly);
    msgStream << static_cast<quint32>(payload.size());
    message.append(payload);
    return message;
}
} // namespace

// Regression test for the use-after-free in SingleInstanceManager::handleNewConnection().
//
// The forwarded payload is handed to the handlers from inside the forwarding
// socket's readyRead emission, which is what lets the "Join game?" confirmation
// appear at all. A handler can spin a nested event loop (a modal QMessageBox),
// and the sending instance exits as soon as it has the acknowledgment, so the
// peer closes *while that loop runs*. deleteLater() called from inside such a
// nested loop does not wait for the outer loop: Qt frees the object as soon as
// the nested loop unwinds, still inside the readyRead emission, so Qt's own
// signal dispatch and the handler lambda both end up holding a freed socket.
// Nothing may therefore delete the socket from the disconnected signal, and the
// teardown is queued until that frame has unwound.
TEST(SingleInstanceManagerTest, HandlersRunSynchronouslyAndSocketIsNotTouchedAfterTheyReturn)
{
    // Scope the hand-off socket to this test process so a Cockatrice the
    // developer happens to be running cannot claim it.
    qputenv("USER", QByteArrayLiteral("cockatrice-test-") + QByteArray::number(QCoreApplication::applicationPid()));

    SingleInstanceManager primary;
    ASSERT_TRUE(primary.tryRun(QStringList())) << "could not become the primary instance";

    auto *server = primary.findChild<QLocalServer *>();
    ASSERT_NE(nullptr, server) << "the primary instance is not listening";

    QLocalSocket sender;
    QStringList handled;
    QByteArray acknowledgment;
    int handlerCalls = 0;
    bool socketConnectedWhenHandled = false;
    bool socketReleased = false;

    // Waited on after the handlers return: the deferred teardown deletes the
    // forwarding socket once the readyRead frame has unwound.
    QEventLoop teardownLoop;

    QEventLoop loop;
    QObject::connect(&primary, &SingleInstanceManager::filesReceived, &primary, [&](const QStringList &files) {
        ++handlerCalls;

        // The payload must reach the handlers while the forwarding socket is
        // still connected: deferring the emit would suppress the modal "Join
        // game?" confirmation this hand-off exists to show.
        auto *connection = server->findChild<QLocalSocket *>();
        socketConnectedWhenHandled = connection != nullptr && connection->state() == QLocalSocket::ConnectedState;

        // Observe the socket's destruction rather than polling for it. `destroyed`
        // is emitted just before the object goes away, so this stays correct
        // however the teardown ends up deleting it, and `&primary` keeps the
        // observer alive even once the socket itself is gone.
        if (connection != nullptr) {
            QObject::connect(connection, &QObject::destroyed, &primary, [&] {
                socketReleased = true;
                teardownLoop.quit();
            });
        }

        // The primary acknowledges the payload before handling it, so that a
        // sender waiting on the acknowledgment never mistakes a busy primary
        // for a dead one.
        sender.waitForReadyRead(HANDSHAKE_TIMEOUT_MS);
        acknowledgment = sender.readAll();

        // Stand in for the modal "Join game?" confirmation: the sending instance
        // goes away while this nested loop is running, which is what used to
        // destroy the socket underneath this readyRead frame.
        sender.abort();

        QEventLoop modalDialog;
        QTimer::singleShot(MODAL_DIALOG_MS, &modalDialog, &QEventLoop::quit);
        modalDialog.exec();

        handled = files;
        loop.quit();
    });

    sender.connectToServer(server->serverName());
    ASSERT_TRUE(sender.waitForConnected(HANDSHAKE_TIMEOUT_MS));
    sender.write(encodePayload(PAYLOAD));
    sender.flush();

    QTimer::singleShot(HANDSHAKE_TIMEOUT_MS, &loop, &QEventLoop::quit);
    loop.exec();

    EXPECT_EQ(1, handlerCalls) << "the payload must be handed to the handlers exactly once";
    EXPECT_EQ(PAYLOAD, handled);
    EXPECT_TRUE(socketConnectedWhenHandled) << "the payload was not handled from inside the readyRead emission";
    EXPECT_FALSE(acknowledgment.isEmpty()) << "the primary never acknowledged the payload";

    // The deferred teardown must not be skipped, and must not delete the socket
    // from inside the nested loop above: either would show up here as a crash or
    // as a socket that outlives the hand-off and leaks a connection per payload.
    QTimer::singleShot(HANDSHAKE_TIMEOUT_MS, &teardownLoop, &QEventLoop::quit);
    teardownLoop.exec();
    EXPECT_TRUE(socketReleased) << "the forwarding socket was never released after the hand-off";
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
