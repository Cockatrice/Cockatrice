#ifndef PRICEUPDATER_H
#define PRICEUPDATER_H

#include <QNetworkReply>
#include "decklist.h"

class QNetworkAccessManager;

/**
 * Price Updater.
 *
 * @author Marcio Ribeiro <mmr@b1n.org>
 */
class PriceUpdater : public QObject
{
    Q_OBJECT
private:
    const DeckList *deck;
    QNetworkAccessManager *nam;
signals:
    void finishedUpdate();
private slots:
    void downloadFinished();
public:
    PriceUpdater(const DeckList *deck);
    void updatePrices();
};
#endif
