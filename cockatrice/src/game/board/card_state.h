#ifndef COCKATRICE_CARD_STATE_H
#define COCKATRICE_CARD_STATE_H

#include "abstract_card_state.h"

#include <QMap>
#include <QObject>
#include <QPoint>
#include <libcockatrice/protocol/pb/serverinfo_card.pb.h>

class CardZoneLogic;
class CardState : public AbstractCardState
{
    Q_OBJECT

private:
    bool attacking = false;
    QMap<int, int> counters;
    QString annotation;
    QString pt;
    bool doesntUntap = false;
    bool destroyOnZoneChange = false;
    bool visible = false;

    QPoint gridPoint;

    CardState *attachedTo = nullptr;
    QList<CardState *> attachedCards;
    CardZoneLogic *zone = nullptr;

signals:
    void stateChanged();

    void attackingChanged(bool newValue);
    void countersChanged(const QMap<int, int> &newCounters);
    void annotationChanged(const QString &newAnnotation);
    void ptChanged(const QString &newPt);
    void doesntUntapChanged(bool newValue);
    void destroyOnZoneChangeChanged(bool newValue);
    void attachedToChanged(CardState *newAttachedTo);
    void zoneChanged(CardState *changedCard, CardZoneLogic *newZone);
    void visibleChanged(bool visible);

public slots:
    void deleteLater();

public:
    explicit CardState(PlayerLogic *_owner, const CardRef &cardRef = {}, int _id = -1, CardZoneLogic *_zone = nullptr);

    void prepareDelete();
    void processCardInfo(const ServerInfo_Card &_info);
    void resetState(bool keepAnnotations);
    void clearAttachedCards();

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

    CardState *getAttachedTo() const
    {
        return attachedTo;
    }

    [[nodiscard]] QPoint getGridPoint() const
    {
        return gridPoint;
    }
    void setGridPoint(const QPoint &_gridPoint)
    {
        gridPoint = _gridPoint;
    }
    [[nodiscard]] QPoint getGridPos() const
    {
        return gridPoint;
    }

    bool getVisible() const
    {
        return visible;
    }

    void setVisible(bool _visible)
    {
        if (_visible == visible) {
            return;
        }
        visible = _visible;
        emit visibleChanged(visible);
    }

    void setAttachedTo(CardState *_attachedTo);
    void addAttachedCard(CardState *card)
    {
        attachedCards.append(card);
    }
    void removeAttachedCard(CardState *card)
    {
        attachedCards.removeOne(card);
    }
    [[nodiscard]] const QList<CardState *> &getAttachedCards() const
    {
        return attachedCards;
    }
};

#endif // COCKATRICE_CARD_STATE_H
