#ifndef GAMEVIEW_H
#define GAMEVIEW_H

#include <QGraphicsView>

class GameView : public QGraphicsView {
protected:
	void resizeEvent(QResizeEvent *event);
public:
	void scaleToScene();
	GameView(QGraphicsScene *scene, QWidget *parent = 0);
};

#endif
