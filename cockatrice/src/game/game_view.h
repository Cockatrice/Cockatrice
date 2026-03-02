/**
 * @file game_view.h
 * @ingroup GameGraphics
 * @brief TODO: Document this.
 */

#ifndef GAMEVIEW_H
#define GAMEVIEW_H

#include <QGraphicsView>

class GameScene;
class QRubberBand;

class GameView : public QGraphicsView
{
    Q_OBJECT
private:
    QAction *aCloseMostRecentZoneView;
    QRubberBand *rubberBand;
    QPointF selectionOrigin;

protected:
    void resizeEvent(QResizeEvent *event) override;
private slots:
    void startRubberBand(const QPointF &selectionOrigin);
    void resizeRubberBand(const QPointF &cursorPoint);
    void stopRubberBand();
    void refreshShortcuts();
public slots:
    void updateSceneRect(const QRectF &rect);

public:
    explicit GameView(GameScene *scene, QWidget *parent = nullptr);
};

#endif
