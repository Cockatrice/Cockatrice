/**
 * @file edhrec_api_response_card_container.h
 * @ingroup ApiResponses
 * @brief TODO: Document this.
 */

#ifndef CONTAINER_ENTRY_H
#define CONTAINER_ENTRY_H

#include "edhrec_api_response_card_list.h"
#include "edhrec_commander_api_response_commander_details.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <QVector>

class EdhrecApiResponseCardContainer
{
public:
    // Constructor
    EdhrecApiResponseCardContainer() = default;

    // Parse deck-related data from JSON
    void fromJson(const QJsonObject &json);

    // Debug method for logging
    void debugPrint() const;

    // Getter methods for deck container
    const QString &getDescription() const
    {
        return description;
    }
    const QVector<QJsonObject> &getBreadcrumb() const
    {
        return breadcrumb;
    }
    const EdhrecCommanderApiResponseCommanderDetails &getCommanderDetails() const
    {
        return card;
    }
    const QVector<EdhrecApiResponseCardList> &getCardlists() const
    {
        return cardlists;
    }
    const QString &getKeywords() const
    {
        return keywords;
    }
    const QString &getTitle() const
    {
        return title;
    }

private:
    QString description;
    QVector<QJsonObject> breadcrumb;
    EdhrecCommanderApiResponseCommanderDetails card;
    QVector<EdhrecApiResponseCardList> cardlists;
    QString keywords;
    QString title;
};

#endif // CONTAINER_ENTRY_H
