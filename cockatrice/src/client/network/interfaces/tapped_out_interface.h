/**
 * @file tapped_out_interface.h
 * @ingroup ApiInterfaces
 */
//! \todo Document this file.

#ifndef TAPPEDOUT_INTERFACE_H
#define TAPPEDOUT_INTERFACE_H

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

    void copyDeckSplitMainAndSide(const DeckList &source, DeckList &mainboard, DeckList &sideboard);
private slots:
    void queryFinished(QNetworkReply *reply);
    void getAnalyzeRequestData(const DeckList &deck, QByteArray &data);

public:
    explicit TappedOutInterface(QObject *parent = nullptr);
    void analyzeDeck(const DeckList &deck);
};

#endif
