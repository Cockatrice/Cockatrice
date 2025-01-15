#ifndef CARD_VIEW_H
#define CARD_VIEW_H

#include <QJsonObject>
#include <QString>

class EdhrecCommanderApiResponseCardDetails
{
public:
    QString name;
    QString sanitized;
    QString sanitizedWo;
    QString url;
    double synergy;
    int inclusion;
    QString label;
    int numDecks;
    int potentialDecks;

    EdhrecCommanderApiResponseCardDetails();

    // Method to populate the object from a JSON object
    void fromJson(const QJsonObject &json);

    // Debug method to print out the data
    void debugPrint() const;
};

#endif // CARD_VIEW_H
