#ifndef LIBCOCKATRICE_INTENT_H
#define LIBCOCKATRICE_INTENT_H

#include <QObject>
#include <QTimer>

/**
 * @brief Abstract base for chained, URL-driven intents.
 *
 * An Intent encapsulates a single async action (e.g. connecting to a server,
 * joining a room).  Concrete subclasses implement @c doExecute() and call
 * @c emitFinished(bool) when the action completes or fails unrecoverably.
 *
 * Typical usage:
 * @code
 *   auto *intent = new MyIntent(ctx, parent);
 *   intent->execute();  // intent deletes itself via deleteLater when finished
 * @endcode
 *
 * Guarantees:
 *   - @c finished() is emitted at most once (subsequent emit attempts are no-ops).
 *   - @c execute() is idempotent: repeated calls do nothing after the first.
 *   - Self-deletes via @c deleteLater after @c finished() fires (success or failure).
 */
class Intent : public QObject
{
    Q_OBJECT

public:
    /** Default deadline for intents that wait on async signals.  Used by the
     *  default @c timeoutMs() implementation so the chain can't hang
     *  indefinitely. */
    static constexpr int DefaultTimeoutMs = 30000;

    explicit Intent(QObject *parent = nullptr) : QObject(parent)
    {
        // Self-delete after finished() fires, regardless of whether the
        // emission came from doExecute() (success/failure) or abort()
        // (external).
        connect(this, &Intent::finished, this, &QObject::deleteLater);
    }
    ~Intent() override = default;

    /**
     * @brief Deadline for this intent's timeout safety net, in milliseconds.
     *
     * Subclasses override when their work is not bounded by network/server
     * timing — e.g. an intent that opens a modal dialog should return a
     * non-positive value to indicate "no auto-timeout, the user paces this
     * step".  Consumed by @c startTimeoutSafetyNet().
     *
     * @return positive deadline in ms, or <= 0 for "no timeout".
     */
    [[nodiscard]] virtual int timeoutMs() const
    {
        return DefaultTimeoutMs;
    }

    /** Start executing the intent.  Idempotent — repeated calls are no-ops. */
    void execute()
    {
        if (m_started)
            return;
        m_started = true;
        doExecute();
    }

    /**
     * @brief Abort the intent externally, emitting finished(false).
     *
     * Used by chain orchestrators (e.g. UrlParser) to propagate a failure
     * from an upstream intent through the rest of the chain without giving
     * outside callers direct access to the protected finished() signal.
     */
    void abort()
    {
        emitFinished(false);
    }

protected:
    virtual void doExecute() = 0;

    /**
     * @brief Single source of truth for emitting @c finished().
     *
     * Gated by an internal flag so subsequent calls are no-ops.  Concrete
     * intents call this instead of @c emit finished(...) directly, which
     * removes the risk of double-emission when multiple completion signals
     * race (success + cleanup disconnect, timeout + late response, etc.).
     */
    void emitFinished(bool success)
    {
        if (m_finished)
            return;
        m_finished = true;
        emit finished(success);
    }

    /**
     * @brief Arm the chain-level deadline; aborts on expiry.
     *
     * Subclasses call this once from @c doExecute() to install the timeout
     * safety net described by @c timeoutMs().  No-op when @c timeoutMs() is
     * non-positive (user-paced intents opt out).
     */
    void startTimeoutSafetyNet()
    {
        if (const int deadline = timeoutMs(); deadline > 0) {
            QTimer::singleShot(deadline, this, [this]() { emitFinished(false); });
        }
    }

signals:
    /**
     * @brief Emitted exactly once when the intent finishes.
     * @param success @c true on success, @c false on failure.
     */
    void finished(bool success);

private:
    bool m_started{false};
    bool m_finished{false};
};

#endif // LIBCOCKATRICE_INTENT_H
