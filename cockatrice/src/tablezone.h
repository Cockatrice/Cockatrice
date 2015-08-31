#ifndef TABLEZONE_H
#define TABLEZONE_H


#include "selectzone.h"
#include "abstractcarditem.h"


/*
* TableZone is the grid based rect where CardItems may be placed.
* It is the main play zone and can be customized with background images.
*
* TODO: Refactor methods to make more readable, extract some logic to
* private methods (Im looking at you TableZone::reorganizeCards())
*/
class TableZone : public SelectZone {
    Q_OBJECT

signals:
    void sizeChanged();

private:
    static const int TABLEROWS = 3;

    /*
    Margins between table edges and cards, paddings between cards
    */
    static const int MARGIN_LEFT = 20;
    static const int MARGIN_RIGHT = 15;
    static const int MARGIN_TOP = 10;
    static const int MARGIN_BOTTOM = 30;
    static const int PADDING_X  = 35;
    static const int PADDING_Y = 30;

    /*
    Minimum width of the table zone including margins.
    */
    static const int MIN_WIDTH = MARGIN_LEFT + (5 * CARD_WIDTH) + MARGIN_RIGHT;

    /*
    Offset sizes when cards are stacked on each other in the grid
    */
    static const int STACKED_CARD_OFFSET_X = CARD_WIDTH / 3;
    static const int STACKED_CARD_OFFSET_Y = PADDING_Y / 3;

    /*
    Width of the box line drawn in the margin around the active player's area
    */
    static const int BOX_LINE_WIDTH = 10;

    /*
    Default inactive mask and border gradient
    */
    static const QColor BACKGROUND_COLOR;
    static const QColor FADE_MASK;
    static const QColor GRADIENT_COLOR;
    static const QColor GRADIENT_COLORLESS;
    
    /*
       Size and shape variables
     */
    int width;
    int height;
    int currentMinimumWidth;

    /*
    Internal cache for widths of stacks of cards by row and column.
    */
    QMap<int, int> cardStackWidth;

    /*
       Holds any custom background image for the TableZone
     */
    QPixmap backgroundPixelMap;

    /*
       If this TableZone is currently active
     */
    bool active;

    bool isInverted() const;

private slots:
    /**
       Loads in any found custom background and updates
     */
    void updateBg();

public slots:
    /**
       Reorganizes CardItems in the TableZone
     */
    void reorganizeCards();

public:
    /**
       Constructs TableZone.

       @param _p the Player
       @param parent defaults to null
     */
    TableZone(Player *_p, QGraphicsItem *parent = 0);
    
    /**
       @return a QRectF of the TableZone bounding box.
     */
    QRectF boundingRect() const;

    /**
       Render the TableZone

       @param painter 
       @param option
     */
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    /**
       Toggles the selected items as tapped.
     */
    void toggleTapped();

    /**
       See HandleDropEventByGrid
     */
    void handleDropEvent(const QList<CardDragItem *> &dragItems, CardZone *startZone, const QPoint &dropPoint);

    /**
       Handles the placement of cards
     */
    void handleDropEventByGrid(const QList<CardDragItem *> &dragItems, CardZone *startZone, const QPoint &gridPoint);

    /**
       @return CardItem from grid location
     */
    CardItem *getCardFromGrid(const QPoint &gridPoint) const;

    /**
       @return CardItem from coordinate location
     */
    CardItem *getCardFromCoords(const QPointF &point) const;

    QPointF closestGridPoint(const QPointF &point);

    static int clampValidTableRow(const int row);

    /**
       Removes a card from view.

       @param position card position
       @param cardId id of card to take
       @param canResize defaults to true
       @return CardItem that has been removed
     */
    CardItem *takeCard(int position, int cardId, bool canResize = true);

    /**
       Resizes the TableZone in case CardItems are within or 
       outside of the TableZone constraints.
     */
    void resizeToContents();

    int getMinimumWidth() const { return currentMinimumWidth; }
    void setWidth(qreal _width) { prepareGeometryChange(); width = _width; }
    qreal getWidth() const { return width; }
    void setActive(bool _active) { active = _active; update(); }

protected:
    void addCardImpl(CardItem *card, int x, int y);

private:
    void paintZoneOutline(QPainter *painter);
    void paintLandDivider(QPainter *painter);

    /*
    Calculates card stack widths so mapping functions work properly
    */
    void computeCardStackWidths();

    /*
    Mapping functions for points to/from gridpoints.
    */
    QPointF mapFromGrid(QPoint gridPoint) const;
    QPoint mapToGrid(const QPointF &mapPoint) const;

    /*
    Helper function to create a single key from a card stack location.
    */
    int getCardStackMapKey (int x, int y) const { return x + (y * 1000); }
};

#endif
