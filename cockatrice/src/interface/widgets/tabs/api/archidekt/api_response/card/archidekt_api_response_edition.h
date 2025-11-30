#ifndef COCKATRICE_ARCHIDEKT_API_RESPONSE_EDITION_H
#define COCKATRICE_ARCHIDEKT_API_RESPONSE_EDITION_H

#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <QVector>

class ArchidektApiResponseEdition
{
public:
    // Constructor
    ArchidektApiResponseEdition() = default;

    // Parse deck-related data from JSON
    void fromJson(const QJsonObject &json);

    // Debug method for logging
    void debugPrint() const;

    [[nodiscard]] QString getEditionCode() const
    {
        return editionCode;
    }
    [[nodiscard]] QString getEditionName() const
    {
        return editionName;
    }
    [[nodiscard]] QString getEditionDate() const
    {
        return editionDate;
    }
    [[nodiscard]] QString getEditionType() const
    {
        return editionType;
    }
    [[nodiscard]] QString getMtgoCode() const
    {
        return mtgoCode;
    }

private:
    QString editionCode;
    QString editionName;
    QString editionDate;
    QString editionType;
    QString mtgoCode;
};

#endif // COCKATRICE_ARCHIDEKT_API_RESPONSE_EDITION_H
