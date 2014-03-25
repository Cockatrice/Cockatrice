#ifndef DECKSTATS_INTERFACE_H
#define DECKSTATS_INTERFACE_H

#include <QObject>

class QNetworkAccessManager;
class QNetworkReply;
class DeckList;

class DeckStatsInterface : public QObject {
    Q_OBJECT
private:
    QNetworkAccessManager *manager;
private slots:
    void queryFinished(QNetworkReply *reply);
public:
    DeckStatsInterface(QObject *parent = 0);
    void analyzeDeck(DeckList *deck);
};

#endif
