#ifndef COCKATRICE_CARD_STATE_H
#define COCKATRICE_CARD_STATE_H

#include <QMap>
#include <QObject>
#include <QPoint>
#include <libcockatrice/card/printing/exact_card.h>
#include <libcockatrice/utility/card_ref.h>

class CardZoneLogic;
class CardItem;
class CardState : public QObject
{
    Q_OBJECT

private:
    int id = -1;
    CardRef cardRef;
    ExactCard exactCard;
    bool tapped = false;
    bool facedown = false;
    QPoint gridPoint;

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

    void cardRefChanged(const CardRef &oldCardRef, const CardRef &newCardRef);
    void cardInfoChanged();
    void cardPixmapUpdated();
    void tappedChanged(bool newTapped, bool canAnimate);
    void facedownChanged(bool newFaceDown);

    void attackingChanged(bool newValue);
    void countersChanged(const QMap<int, int> &newCounters);
    void annotationChanged(const QString &newAnnotation);
    void ptChanged(const QString &newPt);
    void doesntUntapChanged(bool newValue);
    void destroyOnZoneChangeChanged(bool newValue);
    void attachedToChanged(CardItem *newAttachedTo);
    void zoneChanged(CardState *changedCard, CardZoneLogic *newZone);

public:
    explicit CardState(QObject *parent, CardZoneLogic *_zone = nullptr, const CardRef &_cardRef = {}, int _id = -1)
        : QObject(parent), id(_id), cardRef(_cardRef), zone(_zone)
    {
    }

    void resetState(bool keepAnnotations);

    int getId() const
    {
        return id;
    }

    void setId(int _id)
    {
        id = _id;
    }

    const CardRef &getCardRef() const
    {
        return cardRef;
    }

    void setCardRef(const CardRef &_cardRef);

    ExactCard getCard() const
    {
        return exactCard;
    }

    const CardInfo &getCardInfo() const
    {
        return exactCard.getInfo();
    }

    void refreshCardInfo();

    bool getTapped() const
    {
        return tapped;
    }

    void setTapped(bool _tapped, bool canAnimate = false);

    bool getFaceDown() const
    {
        return facedown;
    }

    void setFaceDown(bool _facedown);

    const QPoint &getGridPoint() const
    {
        return gridPoint;
    }

    void setGridPoint(const QPoint &_gridPoint)
    {
        gridPoint = _gridPoint;
    }

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
