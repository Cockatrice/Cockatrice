#ifndef PICTURELOADER_H
#define PICTURELOADER_H

#include "../../../game/cards/card_info.h"
#include "picture_loader_status_bar.h"
#include "picture_loader_worker.h"

#include <QLoggingCategory>

inline Q_LOGGING_CATEGORY(PictureLoaderLog, "picture_loader");
inline Q_LOGGING_CATEGORY(PictureLoaderCardBackCacheFailLog, "picture_loader.card_back_cache_fail");

class PictureLoader : public QObject
{
    Q_OBJECT
public:
    static PictureLoader &getInstance()
    {
        static PictureLoader instance;
        return instance;
    }

private:
    explicit PictureLoader();
    ~PictureLoader() override;
    // Singleton - Don't implement copy constructor and assign operator
    PictureLoader(PictureLoader const &);
    void operator=(PictureLoader const &);

    PictureLoaderWorker *worker;
    PictureLoaderStatusBar *statusBar;

public:
    static void getPixmap(QPixmap &pixmap, const ExactCard &card, QSize size);
    static void getCardBackPixmap(QPixmap &pixmap, QSize size);
    static void getCardBackLoadingInProgressPixmap(QPixmap &pixmap, QSize size);
    static void getCardBackLoadingFailedPixmap(QPixmap &pixmap, QSize size);
    static void clearPixmapCache();
    static void cacheCardPixmaps(const QList<ExactCard> &cards);
    static bool hasCustomArt();

public slots:
    static void clearNetworkCache();

private slots:
    void picDownloadChanged();
    void picsPathChanged();

public slots:
    void imageLoaded(const ExactCard &card, const QImage &image);
};
#endif
