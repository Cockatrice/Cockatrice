/**
 * @file card_picture_loader_status_bar.h
 * @ingroup PictureLoader
 * @brief TODO: Document this.
 */

#ifndef PICTURE_LOADER_STATUS_BAR_H
#define PICTURE_LOADER_STATUS_BAR_H

#include "../../interface/widgets/quick_settings/settings_button_widget.h"
#include "card_picture_loader_worker_work.h"

#include <QHBoxLayout>
#include <QProgressBar>
#include <QWidget>

class CardPictureLoaderStatusBar : public QWidget
{
    Q_OBJECT
public:
    explicit CardPictureLoaderStatusBar(QWidget *parent);

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
