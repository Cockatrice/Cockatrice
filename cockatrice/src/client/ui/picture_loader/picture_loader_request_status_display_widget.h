#ifndef PICTURE_LOADER_REQUEST_STATUS_DISPLAY_WIDGET_H
#define PICTURE_LOADER_REQUEST_STATUS_DISPLAY_WIDGET_H
#include "picture_loader_worker_work.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QWidget>

class PictureLoaderRequestStatusDisplayWidget : public QWidget
{
    Q_OBJECT
public:
    PictureLoaderRequestStatusDisplayWidget(QWidget *parent, const QUrl &url, PictureLoaderWorkerWork *worker);
    PictureLoaderWorkerWork *worker;

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
            int elapsedTime = QDateTime::fromString(startTime->text()).secsTo(QDateTime::currentDateTime());
            elapsedTime->setText(
                QString::number(elapsedTime));
            update();
            repaint();
            return elapsedTime;
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
