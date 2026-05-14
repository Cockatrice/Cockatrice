#include "single_instance_manager.h"

#include <QDataStream>
#include <QLoggingCategory>
#include <QTimer>
#include <memory>

Q_LOGGING_CATEGORY(SingleInstanceLog, "single_instance")

SingleInstanceManager::SingleInstanceManager(const QString &socketName, QObject *parent)
    : QObject(parent), socketName(socketName)
{
}

SingleInstanceManager::~SingleInstanceManager() = default;

QString SingleInstanceManager::perUserSocketName(const QString &base)
{
#ifdef Q_OS_WIN
    const QByteArray user = qgetenv("USERNAME");
#else
    const QByteArray user = qgetenv("USER");
#endif
    if (user.isEmpty())
        return base;
    return base + QStringLiteral("-") + QString::fromLocal8Bit(user);
}

void SingleInstanceManager::becomePrimary()
{
    if (server)
        return; // already listening — idempotent
    QLocalServer::removeServer(socketName);
    server = new QLocalServer(this);
    if (!server->listen(socketName)) {
        qCWarning(SingleInstanceLog) << "Failed to start local server:" << server->errorString();
        return;
    }
    connect(server, &QLocalServer::newConnection, this, &SingleInstanceManager::onNewConnection);
}

void SingleInstanceManager::resolveStartupRole(const QString &maybeUrl)
{
    if (maybeUrl.isEmpty()) {
        // No URL to forward — just try to become primary.  Defer the signal
        // via a queued emission so the contract ("emits roleResolved exactly
        // once, asynchronously") holds uniformly across both branches.
        becomePrimary();
        QMetaObject::invokeMethod(this, [this] { emit roleResolved(false); }, Qt::QueuedConnection);
        return;
    }

    // Probe an existing primary.  Lifetime: probe and timer are owned by
    // *this*; the terminal slot deleteLater()s them.  A shared "resolved"
    // flag prevents double emission if multiple signals race (errorOccurred
    // can fire after readyRead, etc.).  shared_ptr so the flag outlives
    // whichever lambda fires last.
    auto *probe = new QLocalSocket(this);
    auto *timer = new QTimer(this);
    auto resolved = std::make_shared<bool>(false);

    auto finish = [this, probe, timer, resolved](bool forwarded) {
        if (*resolved)
            return;
        *resolved = true;
        timer->stop();
        probe->deleteLater();
        timer->deleteLater();
        if (!forwarded)
            becomePrimary();
        emit roleResolved(forwarded);
    };

    connect(probe, &QLocalSocket::connected, this, [probe, maybeUrl] {
        QDataStream stream(probe);
        stream.setVersion(QDataStream::Qt_5_0);
        stream << maybeUrl;
        probe->flush();
    });

    connect(probe, &QLocalSocket::readyRead, this, [probe, finish] {
        QDataStream in(probe);
        in.setVersion(QDataStream::Qt_5_0);
        in.startTransaction();
        QString ack;
        in >> ack;
        if (!in.commitTransaction())
            return; // partial ACK — wait for readyRead to fire again with the rest.
        finish(true);
    });

    connect(probe, &QLocalSocket::errorOccurred, this, [finish](QLocalSocket::LocalSocketError) {
        // No primary at this socket — become primary ourselves.
        finish(false);
    });

    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, this, [finish] {
        // Primary unresponsive (e.g. stale socket from a dead old primary).
        // Become primary and hope for the best.
        qCWarning(SingleInstanceLog) << "Timed out forwarding URL; becoming primary";
        finish(false);
    });
    timer->start(ForwardTimeoutMs);

    probe->connectToServer(socketName);
}

void SingleInstanceManager::onNewConnection()
{
    while (server->hasPendingConnections()) {
        QLocalSocket *socket = server->nextPendingConnection();
        connect(socket, &QLocalSocket::readyRead, this, [this, socket]() { processConnection(socket); });
        connect(socket, &QLocalSocket::disconnected, socket, &QLocalSocket::deleteLater);

        // The secondary may have written its URL and the bytes may have
        // arrived before we got here.  readyRead has already fired once for
        // them with no slot connected — drain pre-buffered bytes now so the
        // payload doesn't sit unread forever.
        if (socket->bytesAvailable() > 0)
            processConnection(socket);
    }
}

void SingleInstanceManager::processConnection(QLocalSocket *socket)
{
    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_5_0);
    in.startTransaction();
    QString url;
    in >> url;
    if (!in.commitTransaction())
        return; // partial payload — readyRead will fire again

    if (!url.isEmpty()) {
        qCDebug(SingleInstanceLog) << "Received URL from secondary instance:" << url;
        emit urlReceived(url);
    }

    // Acknowledge so the secondary can finish cleanly.
    QDataStream out(socket);
    out.setVersion(QDataStream::Qt_5_0);
    out << QStringLiteral("ACK");
    socket->flush();
}
