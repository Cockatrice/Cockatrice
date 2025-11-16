#ifndef PICTURE_LOADER_STATUS_BAR_H
#define PICTURE_LOADER_STATUS_BAR_H

#include "../../interface/widgets/quick_settings/settings_button_widget.h"
#include "card_picture_loader_worker_work.h"

#include <QHBoxLayout>
#include <QProgressBar>
#include <QTimer>
#include <QWidget>

/**
 * @class CardPictureLoaderStatusBar
 * @ingroup PictureLoader
 * @brief Displays the status of card image downloads in a horizontal progress bar with a log popup.
 *
 * Responsibilities:
 * - Shows overall progress of image downloads.
 * - Maintains a log of individual requests via a popup (SettingsButtonWidget).
 * - Cleans up finished request entries automatically after a delay.
 */
class CardPictureLoaderStatusBar : public QWidget
{
    Q_OBJECT
public:
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
    QHBoxLayout *layout;           ///< Horizontal layout containing progress bar and log button
    QProgressBar *progressBar;     ///< Progress bar showing overall download progress
    SettingsButtonWidget *loadLog; ///< Popup log showing individual request statuses
    QTimer *cleaner;               ///< Timer for periodically cleaning old log entries
};

#endif // PICTURE_LOADER_STATUS_BAR_H
