/**
 * @file card_picture_loader_request_status_display_widget.h
 * @ingroup PictureLoader
 * @brief TODO: Document this.
 */

#ifndef PICTURE_LOADER_REQUEST_STATUS_DISPLAY_WIDGET_H
#define PICTURE_LOADER_REQUEST_STATUS_DISPLAY_WIDGET_H
#include "card_picture_loader_worker_work.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QWidget>

class CardPictureLoaderRequestStatusDisplayWidget : public QWidget
{
    Q_OBJECT
public:
    CardPictureLoaderRequestStatusDisplayWidget(QWidget *parent,
                                                const QUrl &url,
                                                const ExactCard &card,
                                                const QString &setName);

    void setFinished()
    {
        finished->setText("True");
        update();
        repaint();
    }

    bool getFinished() const
    {
        return finished->text() == "True";
    }

    void setElapsedTime(const QString &_elapsedTime) const
    {
        elapsedTime->setText(_elapsedTime);
    }

    int queryElapsedSeconds()
    {
        if (!finished) {
            int elapsedSeconds = QDateTime::fromString(startTime->text()).secsTo(QDateTime::currentDateTime());
            elapsedTime->setText(QString::number(elapsedSeconds));
            update();
            repaint();
            return elapsedSeconds;
        }
        return elapsedTime->text().toInt();
    }

    QString getStartTime() const
    {
        return startTime->text();
    }

    QString getUrl() const
    {
        return url->text();
    }

private:
    QHBoxLayout *layout;
    QLabel *name;
    QLabel *setShortname;
    QLabel *providerId;
    QLabel *startTime;
    QLabel *elapsedTime;
    QLabel *finished;
    QLabel *url;
};

#endif // PICTURE_LOADER_REQUEST_STATUS_DISPLAY_WIDGET_H
