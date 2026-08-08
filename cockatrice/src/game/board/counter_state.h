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
    CounterState(int _id,
                 const QString &_name,
                 const QColor &_color,
                 int _radius,
                 int _value,
                 bool _active = true,
                 QObject *parent = nullptr);

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
    /** @brief Returns whether this counter is active (visible and modifiable). */
    bool isActive() const
    {
        return active;
    }

    void setValue(int newValue);
    /** @brief Sets the active (visible) state and emits activeChanged if it changed. */
    void setActive(bool newActive);

signals:
    void valueChanged(int oldValue, int newValue);
    /** @brief Emitted when the counter's active state changes. */
    void activeChanged(bool newActive);

private:
    int id;
    QString name;
    QColor color;
    int radius;
    int value;
    bool active; ///< Inactive counters are hidden; server rejects modification attempts
};

#endif // COCKATRICE_COUNTER_STATE_H
