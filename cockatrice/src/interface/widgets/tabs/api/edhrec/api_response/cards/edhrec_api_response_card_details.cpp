#include "edhrec_api_response_card_details.h"

#include <QDebug>

EdhrecApiResponseCardDetails::EdhrecApiResponseCardDetails()
    : synergy(0.0), inclusion(0), numDecks(0), potentialDecks(0)
{
}

void EdhrecApiResponseCardDetails::fromJson(const QJsonObject &json)
{
    // Parse the fields from the JSON object
    name = json.value("name").toString();
    sanitized = json.value("sanitized").toString();
    sanitizedWo = json.value("sanitized_wo").toString();
    url = json.value("url").toString();
    synergy = json.value("synergy").toDouble(0.0);
    inclusion = json.value("inclusion").toInt(0);
    label = json.value("label").toString();
    numDecks = json.value("num_decks").toInt(0);
    potentialDecks = json.value("potential_decks").toInt(0);
}

void EdhrecApiResponseCardDetails::debugPrint() const
{
    // Print out all the fields for debugging
    qDebug() << "Name:" << name;
    qDebug() << "Sanitized:" << sanitized;
    qDebug() << "Sanitized Wo:" << sanitizedWo;
    qDebug() << "URL:" << url;
    qDebug() << "Synergy:" << synergy;
    qDebug() << "Inclusion:" << inclusion;
    qDebug() << "Label:" << label;
    qDebug() << "Num Decks:" << numDecks;
    qDebug() << "Potential Decks:" << potentialDecks;
}
