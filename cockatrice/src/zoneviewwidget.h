#ifndef ZONEVIEWWIDGET_H
#define ZONEVIEWWIDGET_H

#include <QGraphicsWidget>
#include <QCheckBox>

class QLabel;
class QPushButton;
class CardZone;
class ZoneViewZone;
class Player;
class CardDatabase;
class QScrollBar;
class QCheckBox;
class GameScene;
class ServerInfo_Card;
class QGraphicsSceneMouseEvent;
class QGraphicsSceneWheelEvent;
class QStyleOption;

class ZoneViewWidget : public QGraphicsWidget {
    Q_OBJECT
private:
    ZoneViewZone *zone;
    QGraphicsWidget *zoneContainer;

    QPushButton *closeButton;
    QScrollBar *scrollBar;
    QCheckBox sortByNameCheckBox;
    QCheckBox sortByTypeCheckBox;
    QCheckBox shuffleCheckBox;
    QCheckBox pileViewCheckBox;
    
    bool canBeShuffled;
    int extraHeight;
    Player *player;
signals:
    void closePressed(ZoneViewWidget *zv);
private slots:
    void processSortByType(int value);
    void processSortByName(int value);
    void processSetPileView(int value);
    void resizeToZoneContents();
    void handleWheelEvent(QGraphicsSceneWheelEvent *event);
    void handleScrollBarChange(int value);
    void zoneDeleted();
    void moveEvent(QGraphicsSceneMoveEvent * /* event */);
public:
    ZoneViewWidget(Player *_player, CardZone *_origZone, int numberCards = 0, bool _revealZone = false, bool _writeableRevealZone = false, const QList<const ServerInfo_Card *> &cardList = QList<const ServerInfo_Card *>());
    ZoneViewZone *getZone() const { return zone; }
    void retranslateUi();
protected:
    void closeEvent(QCloseEvent *event);
    void initStyleOption(QStyleOption *option) const;
};

#endif
