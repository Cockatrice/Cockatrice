#ifndef PHASESTOOLBAR_H
#define PHASESTOOLBAR_H

#include <QFrame>
#include <QList>

class QPushButton;

class PhasesToolbar : public QFrame {
	Q_OBJECT
public:
	PhasesToolbar(QWidget *parent = 0);
private:
	QList<QPushButton *> buttonList;
};

#endif
