/**
 * @file tapped_out_interface.h
 * @ingroup ApiInterfaces
 */
//! \todo Document this file.

#ifndef TAPPEDOUT_INTERFACE_H
#define TAPPEDOUT_INTERFACE_H

#include <QLoggingCategory>
#include <QObject>
#include <qtmetamacros.h>

inline Q_LOGGING_CATEGORY(TappedOutInterfaceLog, "tapped_out_interface");

class QByteArray;
class QString;
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
    bool replyHandled = false;
    void showError(const QString &message);

    void copyDeckSplitMainAndSide(const DeckList &source, DeckList &mainboard, DeckList &sideboard);
private slots:
    void queryFinished(QNetworkReply *reply);
    void getAnalyzeRequestData(const DeckList &deck, QByteArray &data);

public:
    explicit TappedOutInterface(QObject *parent = nullptr);
    /**
     * Posts the deck for analysis and returns immediately; there is no signal for the
     * outcome. The result opens in the browser or an error dialog, after which the
     * interface deletes itself, so a caller must call this once and not keep or reuse
     * the pointer afterwards.
     */
    void analyzeDeck(const DeckList &deck);
};

#endif
