#include "phasebutton.h"
#include <QPainter>
#include <QPen>
//
PhaseButton::PhaseButton(QIcon icon, QString name  ) 
	: QPushButton(icon,name)
{
	
}

//PhaseButton::PhaseButton(  ) 
	//: QPushButton()
//{
	//
//}

void PhaseButton::update()
{
	QPushButton::update();
}

void PhaseButton::paintEvent(QPaintEvent *event)
{
	QPushButton::paintEvent(event);
	if(isChecked())
	{
		QPainter painter(this);
		int height = size().height();
		int width  = size().width();
		QPen pen;
		pen.setWidth(3);
		pen.setColor(QColor::fromRgb(180,0,0));
		painter.setPen(pen);
		painter.drawRect(3,3, width-7, height-7);
	}
		
	
}
//
