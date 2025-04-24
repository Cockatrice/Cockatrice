#ifndef CARD_COUNTER_SETTINGS_H
#define CARD_COUNTER_SETTINGS_H

#include <QObject>

class QSettings;
class QColor;

class CardCounterSettings : public QObject
{
    Q_OBJECT

public:
    CardCounterSettings(const QString &settingsPath, QObject *parent = nullptr);

    QColor color(int counterId) const;

    QString displayName(int counterId) const;

public slots:
    void setColor(int counterId, const QColor &color);

signals:
    void colorChanged(int counterId, const QColor &color);

private:
    QSettings *m_settings;
};

#endif // CARD_COUNTER_SETTINGS_H
