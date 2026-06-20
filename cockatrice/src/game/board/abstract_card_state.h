#ifndef COCKATRICE_ABSTRACT_CARD_STATE_H
#define COCKATRICE_ABSTRACT_CARD_STATE_H
#include <QObject>
#include <libcockatrice/card/printing/exact_card.h>
#include <libcockatrice/utility/card_ref.h>

class PlayerLogic;

class AbstractCardState : public QObject
{
    Q_OBJECT

signals:
    void cardChanged(ExactCard *card);
    void cardRefChanged(CardRef *cardRef);
    void tappedChanged(bool tapped);
    void faceDownChanged(bool faceDown);
    void cardInfoRefreshed(ExactCard card);
    void colorChanged();

protected:
    PlayerLogic *owner;
    CardRef cardRef;
    ExactCard exactCard;
    int id;
    bool tapped;
    bool facedown;
    QString color;

public slots:
    void refreshCardInfo();

public:
    AbstractCardState(PlayerLogic *_owner = nullptr, const CardRef &cardRef = {}, int _id = -1);
    PlayerLogic *getOwner() const
    {
        return owner;
    }
    void setOwner(PlayerLogic *_owner)
    {
        owner = _owner;
    }
    ExactCard getCard() const
    {
        return exactCard;
    }
    const CardInfo &getCardInfo() const;
    int getId() const
    {
        return id;
    }
    void setId(int _id)
    {
        id = _id;
    }
    QString getName() const
    {
        return cardRef.name;
    }
    QString getProviderId() const
    {
        return cardRef.providerId;
    }
    void setCardRef(const CardRef &_cardRef);
    CardRef getCardRef() const
    {
        return cardRef;
    }
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
    QString getColor() const
    {
        return color;
    }
    void setColor(const QString &_color);
};

#endif // COCKATRICE_ABSTRACT_CARD_STATE_H
