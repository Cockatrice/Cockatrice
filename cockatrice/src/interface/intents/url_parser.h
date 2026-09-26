#ifndef COCKATRICE_URL_PARSER_H
#define COCKATRICE_URL_PARSER_H

#include <QList>
#include <QObject>
#include <QUrlQuery>

class Intent;
class MainWindow;
struct ContextJoinGame;

/**
 * @brief One queued intent chain with the session-migration bookkeeping for it.
 *
 * The restore fields are per-chain on purpose: chains are parsed while earlier
 * ones are still queued, so parser-wide state would let one chain's failure
 * consume the restore data another chain recorded.
 */
struct PendingIntentChain
{
    QList<Intent *> intents;

    // Snapshot of the session in place when this chain started running, so a
    // queued chain follows whichever server the chain before it settled on.
    QString previousServerHost;
    QString previousServerPort;

    // Recorded at parse time when the user confirmed migrating away from a live
    // session to the host/port named by the link.
    QString migrationTargetHost;
    QString migrationTargetPort;
    bool pendingRestore = false;
};

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
    Intent *createJoinGameIntent(const QUrlQuery &query, PendingIntentChain &chain);
    Intent *createOpenDeckIntent(const QUrlQuery &query, PendingIntentChain &chain);
    QString generateJoinGameMessage(const ContextJoinGame &context, const QString &gameDescription);
    [[nodiscard]] bool isConnectedTo(const QString &hostname, const QString &port) const;
    void startNextChain();
    void chainEnded(bool chainSucceeded);
    void onChainIntentDestroyed();
    void restorePreviousServer(const PendingIntentChain &chain);
    void restoreToPreviousServer(const PendingIntentChain &chain);

    MainWindow *mainWindow;
    QList<PendingIntentChain> pendingChains;
    bool chainRunning = false;
    // Disconnects the destroyed-signal backstop once a chain ends, so an old
    // intent's deferred deletion cannot end the chain that runs after it.
    QMetaObject::Connection chainBackstopConnection;
};

#endif // COCKATRICE_URL_PARSER_H
