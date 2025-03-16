#ifndef CARDZONE_H
#define CARDZONE_H

#include "../../client/translation.h"
#include "../board/abstract_graphics_item.h"
#include "../cards/card_list.h"

#include <QLoggingCategory>
#include <QString>

inline Q_LOGGING_CATEGORY(CardZoneLog, "card_zone");

class Player;
class ZoneViewZone;
class QMenu;
class QAction;
class QPainter;
class CardDragItem;

/**
 * A zone in the game that can contain cards.
 * This class contains methods to get and modify the cards that are contained inside this zone.
 *
 * The cards are stored as a list of `CardItem*`.
 */
class CardZone : public AbstractGraphicsItem
{
    Q_OBJECT
protected:
    Player *player;
    QString name;
    CardList cards;
    QList<ZoneViewZone *> views;
    QMenu *menu;
    QAction *doubleClickAction;
    bool hasCardAttr;
    bool isShufflable;
    bool isView;
    bool alwaysRevealTopCard;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void addCardImpl(CardItem *card, int x, int y) = 0;
signals:
    void cardCountChanged();
public slots:
    void moveAllToZone();
    bool showContextMenu(const QPoint &screenPos);

private slots:
    void refreshCardInfos();

public:
    enum
    {
        Type = typeZone
    };
    int type() const override
    {
        return Type;
    }

    /* Called when a card is dragged on top of the zone.

       Return `true` to accept the drag, `false` otherwise.
    */
    virtual bool dragEnter(CardDragItem *dragItem, const QPointF &pos);

    /* Called when a card that has been accepted by `dragEnter` is moved over
       the zone.
    */
    virtual void dragMove(CardDragItem *dragItem, const QPointF &pos);

    /* Called when a card that has been accepted by `dragEnter` is dropped onto
       the zone. */
    virtual void dragAccept(CardDragItem *dragItem, const QPointF &pos);

    /* Called when a card that has been accepted by `dragEnter` leaves the zone.
     */
    virtual void dragLeave(CardDragItem *dragItem);

    virtual void
    handleDropEvent(const QList<CardDragItem *> &dragItem, CardZone *startZone, const QPoint &dropPoint) = 0;
    CardZone(Player *_player,
             const QString &_name,
             bool _hasCardAttr,
             bool _isShufflable,
             bool _contentsKnown,
             QGraphicsItem *parent = nullptr,
             bool _isView = false);
    void retranslateUi();
    void clearContents();
    bool getHasCardAttr() const
    {
        return hasCardAttr;
    }
    bool getIsShufflable() const
    {
        return isShufflable;
    }
    QMenu *getMenu() const
    {
        return menu;
    }
    void setMenu(QMenu *_menu, QAction *_doubleClickAction = 0)
    {
        menu = _menu;
        doubleClickAction = _doubleClickAction;
    }
    QString getName() const
    {
        return name;
    }
    QString getTranslatedName(bool theirOwn, GrammaticalCase gc) const;
    Player *getPlayer() const
    {
        return player;
    }
    bool contentsKnown() const
    {
        return cards.getContentsKnown();
    }
    const CardList &getCards() const
    {
        return cards;
    }
    void addCard(CardItem *card, bool reorganize, int x, int y = -1);
    // getCard() finds a card by id.
    CardItem *getCard(int cardId, const QString &cardName);
    // takeCard() finds a card by position and removes it from the zone and from all of its views.
    virtual CardItem *takeCard(int position, int cardId, bool canResize = true);
    void removeCard(CardItem *card);
    QList<ZoneViewZone *> &getViews()
    {
        return views;
    }
    virtual void reorganizeCards() = 0;
    bool getIsView() const
    {
        return isView;
    }
    bool getAlwaysRevealTopCard() const
    {
        return alwaysRevealTopCard;
    }
    void setAlwaysRevealTopCard(bool _alwaysRevealTopCard)
    {
        alwaysRevealTopCard = _alwaysRevealTopCard;
    }
};

#endif
