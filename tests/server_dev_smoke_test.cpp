/** @file server_dev_smoke_test.cpp
 *  @brief End-to-end smoke test: a real `servatrice-dev` process driven by a real `RemoteClient`.
 *  @ingroup Tests
 */

#include "abstract_client.h"
#include "libcockatrice/interfaces/interface_network_settings_provider.h"
#include "libcockatrice/protocol/pb/event_game_joined.pb.h"
#include "libcockatrice/protocol/pb/event_list_rooms.pb.h"
#include "libcockatrice/protocol/pb/response.pb.h"
#include "libcockatrice/protocol/pb/room_commands.pb.h"
#include "libcockatrice/protocol/pb/serverinfo_room.pb.h"
#include "libcockatrice/protocol/pb/session_commands.pb.h"
#include "libcockatrice/protocol/pending_command.h"
#include "remote_client.h"

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QList>
#include <QProcess>
#include <QProcessEnvironment>
#include <QString>
#include <QTemporaryDir>
#include <QThread>
#include <QVariant>
#include <functional>
#include <gtest/gtest.h>
#include <memory>

namespace
{

// Bounded so the worst-case failure path stays inside the 60 s CTest timeout the test macro
// applies, while a local-loopback round trip still gets far more time than it needs.
constexpr int kStartupTimeoutMs = 15000;
constexpr int kProtocolTimeoutMs = 5000;

/** @brief The settings a real socket needs, with no client UI to read them from. */
class StubNetworkSettingsProvider : public INetworkSettingsProvider
{
public:
    QString getClientID() override
    {
        return QStringLiteral("servatrice-dev-smoke-test");
    }
    [[nodiscard]] int getTimeOut() const override
    {
        return 60;
    }
    [[nodiscard]] int getKeepAlive() const override
    {
        return 60;
    }
    [[nodiscard]] bool getNotifyAboutUpdates() const override
    {
        return false;
    }
    void setKnownMissingFeatures(const QString &) override
    {
    }
    QString getKnownMissingFeatures() override
    {
        return {};
    }
};

/**
 * @brief Pumps the Qt event loop until `done()` reports success or the timeout expires.
 *
 * The client runs on this thread, so its socket reads and signal emissions need a running
 * event loop; a signal-driven QEventLoop would need one per awaited condition.
 */
bool waitFor(const std::function<bool()> &done, int timeoutMs)
{
    QElapsedTimer elapsed;
    elapsed.start();
    while (elapsed.elapsed() < timeoutMs) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
        if (done()) {
            return true;
        }
        QThread::msleep(5);
    }
    QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
    return done();
}

class ServerDevSmokeTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        ASSERT_TRUE(tempDir.isValid());

        // servatrice-dev writes its throwaway settings under QDir::tempPath(); pointing that at
        // a directory the test owns means a terminated server cannot leave an ini behind.
        QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
        for (const QString &variable : {QStringLiteral("TMPDIR"), QStringLiteral("TMP"), QStringLiteral("TEMP")}) {
            environment.insert(variable, tempDir.path());
        }
        process.setProcessEnvironment(environment);
        process.setProcessChannelMode(QProcess::SeparateChannels);
        process.start(QStringLiteral(SERVATRICE_DEV_BINARY),
                      {QStringLiteral("--port"), QStringLiteral("0"), QStringLiteral("--seed"), QStringLiteral("42")});
        ASSERT_TRUE(process.waitForStarted(kStartupTimeoutMs)) << process.errorString().toStdString();
        ASSERT_TRUE(readListeningPort()) << "servatrice-dev did not announce a listening port.\nstdout:\n"
                                         << stdoutText.toStdString() << "\nstderr:\n"
                                         << stderrText.toStdString();

        client = std::make_unique<RemoteClient>(nullptr, &provider);
    }

    void TearDown() override
    {
        if (client) {
            client->disconnectFromServer();
            waitFor([this] { return client->getStatus() == StatusDisconnected; }, 2000);
        }
        client.reset();

        if (process.state() != QProcess::NotRunning) {
            process.terminate();
            if (!process.waitForFinished(2000)) {
                process.kill();
                process.waitForFinished(2000);
            }
        }
    }

    /// @brief Reads the child's output until it announces the ephemeral port it listens on.
    bool readListeningPort()
    {
        const QString prefix = QStringLiteral("servatrice-dev: listening on port ");
        QElapsedTimer elapsed;
        elapsed.start();
        while (elapsed.elapsed() < kStartupTimeoutMs) {
            if (!process.waitForReadyRead(100)) {
                if (process.state() == QProcess::NotRunning) {
                    drainProcessOutput();
                    return false;
                }
                continue;
            }
            drainProcessOutput();

            const int at = stdoutText.indexOf(prefix);
            if (at < 0) {
                continue;
            }
            const int end = stdoutText.indexOf(QLatin1Char('\n'), at);
            if (end < 0) {
                continue;
            }
            bool ok = false;
            listeningPort = stdoutText.mid(at + prefix.size(), end - at - prefix.size()).trimmed().toInt(&ok);
            return ok && listeningPort > 0;
        }
        return false;
    }

    /// @brief Keeps the child's pipes drained so logging cannot block it mid-test.
    void drainProcessOutput()
    {
        stdoutText.append(QString::fromLocal8Bit(process.readAllStandardOutput()));
        stderrText.append(QString::fromLocal8Bit(process.readAllStandardError()));
    }

    /// @brief Waits for a client-side condition, giving up as soon as the child has exited.
    bool waitForClient(const std::function<bool()> &done, int timeoutMs)
    {
        return waitFor([&done, this] { return done() || process.state() == QProcess::NotRunning; }, timeoutMs);
    }

    /// @brief Logs in as a guest, which is the only identity AuthenticationNone offers.
    bool login()
    {
        QObject::connect(client.get(), &RemoteClient::statusChanged, [this](ClientStatus status) {
            if (status == StatusLoggedIn) {
                loggedIn = true;
            }
        });
        QObject::connect(client.get(), &RemoteClient::loginError,
                         [this](Response::ResponseCode code, const QString &reason, quint32, const QList<QString> &) {
                             protocolFailure = QStringLiteral("login failed: %1 (code %2)").arg(reason).arg(code);
                         });
        QObject::connect(client.get(), &RemoteClient::socketError, [this](const QString &error) {
            protocolFailure = QStringLiteral("socket error: %1").arg(error);
        });

        client->connectToServer(QStringLiteral("127.0.0.1"), static_cast<unsigned int>(listeningPort),
                                QStringLiteral("smoke-guest"), QString());
        return waitForClient([this] { return loggedIn || !protocolFailure.isEmpty(); }, kProtocolTimeoutMs) && loggedIn;
    }

    /// @brief Sends a session command and returns its response code.
    Response::ResponseCode sendSessionCommand(const google::protobuf::Message &command)
    {
        return awaitResponse(AbstractClient::prepareSessionCommand(command));
    }

    /// @brief Sends a room command and returns its response code.
    Response::ResponseCode sendRoomCommand(const google::protobuf::Message &command, int roomId)
    {
        return awaitResponse(AbstractClient::prepareRoomCommand(command, roomId));
    }

    Response::ResponseCode awaitResponse(PendingCommand *pending)
    {
        Response::ResponseCode code = Response::RespInvalidCommand;
        bool finished = false;
        const QMetaObject::Connection connection =
            QObject::connect(pending, &PendingCommand::finished,
                             [&code, &finished](const Response &response, const CommandContainer &, const QVariant &) {
                                 code = response.response_code();
                                 finished = true;
                             });
        client->sendCommand(pending);
        const bool answered = waitForClient([&finished] { return finished; }, kProtocolTimeoutMs);
        if (!answered) {
            // The command may still be in flight; a late response must not write to this frame.
            QObject::disconnect(connection);
        }
        EXPECT_TRUE(answered) << "no response to " << pending->getCommandContainer().DebugString();
        drainProcessOutput();
        return code;
    }

    QProcess process;
    QTemporaryDir tempDir;
    StubNetworkSettingsProvider provider;
    std::unique_ptr<RemoteClient> client;
    int listeningPort = 0;
    QString stdoutText;
    QString stderrText;
    QString protocolFailure;
    // State the client's signals write to. It has to outlive every connection, because teardown
    // disconnecting the client emits statusChanged again.
    bool loggedIn = false;
    QList<ServerInfo_Room> rooms;
    bool gameJoined = false;
    QString joinedDescription;
};

TEST_F(ServerDevSmokeTest, LogsInListsRoomsJoinsAndCreatesAGame)
{
    ASSERT_TRUE(login()) << protocolFailure.toStdString();

    QObject::connect(client.get(), &RemoteClient::listRoomsEventReceived, [this](const Event_ListRooms &event) {
        for (int i = 0; i < event.room_list_size(); ++i) {
            rooms.append(event.room_list(i));
        }
    });

    EXPECT_EQ(Response::RespOk, sendSessionCommand(Command_ListRooms()));
    ASSERT_TRUE(waitForClient([this] { return !rooms.isEmpty(); }, kProtocolTimeoutMs))
        << "the server never sent a room list";
    ASSERT_EQ(1, rooms.size());
    EXPECT_EQ(QStringLiteral("Dev Lobby"), QString::fromStdString(rooms.first().name()));
    const int roomId = rooms.first().room_id();
    ASSERT_GT(roomId, 0);

    Command_JoinRoom joinRoom;
    joinRoom.set_room_id(static_cast<google::protobuf::uint32>(roomId));
    EXPECT_EQ(Response::RespOk, sendSessionCommand(joinRoom));

    QObject::connect(client.get(), &RemoteClient::gameJoinedEventReceived, [this](const Event_GameJoined &event) {
        gameJoined = true;
        joinedDescription = QString::fromStdString(event.game_info().description());
    });

    Command_CreateGame createGame;
    createGame.set_description("servatrice-dev smoke game");
    createGame.set_max_players(2);
    createGame.set_spectators_allowed(true);
    EXPECT_EQ(Response::RespOk, sendRoomCommand(createGame, roomId));
    ASSERT_TRUE(waitForClient([this] { return gameJoined; }, kProtocolTimeoutMs))
        << "the creator was never joined to the game it created";
    EXPECT_EQ(QStringLiteral("servatrice-dev smoke game"), joinedDescription);
}

} // namespace

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
