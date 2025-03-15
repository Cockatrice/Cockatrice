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
    virtual void
    handleDropEvent(const QList<CardDragItem *> &dragItem, CardZone *startZone, const QPoint &dropPoint) = 0;
    CardZone(Player *_player,
             const QString &_name,
             bool _hasCardAttr,
             bool _isShufflable,
             bool _contentsKnown,
             QGraphicsItem *parent = nullptr);
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
    virtual QPointF closestGridPoint(const QPointF &point);
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
