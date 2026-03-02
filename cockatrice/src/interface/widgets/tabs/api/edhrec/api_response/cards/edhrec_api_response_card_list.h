/**
 * @file edhrec_api_response_card_list.h
 * @ingroup ApiResponses
 * @brief TODO: Document this.
 */

#ifndef CARD_LIST_H
#define CARD_LIST_H

#include "edhrec_api_response_card_details.h"

#include <QJsonArray>
#include <QList>
#include <QString>

class EdhrecApiResponseCardList
{
public:
    QString header;
    QList<EdhrecApiResponseCardDetails> cardViews;

    // Default constructor
    EdhrecApiResponseCardList();

    // Method to populate the object from a JSON object
    void fromJson(const QJsonObject &json);

    // Debug method to print out the data
    void debugPrint() const;
};

#endif // CARD_LIST_H
