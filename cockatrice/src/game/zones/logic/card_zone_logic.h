/**
 * @file card_zone_logic.h
 * @ingroup GameLogicZones
 * @brief TODO: Document this.
 */

#ifndef COCKATRICE_CARD_ZONE_LOGIC_H
#define COCKATRICE_CARD_ZONE_LOGIC_H

#include "../../../client/translation.h"
#include "../../board/card_list.h"

#include <QLoggingCategory>
#include <QObject>

inline Q_LOGGING_CATEGORY(CardZoneLogicLog, "card_zone_logic");

class Player;
class ZoneViewZone;
class QMenu;
class QAction;
class QPainter;
class CardDragItem;

class CardZoneLogic : public QObject
{
    Q_OBJECT

signals:
    void cardAdded(CardItem *addedCard);
    void cardCountChanged();
    void reorganizeCards();
    void updateGraphics();
    void setGraphicsVisibility(bool visible);
    void retranslateUi();

public:
    explicit CardZoneLogic(Player *_player,
                           const QString &_name,
                           bool _hasCardAttr,
                           bool _isShufflable,
                           bool _contentsKnown,
                           QObject *parent = nullptr);

    void addCard(CardItem *card, bool reorganize, int x, int y = -1);
    // getCard() finds a card by id.
    CardItem *getCard(int cardId);
    void removeCard(CardItem *card);
    // takeCard() finds a card by position and removes it from the zone and from all of its views.
    virtual CardItem *takeCard(int position, int cardId, bool canResize = true);

    void rawInsertCard(CardItem *card, int index)
    {
        cards.insert(index, card);
    };

    [[nodiscard]] const CardList &getCards() const
    {
        return cards;
    }

    void sortCards(const QList<CardList::SortOption> &options)
    {
        cards.sortBy(options);
    }
    [[nodiscard]] QString getName() const
    {
        return name;
    }
    [[nodiscard]] QString getTranslatedName(bool theirOwn, GrammaticalCase gc) const;
    [[nodiscard]] Player *getPlayer() const
    {
        return player;
    }
    [[nodiscard]] bool contentsKnown() const
    {
        return cards.getContentsKnown();
    }
    QList<ZoneViewZone *> &getViews()
    {
        return views;
    }
    void setAlwaysRevealTopCard(bool _alwaysRevealTopCard)
    {
        alwaysRevealTopCard = _alwaysRevealTopCard;
    }
    [[nodiscard]] bool getAlwaysRevealTopCard() const
    {
        return alwaysRevealTopCard;
    }
    [[nodiscard]] bool getHasCardAttr() const
    {
        return hasCardAttr;
    }
    [[nodiscard]] bool getIsShufflable() const
    {
        return isShufflable;
    }
    void clearContents();

public slots:
    void moveAllToZone();

private slots:
    void refreshCardInfos();

protected:
    Player *player;
    QString name;
    CardList cards;
    QList<ZoneViewZone *> views;
    bool hasCardAttr;
    bool isShufflable;
    bool alwaysRevealTopCard;

    virtual void addCardImpl(CardItem *card, int x, int y) = 0;
};

#endif // COCKATRICE_CARD_ZONE_LOGIC_H
