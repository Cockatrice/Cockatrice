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
    void countersChanged();
    void annotationChanged();
    void ptChanged();
    void doesntUntapChanged(bool);
    void destroyOnZoneChangeChanged(bool);
    void attachedToChanged(CardItem *);
    void zoneChanged(CardZoneLogic *);

public:
    explicit CardState(QObject *parent, CardZoneLogic *_zone) : QObject(parent), zone(_zone)
    {
    }

    void resetState(bool keepAnnotations)
    {
        attacking = false;
        counters.clear();
        pt.clear();
        if (!keepAnnotations) {
            annotation.clear();
        }
        attachedTo = nullptr;
    }

    CardZoneLogic *getZone() const
    {
        return zone;
    }

    void setZone(CardZoneLogic *_zone)
    {
        if (zone == _zone) {
            return;
        }

        zone = _zone;
        emit zoneChanged(zone);
        emit stateChanged();
    }

    bool getAttacking() const
    {
        return attacking;
    }
    void setAttacking(bool v)
    {
        if (attacking == v) {
            return;
        }
        attacking = v;
        emit attackingChanged(v);
        emit stateChanged();
    }

    const QMap<int, int> &getCounters() const
    {
        return counters;
    }

    void insertCounter(int id, int value)
    {
        counters.insert(id, value);

        emit countersChanged();
        emit stateChanged();
    }

    void setCounter(int id, int value)
    {
        if (value) {
            counters[id] = value;
        } else {
            counters.remove(id);
        }

        emit countersChanged();
        emit stateChanged();
    }

    void clearCounters()
    {
        counters.clear();
        emit countersChanged();
        emit stateChanged();
    }

    QString getAnnotation() const
    {
        return annotation;
    }
    void setAnnotation(const QString &a)
    {
        if (annotation == a) {
            return;
        }
        annotation = a;
        emit annotationChanged();
        emit stateChanged();
    }

    QString getPT() const
    {
        return pt;
    }
    void setPT(const QString &v)
    {
        if (pt == v) {
            return;
        }
        pt = v;
        emit ptChanged();
        emit stateChanged();
    }

    bool getDoesntUntap() const
    {
        return doesntUntap;
    }
    void setDoesntUntap(bool v)
    {
        if (doesntUntap == v) {
            return;
        }
        doesntUntap = v;
        emit doesntUntapChanged(v);
        emit stateChanged();
    }

    bool getDestroyOnZoneChange() const
    {
        return destroyOnZoneChange;
    }

    void setDestroyOnZoneChange(bool v)
    {
        if (destroyOnZoneChange == v) {
            return;
        }

        destroyOnZoneChange = v;
        emit destroyOnZoneChangeChanged(v);
        emit stateChanged();
    }

    CardItem *getAttachedTo() const
    {
        return attachedTo;
    }

    void setAttachedTo(CardItem *c)
    {
        if (attachedTo == c) {
            return;
        }
        attachedTo = c;
        emit attachedToChanged(c);
        emit stateChanged();
    }
};

#endif // COCKATRICE_CARD_STATE_H
