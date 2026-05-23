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

    void attackingChanged(bool newValue);
    void countersChanged(const QMap<int, int> &newCounters);
    void annotationChanged(const QString &newAnnotation);
    void ptChanged(const QString &newPt);
    void doesntUntapChanged(bool newValue);
    void destroyOnZoneChangeChanged(bool newValue);
    void attachedToChanged(CardItem *newAttachedTo);
    void zoneChanged(CardState *changedCard, CardZoneLogic *newZone);

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
    void setAttacking(bool _attacking);

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

    void setAnnotation(const QString &_annotation);

    QString getPT() const
    {
        return pt;
    }

    void setPT(const QString &_pt);

    bool getDoesntUntap() const
    {
        return doesntUntap;
    }

    void setDoesntUntap(bool _doesntUntap);

    bool getDestroyOnZoneChange() const
    {
        return destroyOnZoneChange;
    }

    void setDestroyOnZoneChange(bool _destroyOnZoneChange);

    CardItem *getAttachedTo() const
    {
        return attachedTo;
    }

    void setAttachedTo(CardItem *_attachedTo);
};

#endif // COCKATRICE_CARD_STATE_H
