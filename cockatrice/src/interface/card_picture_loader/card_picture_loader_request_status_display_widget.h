#ifndef PICTURE_LOADER_REQUEST_STATUS_DISPLAY_WIDGET_H
#define PICTURE_LOADER_REQUEST_STATUS_DISPLAY_WIDGET_H

#include "card_picture_loader_worker_work.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QWidget>

/**
 * @class CardPictureLoaderRequestStatusDisplayWidget
 * @ingroup PictureLoader
 * @brief A small widget to display the status of a single card image request.
 *
 * Displays:
 * - Card name
 * - Set short name
 * - Provider ID
 * - Start time of request
 * - Elapsed time since start
 * - Finished status
 * - URL of the requested image
 */
class CardPictureLoaderRequestStatusDisplayWidget : public QWidget
{
    Q_OBJECT
public:
    /**
     * @brief Constructs a new status widget for a specific card image request.
     * @param parent Parent widget
     * @param url URL of the image being requested
     * @param card The ExactCard being downloaded
     * @param setName Set name for the card
     */
    CardPictureLoaderRequestStatusDisplayWidget(QWidget *parent,
                                                const QUrl &url,
                                                const ExactCard &card,
                                                const QString &setName);

    /** Marks the request as finished */
    void setFinished()
    {
        finished->setText("True");
        update();
        repaint();
    }

    /** Returns whether the request has finished */
    [[nodiscard]] bool getFinished() const
    {
        return finished->text() == "True";
    }

    /** Updates the elapsed time display */
    void setElapsedTime(const QString &_elapsedTime) const
    {
        elapsedTime->setText(_elapsedTime);
    }

    /**
     * @brief Queries the elapsed time in seconds since the request started
     * @return Elapsed time in seconds
     */
    int queryElapsedSeconds()
    {
        if (!getFinished()) {
            int elapsedSeconds = QDateTime::fromString(startTime->text()).secsTo(QDateTime::currentDateTime());
            elapsedTime->setText(QString::number(elapsedSeconds));
            update();
            repaint();
            return elapsedSeconds;
        }
        return elapsedTime->text().toInt();
    }

    /** Returns the start time as a string */
    [[nodiscard]] QString getStartTime() const
    {
        return startTime->text();
    }

    /** Returns the URL of the request */
    [[nodiscard]] QString getUrl() const
    {
        return url->text();
    }

private:
    QHBoxLayout *layout;  ///< Horizontal layout for arranging labels
    QLabel *name;         ///< Card name
    QLabel *setShortname; ///< Set short name
    QLabel *providerId;   ///< Provider ID for the card
    QLabel *startTime;    ///< Start time of the request
    QLabel *elapsedTime;  ///< Elapsed time since start
    QLabel *finished;     ///< Whether the request has finished
    QLabel *url;          ///< URL of the requested image
};

#endif // PICTURE_LOADER_REQUEST_STATUS_DISPLAY_WIDGET_H
