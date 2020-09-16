#ifndef COCKATRICE_GAMETIMEFILTER_H
#define COCKATRICE_GAMETIMEFILTER_H

#include <QDebug>
#include <QVector>

// If this enum is ever expanded, make sure to update:
// - getMaxGameAgeEnumOptions (order shown is order presented to user)
// - text option generation in getMaxGameAgeInSecondsFromEnum
// - validation conditions in getMaxGameAgeInSecondsFromComboBoxIndex
enum MaxGameAge
{
    NO_MAX_AGE = 0,
    FIVE_MINUTES = 1,
    TEN_MINUTES = 2,
    THIRTY_MINUTES = 3,
    ONE_HOUR = 4,
    TWO_HOURS = 5
};

inline const QVector<MaxGameAge> getMaxGameAgeEnumOptions()
{
    return QVector<MaxGameAge>({NO_MAX_AGE, FIVE_MINUTES, TEN_MINUTES, THIRTY_MINUTES, ONE_HOUR, TWO_HOURS});
}

inline int getMaxGameAgeInSecondsFromEnum(const MaxGameAge maxGameAgeEnum)
{
    const int kSecsPerMin = 60;
    const int kSecsPerHour = 3600;
    switch (maxGameAgeEnum) {
        case FIVE_MINUTES:
            return 5 * kSecsPerMin;
        case TEN_MINUTES:
            return 10 * kSecsPerMin;
        case THIRTY_MINUTES:
            return 30 * kSecsPerMin;
        case ONE_HOUR:
            return 1 * kSecsPerHour;
        case TWO_HOURS:
            return 2 * kSecsPerHour;
        default:
            return -1;
    }
}

inline int getMaxGameAgeInSecondsFromComboBoxIndex(const int maxGameAgeComboBoxIndex)
{
    if (maxGameAgeComboBoxIndex < NO_MAX_AGE || maxGameAgeComboBoxIndex > TWO_HOURS) {
        qDebug() << "Max game age combo box index is out of bounds";
        return -1;
    }
    return getMaxGameAgeInSecondsFromEnum(static_cast<MaxGameAge>(maxGameAgeComboBoxIndex));
}

inline int getComboBoxIndexForMaxGameAge(const int maxGameAgeSeconds)
{
    for (const MaxGameAge ageEnum : getMaxGameAgeEnumOptions()) {
        if (maxGameAgeSeconds == getMaxGameAgeInSecondsFromEnum(ageEnum)) {
            return (int)ageEnum;
        }
    }
    return (int)NO_MAX_AGE;
}

#endif // COCKATRICE_GAMETIMEFILTER_H
