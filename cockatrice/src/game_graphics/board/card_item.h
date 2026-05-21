/**
 * @file card_item.h
 * @ingroup GameGraphicsCards
 */
//! \todo Document this file.

#ifndef CARDITEM_H
#define CARDITEM_H

#include "../../game/board/card_state.h"
#include "../../game/zones/card_zone_logic.h"
#include "abstract_card_item.h"

#include <libcockatrice/network/server/remote/game/server_card.h>
#include <libcockatrice/utility/trice_limits.h>

class CardDatabase;
class CardDragItem;
class CardZone;
class ServerInfo_Card;
class PlayerLogic;
class QAction;
class QColor;

const int ROTATION_DEGREES_PER_FRAME = 10;

class CardItem : public AbstractCardItem
{
    Q_OBJECT
private:
    CardState *state;
    CardZone *zone;

    CardDragItem *dragItem;

    void prepareDelete();
    void handleClickedToPlay(bool shiftHeld);
public slots:
    void deleteLater();

public:
    enum
    {
        Type = typeCard
    };
    [[nodiscard]] int type() const override
    {
        return Type;
    }
    explicit CardItem(CardState *state, CardZone *parent);

    void retranslateUi();
    [[nodiscard]] CardZone *getZone() const
    {
        return zone;
    }
    [[nodiscard]] CardState *getState() const
    {
        return state;
    }
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void resetState(bool keepAnnotations = false);

    bool animationEvent();
    CardDragItem *createDragItem(int _id, const QPointF &_pos, const QPointF &_scenePos, bool forceFaceDown);
    void deleteDragItem();
    void drawArrow(const QColor &arrowColor);
    void drawAttachArrow();
    void playCard(bool faceDown);

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

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
};

#endif
