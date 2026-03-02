#ifndef AVERAGE_DECK_STATISTICS_H
#define AVERAGE_DECK_STATISTICS_H

#include <QJsonObject>

/**
 * @struct EdhrecCommanderApiResponseAverageDeckStatistics
 * @ingroup ApiResponses
 * @brief Represents the typical deck statistics (collapsed section)
 */
struct EdhrecCommanderApiResponseAverageDeckStatistics
{
    int creature = 0;
    int instant = 0;
    int sorcery = 0;
    int artifact = 0;
    int enchantment = 0;
    int battle = 0;
    int planeswalker = 0;
    int land = 0;
    int basic = 0;
    int nonbasic = 0;

    void fromJson(const QJsonObject &json);
};
#endif // AVERAGE_DECK_STATISTICS_H
