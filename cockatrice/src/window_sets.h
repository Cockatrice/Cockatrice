#ifndef WINDOW_SETS_H
#define WINDOW_SETS_H

#include <QMainWindow>

class SetsModel;
class QTreeView;
class CardDatabase;

class WndSets : public QMainWindow {
	Q_OBJECT
private:
	SetsModel *model;
	QTreeView *view;
public:
	WndSets(QWidget *parent = 0);
	~WndSets();
};

#endif
