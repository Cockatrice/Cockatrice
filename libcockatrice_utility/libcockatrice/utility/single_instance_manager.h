#ifndef LIBCOCKATRICE_SINGLE_INSTANCE_MANAGER_H
#define LIBCOCKATRICE_SINGLE_INSTANCE_MANAGER_H

#include <QLocalServer>
#include <QLocalSocket>
#include <QObject>
#include <QString>

/**
 * @brief Local-socket-based single-instance guard with URL forwarding.
 *
 * Asynchronously resolves whether this process is the primary instance or a
 * secondary that should forward a URL and exit.  All transitions are
 * driven by Qt signals (QLocalSocket::connected / readyRead /
 * errorOccurred) plus a single timeout — no synchronous waitFor* calls
 * anywhere, so platform-specific event-pump quirks don't race.
 *
 * Usage at startup (typically in main(), before QApplication::exec()):
 *
 * @code
 *   SingleInstanceManager sim(SingleInstanceManager::perUserSocketName("MyApp"));
 *   QEventLoop startupLoop;
 *   bool wasForwarded = false;
 *   QObject::connect(&sim, &SingleInstanceManager::roleResolved,
 *                    [&](bool forwarded) {
 *       wasForwarded = forwarded;
 *       startupLoop.quit();
 *   });
 *   sim.resolveStartupRole(urlFromArgv);  // may be empty
 *   startupLoop.exec();
 *   if (wasForwarded) return 0;
 *   // ...continue as primary (or as a non-primary secondary if no URL)...
 *   QObject::connect(&sim, &SingleInstanceManager::urlReceived,
 *                    &mainWindow, &MainWindow::handleUrl);
 * @endcode
 */
class SingleInstanceManager : public QObject
{
    Q_OBJECT

public:
    /** Deadline for the probe-and-forward handshake.  After this, we assume
     *  no primary is listening (or the old primary is dead) and become
     *  primary ourselves. */
    static constexpr int ForwardTimeoutMs = 2000;

    explicit SingleInstanceManager(const QString &socketName, QObject *parent = nullptr);
    ~SingleInstanceManager() override;

    /**
     * @brief Build a per-user socket name to prevent cross-user squatting.
     *
     * Appends the current user's name from $USER (Unix) or $USERNAME (Windows)
     * to @p base, separated by a dash.  Falls back to @p base unchanged when
     * the env var is empty.
     */
    static QString perUserSocketName(const QString &base);

    /**
     * @brief Asynchronously resolve our startup role.
     *
     * - If @p maybeUrl is non-empty AND a primary instance is already
     *   running, forward the URL to it and emit @c roleResolved(true).
     * - Otherwise (no URL, OR no primary, OR primary unresponsive within
     *   @c ForwardTimeoutMs), become the primary instance and emit
     *   @c roleResolved(false).
     *
     * Emits @c roleResolved exactly once.  Intended to be called at most
     * once at process startup, before @c QApplication::exec().
     */
    void resolveStartupRole(const QString &maybeUrl);

signals:
    /** Emitted exactly once after resolveStartupRole completes.
     *  @param forwarded true if a primary existed and we sent the URL to it
     *                   (caller should exit); false if we are now primary. */
    void roleResolved(bool forwarded);

    /** Emitted on the primary instance whenever another instance sends a URL. */
    void urlReceived(const QString &url);

private slots:
    void onNewConnection();

private:
    QString socketName;
    QLocalServer *server{nullptr};

    /** Listen on @c socketName.  Idempotent — safe to call once we've been
     *  resolved as the primary. */
    void becomePrimary();

    /** Read the URL from @p socket and emit @c urlReceived, then ACK. */
    void processConnection(QLocalSocket *socket);
};

#endif // LIBCOCKATRICE_SINGLE_INSTANCE_MANAGER_H
