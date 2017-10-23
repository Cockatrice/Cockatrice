#ifndef PRICEUPDATER_H
#define PRICEUPDATER_H

#include <QNetworkAccessManager>
#include "decklist.h"

class QNetworkAccessManager;

// If we don't typedef this, won't compile on OS X < 10.9
typedef QMap<int, QString> MuidStringMap;

/**
 * Price Updater.
 *
 * @author Marcio Ribeiro <mmr@b1n.org>
 */
class AbstractPriceUpdater : public QWidget
{
    Q_OBJECT
public:
    enum PriceSource { DBPriceSource };
protected:
    const DeckList *deck;
    QNetworkAccessManager *nam;
signals:
    void finishedUpdate();
protected slots:
    virtual void downloadFinished() = 0;
public:
    AbstractPriceUpdater(const DeckList *deck);
    virtual void updatePrices() = 0;
};

/*
class DBPriceUpdater : public AbstractPriceUpdater
{
    Q_OBJECT
protected:
    MuidStringMap muidMap;
    QList<QString> urls;
protected:
    virtual void downloadFinished();
    void requestNext();
public:
    DBPriceUpdater(const DeckList *deck);
    virtual void updatePrices();
};
*/
#endif
