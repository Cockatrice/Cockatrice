#ifndef COCKATRICE_TIP_OF_THE_DAY_H
#define COCKATRICE_TIP_OF_THE_DAY_H

#include <QString>
#include "optional.h"

namespace ex = std::experimental;

/**
 * A description of a tip of the day to be shown to the user.
 */
class TipOfTheDay
{
public:
    const QString &title;
    const QString &longDescription;
    const QString &settingName;
    const ex::optional<QString> &tipImageFileName;

    TipOfTheDay(QString &_title, QString &_longDescription, QString &_settingName,
                ex::optional<QString> &_tipImageFileName) : title(_title),
                                                            longDescription(_longDescription),
                                                            settingName(_settingName),
                                                            tipImageFileName(_tipImageFileName)
    { };
    static QList<TipOfTheDay> &AllTips;
};

#endif //COCKATRICE_TIP_OF_THE_DAY_H
