#include "card_counter_settings.h"

#include <QColor>
#include <QSettings>
#include <QtMath>

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
        int s = 128 + 64 * qSin((counterId * 97) * 0.1);   // 64-192
        int v = 196 + 32 * qSin((counterId * 101) * 0.07); // 164-228

        defaultColor = QColor::fromHsv(h, s, v);
    }

    return settings.value(QString("cards/counters/%1/color").arg(counterId), defaultColor).value<QColor>();
}

QString CardCounterSettings::displayName(int counterId) const
{
    // Currently, card counters name are fixed to A, B, ..., Z, AA, AB, ...

    auto nChars = 1 + counterId / 26;
    QString str;
    str.resize(nChars);

    for (auto it = str.rbegin(); it != str.rend(); ++it) {
        *it = QChar('A' + (counterId) % 26);
        counterId /= 26;
    }

    return str;
}
