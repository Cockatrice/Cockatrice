#ifndef PHASESTOOLBAR_H
#define PHASESTOOLBAR_H

#include <QFrame>
#include <QList>
#include <QPushButton>

class PhaseButton : public QPushButton {
	Q_OBJECT
private:
	QString phaseText;
public:
	PhaseButton();
	PhaseButton(QIcon);
	void setPhaseText(const QString &_phaseText);
	QString getPhaseText() const { return phaseText; }
public slots:
	void update();
protected:
	void paintEvent(QPaintEvent *event);
};

class PhasesToolbar : public QFrame {
	Q_OBJECT
public:
	PhasesToolbar(QWidget *parent = 0);
	void retranslateUi();
private:
	QList<PhaseButton *> buttonList;
};

#endif
