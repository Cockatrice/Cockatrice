#ifndef PICTURE_LOADER_STATUS_BAR_H
#define PICTURE_LOADER_STATUS_BAR_H

#include "../widgets/quick_settings/settings_button_widget.h"
#include "picture_loader_worker_work.h"

#include <QHBoxLayout>
#include <QProgressBar>
#include <QWidget>

class PictureLoaderStatusBar : public QWidget
{
    Q_OBJECT
public:
    explicit PictureLoaderStatusBar(QWidget *parent);

public slots:
    void addQueuedImageLoad(const QUrl &url, const ExactCard &card, const QString &setName);
    void addSuccessfulImageLoad(const QUrl &url);
    void cleanOldEntries();

private:
    QHBoxLayout *layout;
    QProgressBar *progressBar;
    SettingsButtonWidget *loadLog;
    QTimer *cleaner;
};

#endif // PICTURE_LOADER_STATUS_BAR_H
