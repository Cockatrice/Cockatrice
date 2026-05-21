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

    void setValue(int newValue);

signals:
    void valueChanged(int oldValue, int newValue);

private:
    int id;
    QString name;
    QColor color;
    int radius;
    int value;
};

#endif // COCKATRICE_COUNTER_STATE_H
