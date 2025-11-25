#ifndef COCKATRICE_ARCHIDEKT_API_RESPONSE_DECK_OWNER_H
#define COCKATRICE_ARCHIDEKT_API_RESPONSE_DECK_OWNER_H

#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <QVector>

class ArchidektApiResponseDeckOwner
{
public:
    // Constructor
    ArchidektApiResponseDeckOwner() = default;

    // Parse deck-related data from JSON
    void fromJson(const QJsonObject &json);

    // Debug method for logging
    void debugPrint() const;

    [[nodiscard]] QString getName() const
    {
        return userName;
    }

private:
    int id;
    QString userName;
    QUrl avatar;
    bool moderator;
    int pledgeLevel;
    QStringList roles;
};

#endif // COCKATRICE_ARCHIDEKT_API_RESPONSE_DECK_OWNER_H
