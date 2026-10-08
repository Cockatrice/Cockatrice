/**
 * @file card_zone_logic.h
 * @ingroup GameLogicZones
 */
//! \todo Document this file.

#ifndef COCKATRICE_CARD_ZONE_LOGIC_H
#define COCKATRICE_CARD_ZONE_LOGIC_H

#include "../../client/translation.h"
#include "../board/card_list.h"

#include <QLoggingCategory>
#include <QObject>
#include <libcockatrice/protocol/pb/serverinfo_card.pb.h>

inline Q_LOGGING_CATEGORY(CardZoneLogicLog, "card_zone_logic");

class PlayerLogic;
class ZoneViewZoneLogic;
class QMenu;
class QAction;
class QPainter;
class CardDragItem;

class CardZoneLogic : public QObject
{
    Q_OBJECT

signals:
    void cardAdded(CardState *addedCard);
    /** @brief Asks the graphics side to build the card item for @p cardInfo and add it to this zone. */
    void requestCreateCard(const ServerInfo_Card &cardInfo, bool reorganize);
    void cardCountChanged();
    void reorganizeCards();
    void updateGraphics();
    void setGraphicsVisibility(bool visible);
    void retranslateUi();

public:
    explicit CardZoneLogic(PlayerLogic *_player,
                           const QString &_name,
                           bool _hasCardAttr,
                           bool _isShufflable,
                           bool _contentsKnown,
                           QObject *parent = nullptr);

    void addCard(CardState *card, bool reorganize, int x, int y = -1);
    // getCard() finds a card by id.
    CardState *getCard(int cardId);
    void removeCard(CardState *card);
    // takeCard() finds a card by position and removes it from the zone and from all of its views.
    virtual CardState *takeCard(int position, int cardId, bool canResize = true);

    void rawInsertCard(CardState *card, int index)
    {
        cards.insert(index, card);
        emit cardCountChanged();
    }

    [[nodiscard]] const CardList &getCards() const
    {
        return cards;
    }

    void sortCards(const QList<CardList::SortOption> &options)
    {
        cards.sortBy(options);
        emit cardCountChanged();
    }
    [[nodiscard]] QString getName() const
    {
        return name;
    }
    [[nodiscard]] QString getTranslatedName(bool theirOwn, GrammaticalCase gc) const;
    [[nodiscard]] PlayerLogic *getPlayer() const
    {
        return player;
    }
    [[nodiscard]] bool contentsKnown() const
    {
        return cards.getContentsKnown();
    }
    QList<ZoneViewZoneLogic *> &getViews()
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
    PlayerLogic *player;
    QString name;
    CardList cards;
    QList<ZoneViewZoneLogic *> views;
    bool hasCardAttr;
    bool isShufflable;
    bool alwaysRevealTopCard;

    virtual void addCardImpl(CardState *card, int x, int y) = 0;
};

#endif // COCKATRICE_CARD_ZONE_LOGIC_H
