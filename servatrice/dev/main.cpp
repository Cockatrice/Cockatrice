/** @file main.cpp
 *  @brief Servatrice dev server: the real server startup on an in-memory database.
 */

#include "main.h"

#include "libcockatrice/rng/rng_abstract.h"
#include "libcockatrice/rng/rng_sfmt.h"
#include "libcockatrice/server/dev/servatrice_in_memory_database_interface.h"
#include "libcockatrice/utility/cryptoutil.h"
#include "servatrice.h"
#include "server_logger.h"
#include "server_room.h"
#include "settingscache.h"
#include "smtpclient.h"
#include "version_string.h"

#include <QByteArray>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QMetaObject>
#include <QSaveFile>
#include <QString>
#include <QThread>
#include <cstdlib>
#include <google/protobuf/stubs/common.h>
#include <iostream>

namespace
{

/** @brief The room the dev server always offers, so a client has somewhere to log in to. */
void seedDevDatabase(Servatrice_InMemoryDatabaseInterface &database)
{
    Server_DatabaseInterface::BootstrapRoomRecord lobby;
    lobby.id = 1;
    lobby.name = "Dev Lobby";
    lobby.description = "In-memory room for local development.";
    lobby.permissionLevel = "none";
    lobby.privateLevel = "none";
    lobby.autoJoin = true;
    lobby.chatHistorySize = 100;
    lobby.gameTypes = {"Commander"};
    database.addRoom(lobby);

    // AuthenticationNone on the shipped server reports every login as an unknown user, which is
    // what makes a client connect as a guest.
    database.setAuthenticationResult(UnknownUser);
}

/** @brief The shipped server with its database replaced by the in-memory implementation. */
class DevServatrice : public Servatrice
{
public:
    Servatrice_InMemoryDatabaseInterface *getInMemoryDatabase() const
    {
        return inMemoryDatabase;
    }

protected:
    Server_DatabaseInterface *createMainDatabaseInterface() override
    {
        inMemoryDatabase = new Servatrice_InMemoryDatabaseInterface(this);
        seedDevDatabase(*inMemoryDatabase);
        return inMemoryDatabase;
    }

private:
    Servatrice_InMemoryDatabaseInterface *inMemoryDatabase = nullptr;
};

/** @brief Routes Qt log output through the server logger, as the shipped binary does. */
void logThroughServerLogger(QtMsgType, const QMessageLogContext &, const QString &message)
{
    logger->logMessage(message);
}

void dumpState(Servatrice &server, const Servatrice_InMemoryDatabaseInterface &database)
{
    std::cout << "servatrice-dev: users=" << server.getUsersCount() << std::endl;

    for (const auto *room : server.getRooms()) {
        std::cout << "servatrice-dev: room id=" << room->getId() << " name=\"" << room->getName().toStdString() << "\""
                  << std::endl;
    }

    std::cout << "servatrice-dev: log entries=" << database.getLogEntries().size() << std::endl;
}

/**
 * @brief Publishes the bound port the way a script can read it.
 *
 * The stdout announcement is written from this thread while the logger thread writes to the same
 * stream, so a reader can see a log line inside it. A file written through QSaveFile is either
 * absent or complete, which makes the port deterministic to consume.
 */
bool writeListeningPort(const QString &path, int port)
{
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }
    file.write(QByteArray::number(port) + '\n');
    return file.commit();
}

} // namespace

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setOrganizationName("Cockatrice");
    QCoreApplication::setApplicationName("Servatrice-dev");
    QCoreApplication::setApplicationVersion(VERSION_STRING);

    QCommandLineParser parser;
    parser.setApplicationDescription("Servatrice on an in-memory database.");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption portOption({"p", "port"}, "TCP port to listen on; 0 picks an ephemeral port.", "port", "0");
    QCommandLineOption portFileOption("port-file",
                                      "Write the bound listening port to this file once the server listens.",
                                      "path");
    QCommandLineOption seedOption("seed", "Seed for the game RNG; random when omitted.", "seed");
    QCommandLineOption dumpStateOption("dump-state", "Print the server state after startup and keep serving.");
    parser.addOption(portOption);
    parser.addOption(portFileOption);
    parser.addOption(seedOption);
    parser.addOption(dumpStateOption);
    parser.process(app);

    bool portOk = false;
    const int port = parser.value(portOption).toInt(&portOk);
    if (!portOk || port < 0 || port > 65535) {
        std::cerr << "servatrice-dev: invalid port" << std::endl;
        return EXIT_FAILURE;
    }

    const QString seedValue = parser.value(seedOption);
    bool seedOk = false;
    const quint32 seed =
        seedValue.isEmpty() ? static_cast<quint32>(CryptoUtil::randomUInt64()) : seedValue.toUInt(&seedOk);
    if (!seedValue.isEmpty() && !seedOk) {
        std::cerr << "servatrice-dev: invalid seed" << std::endl;
        return EXIT_FAILURE;
    }

    // The shipped server treats DATABASE_URL as "use MySQL"; the dev server has no database.
    qunsetenv("DATABASE_URL");

    // Never touch the user's servatrice.ini: the dev server writes its throwaway settings to a
    // temporary file it deletes on exit.
    const QString configPath =
        QDir(QDir::tempPath())
            .filePath("servatrice-dev-" + QString::number(QCoreApplication::applicationPid()) + ".ini");
    settingsCache = new SettingsCache(configPath);
    settingsCache->setValue("server/port", port);
    settingsCache->setValue("server/number_pools", 1);
    settingsCache->setValue("server/websocket_number_pools", 0);
    settingsCache->setValue("database/type", "none");
    settingsCache->setValue("authentication/method", "none");
    settingsCache->setValue("rooms/method", "sql");
    settingsCache->setValue("server/writelog", false);
    settingsCache->sync();

    loggerThread = new QThread;
    loggerThread->setObjectName("logger");
    logger = new ServerLogger(true, false, QString());
    logger->moveToThread(loggerThread);
    loggerThread->start();
    QMetaObject::invokeMethod(logger, "startLog", Qt::BlockingQueuedConnection, Q_ARG(QString, QString()));
    qInstallMessageHandler(logThroughServerLogger);

    std::cout << "servatrice-dev: rng seed=" << seed << std::endl;
    rng = new RNG_SFMT(seed);
    smtpClient = new SmtpClient();

    auto *server = new DevServatrice();
    QObject::connect(server, &QObject::destroyed, &app, &QCoreApplication::quit, Qt::QueuedConnection);

    int retval = EXIT_SUCCESS;
    if (!server->initServer()) {
        std::cerr << "servatrice-dev: initialization failed" << std::endl;
        retval = EXIT_FAILURE;
    } else {
        const int listeningPort = server->getGameServerPort();
        if (listeningPort <= 0) {
            std::cerr << "servatrice-dev: the server is not listening" << std::endl;
            retval = EXIT_FAILURE;
        } else {
            const QString portFilePath = parser.value(portFileOption);
            if (!portFilePath.isEmpty() && !writeListeningPort(portFilePath, listeningPort)) {
                std::cerr << "servatrice-dev: cannot write the listening port to " << portFilePath.toStdString()
                          << std::endl;
                retval = EXIT_FAILURE;
            } else {
                std::cout << "servatrice-dev: listening on port " << listeningPort << std::endl;
                if (parser.isSet(dumpStateOption)) {
                    dumpState(*server, *server->getInMemoryDatabase());
                }
                retval = QCoreApplication::exec();
            }
        }
    }

    qInstallMessageHandler(nullptr);

    delete smtpClient;
    delete rng;
    delete settingsCache;
    QFile::remove(configPath);

    logger->deleteLater();
    loggerThread->wait();
    delete loggerThread;

    google::protobuf::ShutdownProtobufLibrary();
    return retval;
}
