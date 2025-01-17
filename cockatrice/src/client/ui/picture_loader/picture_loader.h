#ifndef PICTURELOADER_H
#define PICTURELOADER_H

#include "../../../game/cards/card_database.h"
#include "picture_loader_worker.h"

#include <QLoggingCategory>

inline Q_LOGGING_CATEGORY(PictureLoaderLog, "picture_loader")

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

public:
    static void getPixmap(QPixmap &pixmap, CardInfoPtr card, QSize size);
    static void getCardBackPixmap(QPixmap &pixmap, QSize size);
    static void getCardBackLoadingInProgressPixmap(QPixmap &pixmap, QSize size);
    static void getCardBackLoadingFailedPixmap(QPixmap &pixmap, QSize size);
    static void clearPixmapCache(CardInfoPtr card);
    static void clearPixmapCache();
    static void cacheCardPixmaps(QList<CardInfoPtr> cards);

public slots:
    static void clearNetworkCache();

private slots:
    void picDownloadChanged();
    void picsPathChanged();

public slots:
    void imageLoaded(CardInfoPtr card, const QImage &image);
};
#endif
