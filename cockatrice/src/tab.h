#ifndef TAB_H
#define TAB_H

#include <QWidget>

class QMenu;
class TabSupervisor;

class Tab : public QWidget {
	Q_OBJECT
signals:
	void userEvent(bool globalEvent = true);
protected:
	QMenu *tabMenu;
	TabSupervisor *tabSupervisor;
private:
	bool contentsChanged;
public:
	Tab(TabSupervisor *_tabSupervisor, QWidget *parent = 0)
		: QWidget(parent), tabMenu(0), tabSupervisor(_tabSupervisor), contentsChanged(false) { }
	QMenu *getTabMenu() const { return tabMenu; }
	bool getContentsChanged() const { return contentsChanged; }
	void setContentsChanged(bool _contentsChanged) { contentsChanged = _contentsChanged; }
	virtual QString getTabText() const = 0;
	virtual void retranslateUi() = 0;
	virtual void closeRequest() { }
};

#endif
