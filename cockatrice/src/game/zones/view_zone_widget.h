/**
 * @file view_zone_widget.h
 * @ingroup GameGraphicsZones
 * @brief TODO: Document this.
 */
#ifndef ZONEVIEWWIDGET_H
#define ZONEVIEWWIDGET_H

#include "logic/card_zone_logic.h"

#include <QCheckBox>
#include <QComboBox>
#include <QGraphicsProxyWidget>
#include <QGraphicsWidget>
#include <QLineEdit>
#include <QPointer>
#include <libcockatrice/utility/macros.h>

class QLabel;
class QPushButton;
class CardZone;
class ZoneViewZone;
class Player;
class CardDatabase;
class QScrollBar;
class GameScene;
class ServerInfo_Card;
class QGraphicsSceneMouseEvent;
class QGraphicsSceneWheelEvent;
class QStyleOption;
class QGraphicsView;
class QWidget;

class ScrollableGraphicsProxyWidget : public QGraphicsProxyWidget
{
    Q_OBJECT
public slots:
    void recieveWheelEvent(QGraphicsSceneWheelEvent *event)
    {
        wheelEvent(event);
    }
};

/**
 * A QGraphicsWidget that holds a ZoneViewZone.
 *
 * Some zone views allow sorting.
 * This widget will display the sort options when relevant, and forward the values of the options to the ZoneViewZone.
 */
class ZoneViewWidget : public QGraphicsWidget
{
    Q_OBJECT
private:
    ZoneViewZone *zone;
    QGraphicsWidget *zoneContainer;

    QScrollBar *scrollBar;
    ScrollableGraphicsProxyWidget *scrollBarProxy;

    QLineEdit searchEdit;
    QComboBox groupBySelector;
    QComboBox sortBySelector;
    QCheckBox shuffleCheckBox;
    QCheckBox pileViewCheckBox;

    bool canBeShuffled;
    int extraHeight;
    Player *player;

    bool draggingWindow = false;
    QPoint dragStartScreenPos;
    QPointF dragStartItemPos;
    QPointer<QGraphicsView> dragView;

    void stopWindowDrag();
    void startWindowDrag(QGraphicsSceneMouseEvent *event);
    QRectF closeButtonRect(QWidget *styleWidget) const;
    /**
     * @brief Resolves the QGraphicsView to use for drag coordinate mapping
     *
     * @param eventWidget QWidget that originated the mouse event
     * @return The resolved QGraphicsView
     */
    QGraphicsView *findDragView(QWidget *eventWidget) const;
    /**
     * @brief Calculates the desired widget position while dragging
     *
     * @param screenPos Global screen coordinates of the current mouse position
     * @param scenePos Scene coordinates of the current mouse position
     * @param buttonDownScenePos Scene coordinates of the initial mouse press position
     *
     * @return The new widget position in scene coordinates
     */
    QPointF
    calcDraggedWindowPos(const QPoint &screenPos, const QPointF &scenePos, const QPointF &buttonDownScenePos) const;

    void resizeScrollbar(qreal newZoneHeight);
signals:
    void closePressed(ZoneViewWidget *zv);
private slots:
    void processGroupBy(int value);
    void processSortBy(int value);
    void processSetPileView(QT_STATE_CHANGED_T value);
    void resizeToZoneContents(bool forceInitialHeight = false);
    void handleScrollBarChange(int value);
    void zoneDeleted();
    void resizeEvent(QGraphicsSceneResizeEvent * /* event */) override;
    void expandWindow();

public:
    ZoneViewWidget(Player *_player,
                   CardZoneLogic *_origZone,
                   int numberCards = 0,
                   bool _revealZone = false,
                   bool _writeableRevealZone = false,
                   const QList<const ServerInfo_Card *> &cardList = QList<const ServerInfo_Card *>(),
                   bool _isReversed = false);
    ZoneViewZone *getZone() const
    {
        return zone;
    }
    Player *getPlayer() const
    {
        return player;
    }
    void retranslateUi();

protected:
    void closeEvent(QCloseEvent *event) override;
    void initStyleOption(QStyleOption *option) const override;
    bool windowFrameEvent(QEvent *event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
};

#endif
