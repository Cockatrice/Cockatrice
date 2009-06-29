#include "abstractgraphicsitem.h"
#include <QPainter>

void AbstractGraphicsItem::paintNumberEllipse(int number, QPainter *painter)
{
	painter->save();

	QString numStr = QString::number(number);
	QFont font("Times");
	font.setPixelSize(32);
	font.setWeight(QFont::Bold);
	
	QFontMetrics fm(font);
	double w = fm.width(numStr) * 1.5;
	double h = fm.height() * 1.5;
	if (w < h)
		w = h;

	painter->setPen(QColor(255, 255, 255, 0));
	QRadialGradient grad(QPointF(0.5, 0.5), 0.5);
	grad.setCoordinateMode(QGradient::ObjectBoundingMode);
	grad.setColorAt(0, QColor(255, 255, 255, 200));
	grad.setColorAt(0.7, QColor(255, 255, 255, 200));
	grad.setColorAt(1, QColor(255, 255, 255, 0));
	painter->setBrush(QBrush(grad));
	painter->drawEllipse(QRectF((boundingRect().width() - w) / 2.0, (boundingRect().height() - h) / 2.0, w, h));

	painter->setPen(Qt::black);
	painter->setFont(font);
	painter->drawText(boundingRect(), Qt::AlignCenter, numStr);

	painter->restore();
}
