#ifndef PHASESTOOLBAR_H
#define PHASESTOOLBAR_H

#include <QFrame>
#include <QList>
#include <QPushButton>

class Player;
class GameCommand;

class PhaseButton : public QPushButton {
	Q_OBJECT
private:
	QString phaseText;
	bool active;
	int activeAnimationCounter;
	QTimer *activeAnimationTimer;
	QAction *doubleClickAction;
	QPixmap pixmap;
	
	void updatePixmap(QPixmap &pixmap);
private slots:
	void updateAnimation();
public:
	PhaseButton(const QIcon &icon, QAction *_doubleClickAction = 0);
	void setPhaseText(const QString &_phaseText);
	QString getPhaseText() const { return phaseText; }
	void setActive(bool _active);
	bool getActive() const { return active; }
	void triggerDoubleClickAction();
protected:
	void paintEvent(QPaintEvent *event);
	void mouseDoubleClickEvent(QMouseEvent *event);
};

class PhasesToolbar : public QFrame {
	Q_OBJECT
private:
	QList<PhaseButton *> buttonList;
public:
	PhasesToolbar(QWidget *parent = 0);
	void retranslateUi();
	int phaseCount() const { return buttonList.size(); }
public slots:
	void setActivePhase(int phase);
private slots:
	void phaseButtonClicked();
	void actNextTurn();
	void actUntapAll();
	void actDrawCard();
signals:
	void sendGameCommand(GameCommand *command, int playerId);
};

#endif
