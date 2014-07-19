#ifndef DECKSTATS_INTERFACE_H
#define DECKSTATS_INTERFACE_H

#include <QObject>

class QByteArray;
class QNetworkAccessManager;
class QNetworkReply;
class DeckList;

class DeckStatsInterface : public QObject {
    Q_OBJECT
private:
    QNetworkAccessManager *manager;
private slots:
    void queryFinished(QNetworkReply *reply);
    void getAnalyzeRequestData(DeckList *deck, QByteArray *data);
public:
    DeckStatsInterface(QObject *parent = 0);
    void analyzeDeck(DeckList *deck);
};

#endif
