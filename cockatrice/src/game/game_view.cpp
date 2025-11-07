#include "game_view.h"

#include "../client/settings/cache_settings.h"
#include "game_scene.h"

#include <QAction>
#include <QResizeEvent>
#include <QRubberBand>

GameView::GameView(GameScene *scene, QWidget *parent) : QGraphicsView(scene, parent), rubberBand(0)
{
    setBackgroundBrush(QBrush(QColor(0, 0, 0)));
    setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing);
    setFocusPolicy(Qt::ClickFocus);
    setViewportUpdateMode(BoundingRectViewportUpdate);

    connect(scene, &GameScene::sceneRectChanged, this, &GameView::updateSceneRect);

    connect(scene, &GameScene::sigStartRubberBand, this, &GameView::startRubberBand);
    connect(scene, &GameScene::sigResizeRubberBand, this, &GameView::resizeRubberBand);
    connect(scene, &GameScene::sigStopRubberBand, this, &GameView::stopRubberBand);

    aCloseMostRecentZoneView = new QAction(this);

    connect(aCloseMostRecentZoneView, &QAction::triggered, scene, &GameScene::closeMostRecentZoneView);
    addAction(aCloseMostRecentZoneView);
    connect(&SettingsCache::instance().shortcuts(), &ShortcutsSettings::shortCutChanged, this,
            &GameView::refreshShortcuts);
    refreshShortcuts();
    rubberBand = new QRubberBand(QRubberBand::Rectangle, this);
}

void GameView::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);

    GameScene *s = dynamic_cast<GameScene *>(scene());
    if (s)
        s->processViewSizeChange(event->size());

    updateSceneRect(scene()->sceneRect());
}

void GameView::updateSceneRect(const QRectF &rect)
{
    fitInView(rect, Qt::KeepAspectRatio);
}

void GameView::startRubberBand(const QPointF &_selectionOrigin)
{
    selectionOrigin = _selectionOrigin;
    rubberBand->setGeometry(QRect(mapFromScene(selectionOrigin), QSize(0, 0)));
    rubberBand->show();
}

void GameView::resizeRubberBand(const QPointF &cursorPoint)
{
    if (rubberBand)
        rubberBand->setGeometry(QRect(mapFromScene(selectionOrigin), cursorPoint.toPoint()).normalized());
}

void GameView::stopRubberBand()
{
    rubberBand->hide();
}

void GameView::refreshShortcuts()
{
    aCloseMostRecentZoneView->setShortcuts(
        SettingsCache::instance().shortcuts().getShortcut("Player/aCloseMostRecentZoneView"));
}
