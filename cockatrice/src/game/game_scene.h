#ifndef GAMESCENE_H
#define GAMESCENE_H

#include <QGraphicsScene>
#include <QList>
#include <QPointer>
#include <QSet>

class Player;
class ZoneViewWidget;
class CardZone;
class AbstractCardItem;
class CardItem;
class ServerInfo_Card;
class PhasesToolbar;
class QBasicTimer;

class GameScene : public QGraphicsScene
{
    Q_OBJECT
private:
    static const int playerAreaSpacing = 5;

    PhasesToolbar *phasesToolbar;
    QList<Player *> players;
    QList<QList<Player *>> playersByColumn;
    QList<ZoneViewWidget *> zoneViews;
    QSize viewSize;
    QPointer<CardItem> hoveredCard;
    QBasicTimer *animationTimer;
    QSet<CardItem *> cardsToAnimate;
    int playerRotation;
    void updateHover(const QPointF &scenePos);

public:
    GameScene(PhasesToolbar *_phasesToolbar, QObject *parent = nullptr);
    ~GameScene();
    void retranslateUi();
    void processViewSizeChange(const QSize &newSize);
    QTransform getViewTransform() const;
    QTransform getViewportTransform() const;

    void startRubberBand(const QPointF &selectionOrigin);
    void resizeRubberBand(const QPointF &cursorPoint);
    void stopRubberBand();

    void registerAnimationItem(AbstractCardItem *item);
    void unregisterAnimationItem(AbstractCardItem *card);
public slots:
    void toggleZoneView(Player *player, const QString &zoneName, int numberCards, bool isReversed = false);
    void addRevealedZoneView(Player *player,
                             CardZone *zone,
                             const QList<const ServerInfo_Card *> &cardList,
                             bool withWritePermission);
    void removeZoneView(ZoneViewWidget *item);
    void addPlayer(Player *player);
    void removePlayer(Player *player);
    void clearViews();
    void closeMostRecentZoneView();
    void adjustPlayerRotation(int rotationAdjustment);
    void rearrange();

protected:
    bool event(QEvent *event);
    void timerEvent(QTimerEvent *event);
signals:
    void sigStartRubberBand(const QPointF &selectionOrigin);
    void sigResizeRubberBand(const QPointF &cursorPoint);
    void sigStopRubberBand();
};

#endif
