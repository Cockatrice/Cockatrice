/**
 * @file deck_stats_interface.h
 * @ingroup ApiInterfaces
 * @brief TODO: Document this.
 */

#ifndef DECKSTATS_INTERFACE_H
#define DECKSTATS_INTERFACE_H

#include <libcockatrice/card/database/card_database.h>
#include <libcockatrice/deck_list/deck_list.h>

class QByteArray;
class QNetworkAccessManager;
class QNetworkReply;
class DeckList;

class DeckStatsInterface : public QObject
{
    Q_OBJECT
private:
    QNetworkAccessManager *manager;

    CardDatabase &cardDatabase;

    /**
     * Deckstats doesn't recognize token cards, and instead tries to find the
     * closest non-token card instead. So we construct a new deck which has no
     * tokens.
     */
    void copyDeckWithoutTokens(DeckList &source, DeckList &destination);

private slots:
    void queryFinished(QNetworkReply *reply);
    void getAnalyzeRequestData(DeckList *deck, QByteArray *data);

public:
    explicit DeckStatsInterface(CardDatabase &_cardDatabase, QObject *parent = nullptr);
    void analyzeDeck(DeckList *deck);
};

#endif
