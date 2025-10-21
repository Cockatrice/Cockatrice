/**
 * @file card_picture_loader_local.h
 * @ingroup PictureLoader
 * @brief TODO: Document this.
 */

#ifndef PICTURE_LOADER_LOCAL_H
#define PICTURE_LOADER_LOCAL_H

#include <QLoggingCategory>
#include <QObject>
#include <QTimer>
#include <libcockatrice/card/printing/exact_card.h>

inline Q_LOGGING_CATEGORY(CardPictureLoaderLocalLog, "card_picture_loader.local");

/**
 * Handles searching for and loading card images from the local pics and custom image folders.
 * This class maintains an index of the CUSTOM folder, to avoid repeatedly searching the entire directory.
 */
class CardPictureLoaderLocal : public QObject
{
    Q_OBJECT

public:
    explicit CardPictureLoaderLocal(QObject *parent);

    QImage tryLoad(const ExactCard &toLoad) const;

private:
    QString picsPath, customPicsPath;

    QMultiHash<QString, QString> customFolderIndex; // multimap of cardName to picPaths
    QTimer *refreshTimer;

    void refreshIndex();

    QImage tryLoadCardImageFromDisk(const QString &setName,
                                    const QString &correctedCardName,
                                    const QString &collectorNumber,
                                    const QString &providerId) const;

private slots:
    void picsPathChanged();
};

#endif // PICTURE_LOADER_LOCAL_H
