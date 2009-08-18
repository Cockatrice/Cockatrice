#ifndef PHASESTOOLBAR_H
#define PHASESTOOLBAR_H

#include <QFrame>
#include <QList>
#include <QPushButton>

class Player;

class PhaseButton : public QPushButton {
	Q_OBJECT
private:
	QString phaseText;
	bool active;
public:
	PhaseButton();
	PhaseButton(QIcon);
	void setPhaseText(const QString &_phaseText);
	QString getPhaseText() const { return phaseText; }
	void setActive(bool _active) { active = _active; update(); }
public slots:
	void update();
protected:
	void paintEvent(QPaintEvent *event);
};

class PhasesToolbar : public QFrame {
	Q_OBJECT
private:
	QList<PhaseButton *> buttonList;
public:
	PhasesToolbar(QWidget *parent = 0);
	void retranslateUi();
public slots:
	void setActivePhase(int phase);
private slots:
	void phaseButtonClicked();
signals:
	void signalSetPhase(int phase);
	void signalNextTurn();
};

#endif
