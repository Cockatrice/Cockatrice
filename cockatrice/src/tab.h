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
private:
	bool contentsChanged;
public:
	Tab(QWidget *parent = 0)
		: QWidget(parent), tabMenu(0), contentsChanged(false) { }
	QMenu *getTabMenu() const { return tabMenu; }
	bool getContentsChanged() const { return contentsChanged; }
	void setContentsChanged(bool _contentsChanged) { contentsChanged = _contentsChanged; }
	virtual QString getTabText() const = 0;
	virtual void retranslateUi() = 0;
};

#endif
