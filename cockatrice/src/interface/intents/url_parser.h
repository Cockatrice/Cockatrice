#ifndef COCKATRICE_URL_PARSER_H
#define COCKATRICE_URL_PARSER_H
#include <QList>
#include <QObject>
#include <QUrlQuery>

class Intent;
class MainWindow;
struct ContextJoinGame;

/**
 * @brief Parses cockatrice:// links and runs them as serialized intent chains.
 *
 * Links are parsed by action (joingame/opendeck) and translated into an intent
 * chain. Chains are queued and run one at a time: a document can hand multiple
 * links to the window while an earlier chain still connects, and running two
 * connect chains concurrently tears the connection down. urlChainFinished is
 * emitted once the queue has fully drained.
 */
class IntentUrlParser : public QObject
{
    Q_OBJECT

public:
    IntentUrlParser(QObject *parent, MainWindow *mainWindow);
    void handle(const QString &urlStr);

signals:
    /** @brief Emitted when the last queued chain ended; carries whether the client is logged in. */
    void urlChainFinished(bool connected);

private:
    Intent *createJoinGameIntent(const QUrlQuery &query, QList<Intent *> &chain);
    Intent *createOpenDeckIntent(const QUrlQuery &query, QList<Intent *> &chain);
    QString generateJoinGameMessage(const ContextJoinGame &context, const QString &gameDescription);
    [[nodiscard]] bool isConnectedTo(const QString &hostname, const QString &port) const;
    void startNextChain();
    void chainEnded();
    void restorePreviousServer();
    void restoreToPreviousServer();

    MainWindow *mainWindow;
    QList<QList<Intent *>> pendingChains;
    bool chainRunning = false;
    bool currentChainSucceeded = false;

    // Set when an open-deck link migrates the session to another server. If the
    // chain then fails or is cancelled while still on that server, the previous
    // session is restored (reconnect if credentials are saved, else disconnect).
    QString migrationTargetHost;
    QString migrationTargetPort;
    QString previousServerHost;
    QString previousServerPort;
    bool pendingRestore = false;
};

#endif // COCKATRICE_URL_PARSER_H
