#ifndef TAB_H
#define TAB_H

#include <QWidget>

class QMenu;

class Tab : public QWidget {
	Q_OBJECT
signals:
	void userEvent();
protected:
	QMenu *tabMenu;
public:
	Tab(QWidget *parent = 0)
		: QWidget(parent), tabMenu(0) { }
	QMenu *getTabMenu() const { return tabMenu; }
	virtual QString getTabText() const = 0;
	virtual void retranslateUi() = 0;
};

#endif
