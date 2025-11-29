/**
 * @file edhrec_api_response_card_container.h
 * @ingroup ApiResponses
 * @brief TODO: Document this.
 */

#ifndef CONTAINER_ENTRY_H
#define CONTAINER_ENTRY_H

#include "edhrec_api_response_card_list.h"
#include "edhrec_commander_api_response_commander_details.h"

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
    [[nodiscard]] const QString &getDescription() const
    {
        return description;
    }
    [[nodiscard]] const QVector<QJsonObject> &getBreadcrumb() const
    {
        return breadcrumb;
    }
    [[nodiscard]] const EdhrecCommanderApiResponseCommanderDetails &getCommanderDetails() const
    {
        return card;
    }
    [[nodiscard]] const QVector<EdhrecApiResponseCardList> &getCardlists() const
    {
        return cardlists;
    }
    [[nodiscard]] const QString &getKeywords() const
    {
        return keywords;
    }
    [[nodiscard]] const QString &getTitle() const
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
