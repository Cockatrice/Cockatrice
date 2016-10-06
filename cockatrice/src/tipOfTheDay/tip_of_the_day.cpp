#include <QObject>
#include "tip_of_the_day.h"

QList<TipOfTheDay>& TipOfTheDay::AllTips =
        QList<TipOfTheDay>()
        << TipOfTheDay(QObject::tr("Tip of the Day"), QObject::tr("totd desc"), QObject::tr("tip_of_the_day"), ex::nullopt);
