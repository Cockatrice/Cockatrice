#ifndef COCKATRICE_COUNTER_STATE_H
#define COCKATRICE_COUNTER_STATE_H

#include <QColor>
#include <QObject>
#include <QString>
#include <libcockatrice/protocol/pb/serverinfo_counter.pb.h>

class CounterState : public QObject
{
    Q_OBJECT
public:
    CounterState(int id, const QString &name, const QColor &color, int radius, int value, QObject *parent = nullptr);

    static CounterState *fromProto(const ServerInfo_Counter &counter, QObject *parent = nullptr);

    int getId() const
    {
        return id;
    }
    QString getName() const
    {
        return name;
    }
    QColor getColor() const
    {
        return color;
    }
    int getRadius() const
    {
        return radius;
    }
    int getValue() const
    {
        return value;
    }

    /**
     * @brief Set the counter value.
     * @param newValue The new value.
     * @param skipDamageAnimation When true, valueChanged is emitted with skipDamageAnimation=true, letting views
     * suppress damage-related feedback (e.g. battlefield shimmer, life counter flash) for values set during replay
     * rewinds.
     */
    void setValue(int newValue, bool skipDamageAnimation = false);

signals:
    /**
     * @brief Emitted whenever the value changes.
     * @param oldValue The previous value.
     * @param newValue The new value.
     * @param skipDamageAnimation True when the change should not trigger damage/life-change feedback in views.
     */
    void valueChanged(int oldValue, int newValue, bool skipDamageAnimation);

private:
    int id;
    QString name;
    QColor color;
    int radius;
    int value;
};

#endif // COCKATRICE_COUNTER_STATE_H
