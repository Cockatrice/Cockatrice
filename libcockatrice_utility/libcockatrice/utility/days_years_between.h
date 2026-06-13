#include <QDateTime>

inline static QPair<int, int> getDaysAndYearsBetween(const QDate &then, const QDate &now)
{
    int years = now.addDays(1 - then.dayOfYear()).year() - then.year(); // there is no yearsTo
    int days = then.addYears(years).daysTo(now);
    return {days, years};
}
