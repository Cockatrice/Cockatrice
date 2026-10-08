#ifndef COCKATRICE_CARD_STATE_H
#define COCKATRICE_CARD_STATE_H

#include <QList>
#include <QMap>
#include <QObject>
#include <QPoint>
#include <QVariant>
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

    CardState *attachedTo = nullptr;
    QList<CardItem *> attachedCards;
    CardZoneLogic *zone = nullptr;

signals:
    void stateChanged();

    void cardRefChanged(const CardRef &oldCardRef, const CardRef &newCardRef);
    void cardInfoChanged();
    void cardPixmapUpdated();
    void tappedChanged(bool newTapped, bool canAnimate);
    void facedownChanged(bool newFaceDown);
    void stateReset();

    void attackingChanged(bool newValue);
    void countersChanged(const QMap<int, int> &newCounters);
    void annotationChanged(const QString &newAnnotation);
    void ptChanged(const QString &newPt);
    void doesntUntapChanged(bool newValue);
    void destroyOnZoneChangeChanged(bool newValue);
    void attachedToChanged(CardState *newAttachedTo);
    void zoneChanged(CardState *changedCard, CardZoneLogic *newZone);

    void viewDeleteRequested();

public:
    explicit CardState(QObject *parent, CardZoneLogic *_zone = nullptr, const CardRef &_cardRef = {}, int _id = -1)
        : QObject(parent), id(_id), cardRef(_cardRef), zone(_zone)
    {
    }

    void resetState(bool keepAnnotations);

    /**
     * @brief Asks the card view owning this state to tear itself down.
     *
     * The state never holds a card item directly, so it cannot delete one. It asks instead and lets the view run
     * its own teardown, which detaches attachments, drops it from the active card slot and unregisters its
     * animations before the graphics object goes away.
     */
    void deleteView();

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

    /**
     * @brief Parses a string representing a p/t in order to extract the values from it.
     *
     * If the string contains '/', the string will be split at the '/' and each side will be parsed separately,
     * which means the result list will have two elements.
     *
     * If '/' is not found, then the entire string is parsed together, which means the result list will
     * have a single element.
     *
     * If either side of the split is empty, there will also only be a single element in the result list.
     *
     * This function will attempt to parse each substring as an int first, handling plus and minus prefixes.
     * If successful, it will put the parsed value into the QVariant as an int.
     * If failed, it will just put the substring into the QVariant as a QString.
     *
     * @param pt The p/t string
     * @return A QVariantList that can contain one or two elements, where each QVariant can be either int or QString
     */
    static QVariantList parsePT(const QString &pt);

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

    void setAttachedTo(CardState *_attachedTo);

    void addAttachedCard(CardItem *card)
    {
        attachedCards.append(card);
    }

    void removeAttachedCard(CardItem *card)
    {
        attachedCards.removeOne(card);
    }

    const QList<CardItem *> &getAttachedCards() const
    {
        return attachedCards;
    }
};

#endif // COCKATRICE_CARD_STATE_H
