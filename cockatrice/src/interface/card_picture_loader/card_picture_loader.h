/**
 * @file card_picture_loader.h
 * @ingroup PictureLoader
 * @brief TODO: Document this.
 */

#ifndef CARD_PICTURE_LOADER_H
#define CARD_PICTURE_LOADER_H

#include "card_picture_loader_status_bar.h"
#include "card_picture_loader_worker.h"

#include <QLoggingCategory>
#include <libcockatrice/card/card_info.h>

inline Q_LOGGING_CATEGORY(CardPictureLoaderLog, "card_picture_loader");
inline Q_LOGGING_CATEGORY(CardPictureLoaderCardBackCacheFailLog, "card_picture_loader.card_back_cache_fail");

class CardPictureLoader : public QObject
{
    Q_OBJECT
public:
    static CardPictureLoader &getInstance()
    {
        static CardPictureLoader instance;
        return instance;
    }

private:
    explicit CardPictureLoader();
    ~CardPictureLoader() override;
    // Singleton - Don't implement copy constructor and assign operator
    CardPictureLoader(CardPictureLoader const &);
    void operator=(CardPictureLoader const &);

    CardPictureLoaderWorker *worker;
    CardPictureLoaderStatusBar *statusBar;

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
