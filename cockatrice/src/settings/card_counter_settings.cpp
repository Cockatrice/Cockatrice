#include "card_counter_settings.h"

#include <QColor>
#include <QSettings>

CardCounterSettings::CardCounterSettings(const QString &settingsPath, QObject *parent)
    : SettingsManager(settingsPath + "global.ini", parent)
{
}

void CardCounterSettings::setColor(int counterId, const QColor &color)
{
    QString key = QString("cards/counters/%1/color").arg(counterId);

    if (settings.value(key).value<QColor>() == color)
        return;

    settings.setValue(key, color);
    emit colorChanged(counterId, color);
}

QColor CardCounterSettings::color(int counterId) const
{
    QColor defaultColor;

    if (counterId < 6) {
        // Preserve legacy colors
        defaultColor = QColor::fromHsv(counterId * 60, 150, 255);
    } else {
        // Future-proof support for more counters with pseudo-random colors
        int h = (counterId * 37) % 360;
        int s = 128 + 64 * sin((counterId * 97) * 0.1);   // 64-192
        int v = 196 + 32 * sin((counterId * 101) * 0.07); // 164-228

        defaultColor = QColor::fromHsv(h, s, v);
    }

    return settings.value(QString("cards/counters/%1/color").arg(counterId), defaultColor).value<QColor>();
}

QString CardCounterSettings::displayName(int counterId) const
{
    // These are hardcoded for now. They should become configurable at some
    // point.
    switch (counterId) {
        case 0:
            return tr("Red");
        case 1:
            return tr("Yellow");
        case 2:
            return tr("Green");
        case 3:
            return tr("Cyan");
        case 4:
            return tr("Purple");
        case 5:
            return tr("Magenta");
        default:
            return QString(tr("Custom %1")).arg(counterId - 5);
    }
}
