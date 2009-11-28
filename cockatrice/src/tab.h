#ifndef TAB_H
#define TAB_H

#include <QWidget>

class QMenu;

class Tab : public QWidget {
	Q_OBJECT
protected:
	QMenu *tabMenu;
public:
	Tab(QWidget *parent = 0)
		: QWidget(parent), tabMenu(0) { }
	QMenu *getTabMenu() const { return tabMenu; }
};

#endif
