#ifndef PICTURE_LOADER_STATUS_BAR_H
#define PICTURE_LOADER_STATUS_BAR_H

#include <QList>
#include <QMultiHash>
#include <QString>
#include <QWidget>
#include <qtmetamacros.h>

class CardPictureLoaderRequestStatusDisplayWidget;
class ExactCard;
class QHBoxLayout;
class QProgressBar;
class QTimer;
class QUrl;
class SettingsButtonWidget;

/**
 * @class CardPictureLoaderStatusBar
 * @ingroup PictureLoader
 * @brief Displays the status of card image downloads in a horizontal progress bar with a log popup.
 *
 * Responsibilities:
 * - Shows overall progress of image downloads.
 * - Maintains a log of individual requests via a popup (SettingsButtonWidget).
 * - Cleans up finished request entries automatically after a delay.
 *
 * The log is a diagnostic view, not a queue: it keeps at most MAX_LOG_ENTRIES
 * entries and indexes them by URL, so a burst of requests stays O(1) per event
 * instead of scanning every entry the popup holds.
 */
class CardPictureLoaderStatusBar : public QWidget
{
    Q_OBJECT
public:
    /** Entries kept in the log; older ones are evicted as new requests arrive. */
    static constexpr int MAX_LOG_ENTRIES = 50;
    /** Seconds a finished entry stays in the log before it is cleaned up. */
    static constexpr int LOG_ENTRY_LIFETIME_SECS = 10;

    /**
     * @brief Constructs a status bar with progress bar and log button.
     * @param parent Parent widget
     */
    explicit CardPictureLoaderStatusBar(QWidget *parent);

public slots:
    /**
     * @brief Adds a queued image download to the log and increments the progress bar maximum.
     * @param url URL of the image
     * @param card The card being loaded
     * @param setName The set name of the card
     */
    void addQueuedImageLoad(const QUrl &url, const ExactCard &card, const QString &setName);

    /**
     * @brief Marks an image as successfully loaded.
     *        Updates the progress bar and marks the corresponding log entry as finished.
     * @param url URL of the successfully loaded image
     */
    void addSuccessfulImageLoad(const QUrl &url);

    /**
     * @brief Cleans up old entries from the log that have finished more than 10 seconds ago.
     *        Adjusts the progress bar accordingly.
     */
    void cleanOldEntries();

private:
    /**
     * @brief Evicts the oldest finished entry (or the oldest entry if none finished)
     *        until the log fits within MAX_LOG_ENTRIES.
     */
    void trimLog();

    /**
     * @brief Removes one entry from the log, its URL index and the popup, and keeps
     *        the progress bar counters in step with the entries that remain.
     * @param entry Entry to remove
     */
    void removeLogEntry(CardPictureLoaderRequestStatusDisplayWidget *entry);

    QHBoxLayout *layout;           ///< Horizontal layout containing progress bar and log button
    QProgressBar *progressBar;     ///< Progress bar showing overall download progress
    SettingsButtonWidget *loadLog; ///< Popup log showing individual request statuses
    QTimer *cleaner;               ///< Timer for periodically cleaning old log entries
    QList<CardPictureLoaderRequestStatusDisplayWidget *> logEntries; ///< Logged requests, oldest first
    // A URL can back several live entries: a request parked for a deferred retry is re-queued while
    // its first entry is still pending, so a completion must finish every entry for that URL.
    QMultiHash<QString, CardPictureLoaderRequestStatusDisplayWidget *>
        entriesByUrl; ///< Log entries indexed by their URL for O(1) completion lookup
};

#endif // PICTURE_LOADER_STATUS_BAR_H
