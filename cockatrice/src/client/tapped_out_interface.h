#ifndef TAPPEDOUT_INTERFACE_H
#define TAPPEDOUT_INTERFACE_H

#include "../../../common/deck_list/deck_list.h"
#include "../game/cards/card_database.h"

#include <QLoggingCategory>
#include <QObject>

inline Q_LOGGING_CATEGORY(TappedOutInterfaceLog, "tapped_out_interface");

class QByteArray;
class QNetworkAccessManager;
class QNetworkReply;
class DeckList;

/**
 * TappedOutInterface exists in order to support the "Analyze on TappedOut" feature.
 * An http POST request is sent and the result is retrieved from the reply. Parsing
 * logic is implemented in TappedOutInterface::queryFinished().
 */

class TappedOutInterface : public QObject
{
    Q_OBJECT
private:
    QNetworkAccessManager *manager;

    CardDatabase &cardDatabase;
    void copyDeckSplitMainAndSide(DeckList &source, DeckList &mainboard, DeckList &sideboard);
private slots:
    void queryFinished(QNetworkReply *reply);
    void getAnalyzeRequestData(DeckList *deck, QByteArray *data);

public:
    explicit TappedOutInterface(CardDatabase &_cardDatabase, QObject *parent = nullptr);
    void analyzeDeck(DeckList *deck);
};

#endif
