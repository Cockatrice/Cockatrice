#ifndef TAPPEDOUT_INTERFACE_H
#define TAPPEDOUT_INTERFACE_H

#include "carddatabase.h"
#include "decklist.h"
#include <QObject>

class QByteArray;
class QNetworkAccessManager;
class QNetworkReply;
class DeckList;

/**
 * TappedOutInterface exists in order to support the "Analyze on TappedOut" feature.
 * An http POST request is sent and the result is retrieved from the reply. Parsing
 * logic is implemented in TappedOutInterface::queryFinished().
 */

class TappedOutInterface : public QObject {
    Q_OBJECT
private:
    QNetworkAccessManager *manager;

    CardDatabase &cardDatabase;
    void copyDeckSplitMainAndSide(const DeckList &source, DeckList& mainboard, DeckList& sideboard);
private slots:
    void queryFinished(QNetworkReply *reply);
    void getAnalyzeRequestData(DeckList *deck, QByteArray *data);
public:
    TappedOutInterface(CardDatabase &_cardDatabase, QObject *parent = 0);
    void analyzeDeck(DeckList *deck);
};

#endif
