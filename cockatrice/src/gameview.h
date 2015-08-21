#ifndef GAMEVIEW_H
#define GAMEVIEW_H

#include <QGraphicsView>

class QRubberBand;

class GameView : public QGraphicsView {
    Q_OBJECT
private:
    QAction *aCloseMostRecentZoneView;
    QRubberBand *rubberBand;
    QPointF selectionOrigin;
protected:
    void resizeEvent(QResizeEvent *event);
private slots:
    void startRubberBand(const QPointF &selectionOrigin);
    void resizeRubberBand(const QPointF &cursorPoint);
    void stopRubberBand();
    void refreshShortcuts();
public slots:
    void updateSceneRect(const QRectF &rect);
public:
    GameView(QGraphicsScene *scene, QWidget *parent = 0);
};

#endif
