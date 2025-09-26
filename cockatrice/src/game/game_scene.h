#ifndef GAMESCENE_H
#define GAMESCENE_H

#include "zones/logic/card_zone_logic.h"

#include <QGraphicsScene>
#include <QList>
#include <QLoggingCategory>
#include <QPointer>
#include <QSet>

inline Q_LOGGING_CATEGORY(GameSceneLog, "game_scene");
inline Q_LOGGING_CATEGORY(GameScenePlayerAdditionRemovalLog, "game_scene.player_addition_removal");

class Player;
class PlayerGraphicsItem;
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
    QList<PlayerGraphicsItem *> players;
    QList<QList<PlayerGraphicsItem *>> playersByColumn;
    QList<ZoneViewWidget *> zoneViews;
    QSize viewSize;
    QPointer<CardItem> hoveredCard;
    QBasicTimer *animationTimer;
    QSet<CardItem *> cardsToAnimate;
    int playerRotation;
    void updateHover(const QPointF &scenePos);

public:
    explicit GameScene(PhasesToolbar *_phasesToolbar, QObject *parent = nullptr);
    ~GameScene() override;
    void retranslateUi();
    void processViewSizeChange(const QSize &newSize);
    QTransform getViewTransform() const;
    QTransform getViewportTransform() const;

    void startRubberBand(const QPointF &selectionOrigin);
    void resizeRubberBand(const QPointF &cursorPoint);
    void stopRubberBand();
    QList<Player *> collectActivePlayers(int &firstPlayerIndex) const;
    QList<Player *> rotatePlayers(const QList<Player *> &players, int firstPlayerIndex) const;
    int determineColumnCount(int playerCount) const;
    QSizeF computeSceneSizeAndPlayerLayout(const QList<Player *> &playersPlaying, int columns);
    QList<qreal> calculateMinWidthByColumn() const;
    qreal calculateNewSceneWidth(const QSize &newSize, qreal minWidth) const;
    void resizeColumnsAndPlayers(const QList<qreal> &minWidthByColumn, qreal newWidth);
    CardZone *findTopmostZone(const QList<QGraphicsItem *> &items) const;
    CardItem *findTopmostCardInZone(const QList<QGraphicsItem *> &items, CardZone *zone) const;
    void updateHoveredCard(CardItem *newCard);

    void registerAnimationItem(AbstractCardItem *item);
    void unregisterAnimationItem(AbstractCardItem *card);
public slots:
    void toggleZoneView(Player *player, const QString &zoneName, int numberCards, bool isReversed = false);
    void addRevealedZoneView(Player *player,
                             CardZoneLogic *zone,
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
    bool event(QEvent *event) override;
    void timerEvent(QTimerEvent *event) override;
signals:
    void sigStartRubberBand(const QPointF &selectionOrigin);
    void sigResizeRubberBand(const QPointF &cursorPoint);
    void sigStopRubberBand();
};

#endif
