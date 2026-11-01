#ifndef COCKATRICE_DEBOUNCER_H
#define COCKATRICE_DEBOUNCER_H

#include <QObject>
#include <QTimer>
#include <functional>
#include <utility>

namespace QtUtils
{
/**
 * @brief Runs the most recent call() after a period without further calls.
 *
 * Typical use is coalescing rapid model or editor updates into one refresh:
 *
 *     debouncer.call([this] { refresh(); });
 *
 * The pending callback is owned by the debouncer and replaced when call() is invoked
 * again. The debouncer must outlive any callback it schedules; parenting it to the
 * owning widget is usually enough.
 */
class Debouncer : public QObject
{
public:
    explicit Debouncer(int intervalMs, QObject *parent = nullptr) : QObject(parent)
    {
        timer.setSingleShot(true);
        timer.setInterval(intervalMs);
        connect(&timer, &QTimer::timeout, this, [this] {
            if (callback) {
                auto pending = std::move(callback);
                callback = nullptr;
                pending();
            }
        });
    }

    /**
     * @brief Schedules @p newCallback, replacing any callback not run yet.
     */
    void call(std::function<void()> newCallback)
    {
        callback = std::move(newCallback);
        timer.start();
    }

    /**
     * @brief Drops the pending callback and stops the timer.
     */
    void cancel()
    {
        timer.stop();
        callback = nullptr;
    }

private:
    QTimer timer;
    std::function<void()> callback;
};
} // namespace QtUtils

#endif // COCKATRICE_DEBOUNCER_H
