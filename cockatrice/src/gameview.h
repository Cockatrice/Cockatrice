#ifndef GAMEVIEW_H
#define GAMEVIEW_H

#include <QGraphicsView>

class GameView : public QGraphicsView {
	Q_OBJECT
protected:
	void resizeEvent(QResizeEvent *event);
public slots:
	void updateSceneRect(const QRectF &rect);
public:
	GameView(QGraphicsScene *scene, QWidget *parent = 0);
};

#endif
