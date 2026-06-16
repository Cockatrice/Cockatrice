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
    bool isActive() const
    {
        return active;
    }

    void setValue(int newValue);
    void setActive(bool newActive);

signals:
    void valueChanged(int oldValue, int newValue);
    void activeChanged(bool newActive);

private:
    int id;
    QString name;
    QColor color;
    int radius;
    int value;
    bool active;
};

#endif // COCKATRICE_COUNTER_STATE_H
