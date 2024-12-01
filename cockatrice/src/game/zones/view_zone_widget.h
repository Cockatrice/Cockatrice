#ifndef ZONEVIEWWIDGET_H
#define ZONEVIEWWIDGET_H

#include "../../utility/macros.h"

#include <QCheckBox>
#include <QComboBox>
#include <QGraphicsProxyWidget>
#include <QGraphicsWidget>

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

    QPushButton *closeButton;
    QScrollBar *scrollBar;
    ScrollableGraphicsProxyWidget *scrollBarProxy;
    QComboBox groupBySelector;
    QComboBox sortBySelector;
    QCheckBox shuffleCheckBox;
    QCheckBox pileViewCheckBox;

    bool canBeShuffled;
    int extraHeight;
    Player *player;
signals:
    void closePressed(ZoneViewWidget *zv);
private slots:
    void processGroupBy(int value);
    void processSortBy(int value);
    void processSetPileView(QT_STATE_CHANGED_T value);
    void resizeToZoneContents();
    void handleScrollBarChange(int value);
    void zoneDeleted();
    void moveEvent(QGraphicsSceneMoveEvent * /* event */);

public:
    ZoneViewWidget(Player *_player,
                   CardZone *_origZone,
                   int numberCards = 0,
                   bool _revealZone = false,
                   bool _writeableRevealZone = false,
                   const QList<const ServerInfo_Card *> &cardList = QList<const ServerInfo_Card *>());
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
    void closeEvent(QCloseEvent *event);
    void initStyleOption(QStyleOption *option) const;
};

#endif
