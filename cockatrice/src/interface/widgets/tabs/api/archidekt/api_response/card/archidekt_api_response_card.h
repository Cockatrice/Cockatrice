#ifndef COCKATRICE_ARCHIDEKT_API_RESPONSE_CARD_H
#define COCKATRICE_ARCHIDEKT_API_RESPONSE_CARD_H

#include "archidekt_api_response_edition.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <QVector>

class ArchidektApiResponseCard
{
public:
    // Constructor
    ArchidektApiResponseCard() = default;

    // Parse deck-related data from JSON
    void fromJson(const QJsonObject &json);

    // Debug method for logging
    void debugPrint() const;

    QJsonObject getOracleCard() const
    {
        return oracleCard;
    };

    QString getCollectorNumber() const
    {
        return collectorNumber;
    }

    ArchidektApiResponseEdition getEdition() const
    {
        return edition;
    }

private:
    int id;
    QString artist;
    int tcgProductId;
    int ckFoilId;
    int ckNormalId;
    QString cmEd;
    QString scgSku;
    QString scgFoilSku;
    QString collectorNumber;
    int multiverseId;
    int mtgoFoilId;
    int mtgoNormalId;
    QString uid;
    QString displayName;
    QString releasedAt;
    ArchidektApiResponseEdition edition;
    QString flavor;
    QStringList games;
    QStringList options;
    QString scryfallImageHash;
    QJsonObject oracleCard;
    int owned;
    int pinnedStatus;
    // ArchidektApiResponsePrices prices;
    QString rarity;
    QStringList globalCategories;
};

#endif // COCKATRICE_ARCHIDEKT_API_RESPONSE_CARD_H
