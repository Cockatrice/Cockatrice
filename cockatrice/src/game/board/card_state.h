#ifndef COCKATRICE_CARD_STATE_H
#define COCKATRICE_CARD_STATE_H

#include <QMap>
#include <QObject>

class CardZoneLogic;
class CardItem;
class CardState : public QObject
{
    Q_OBJECT

private:
    bool attacking = false;
    QMap<int, int> counters;
    QString annotation;
    QString pt;
    bool doesntUntap = false;
    bool destroyOnZoneChange = false;

    CardItem *attachedTo = nullptr;
    CardZoneLogic *zone = nullptr;

signals:
    void stateChanged();

    void attackingChanged(bool);
    void countersChanged(QMap<int, int>);
    void annotationChanged(QString);
    void ptChanged(QString);
    void doesntUntapChanged(bool);
    void destroyOnZoneChangeChanged(bool);
    void attachedToChanged(CardItem *);
    void zoneChanged(CardZoneLogic *);

public:
    explicit CardState(QObject *parent, CardZoneLogic *_zone) : QObject(parent), zone(_zone)
    {
    }

    void resetState(bool keepAnnotations);

    CardZoneLogic *getZone() const
    {
        return zone;
    }

    void setZone(CardZoneLogic *_zone);

    bool getAttacking() const
    {
        return attacking;
    }
    void setAttacking(bool v);

    const QMap<int, int> &getCounters() const
    {
        return counters;
    }

    void insertCounter(int id, int value);

    void setCounter(int id, int value);

    void clearCounters();

    QString getAnnotation() const
    {
        return annotation;
    }

    void setAnnotation(const QString &a);

    QString getPT() const
    {
        return pt;
    }

    void setPT(const QString &v);

    bool getDoesntUntap() const
    {
        return doesntUntap;
    }

    void setDoesntUntap(bool v);

    bool getDestroyOnZoneChange() const
    {
        return destroyOnZoneChange;
    }

    void setDestroyOnZoneChange(bool v);

    CardItem *getAttachedTo() const
    {
        return attachedTo;
    }

    void setAttachedTo(CardItem *c);
};

#endif // COCKATRICE_CARD_STATE_H
