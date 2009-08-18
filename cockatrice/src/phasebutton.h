#ifndef PHASEBUTTON_H
#define PHASEBUTTON_H
//
#include <QPushButton>
//
class PhaseButton : public QPushButton
{
Q_OBJECT
public:
	PhaseButton();
	PhaseButton(QIcon, QString);
	void update();
	void paintEvent(QPaintEvent *event);
	
};
#endif
