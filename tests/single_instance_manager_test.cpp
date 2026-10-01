#include "single_instance_manager.h"

#include <QCoreApplication>
#include <QDataStream>
#include <QEventLoop>
#include <QLocalServer>
#include <QLocalSocket>
#include <QTimer>
#include <gtest/gtest.h>

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
// The forwarded payload used to be handed to the handlers straight from inside
// the forwarding socket's readyRead emission. A handler can spin a nested event
// loop (the "Join game?" confirmation is a modal QMessageBox), and the sending
// instance exits as soon as it has the acknowledgment, so the socket was
// disconnected and deleteLater'd while that loop ran: the socket died in the
// middle of its own readyRead signal, which both this lambda and Qt's signal
// dispatch then kept using.
TEST(SingleInstanceManagerTest, PayloadIsHandledAfterTheForwardingSocketIsClosed)
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
    bool socketClosedWhenHandled = false;

    QEventLoop loop;
    QObject::connect(&primary, &SingleInstanceManager::filesReceived, &primary, [&](const QStringList &files) {
        ++handlerCalls;

        // The forwarding socket must already be closing by the time a handler
        // runs: a modal handler spins a nested event loop during which the
        // sending instance closes its end, and the socket would then be deleted
        // in the middle of its own readyRead emission.
        auto *connection = server->findChild<QLocalSocket *>();
        socketClosedWhenHandled = connection == nullptr || connection->state() != QLocalSocket::ConnectedState;

        // The primary acknowledges the payload before handling it, so that a
        // sender waiting on the acknowledgment never mistakes a busy primary
        // for a dead one.
        sender.waitForReadyRead(HANDSHAKE_TIMEOUT_MS);
        acknowledgment = sender.readAll();

        // Stand in for the modal "Join game?" confirmation: the sending instance
        // goes away while this nested loop is running.
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
    EXPECT_TRUE(socketClosedWhenHandled) << "the payload was handled while the forwarding socket was still connected";
    EXPECT_FALSE(acknowledgment.isEmpty()) << "the primary never acknowledged the payload";
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
