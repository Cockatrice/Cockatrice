#include "abstractgraphicsitem.h"
#include <QPainter>

void AbstractGraphicsItem::paintNumberEllipse(int number, int fontSize, const QColor &color, int position, QPainter *painter)
{
	painter->save();

	QString numStr = QString::number(number);
	QFont font("Serif");
	font.setPixelSize(fontSize);
	font.setWeight(QFont::Bold);
	
	QFontMetrics fm(font);
	double w = fm.width(numStr) * 1.3;
	double h = fm.height() * 1.3;
	if (w < h)
		w = h;

	painter->setPen(QColor(255, 255, 255, 0));
	QRadialGradient grad(QPointF(0.5, 0.5), 0.5);
	grad.setCoordinateMode(QGradient::ObjectBoundingMode);
	QColor color1(color), color2(color);
	color1.setAlpha(255);
	color2.setAlpha(0);
	grad.setColorAt(0, color1);
	grad.setColorAt(0.8, color1);
	grad.setColorAt(1, color2);
	painter->setBrush(QBrush(grad));
	
	QRectF textRect;
	if (position == -1)
		textRect = QRectF((boundingRect().width() - w) / 2.0, (boundingRect().height() - h) / 2.0, w, h);
	else {
		qreal offset = boundingRect().width() / 20.0;
		textRect = QRectF(offset, offset * (position + 1) + h * position, w, h);
	}
	
	painter->drawEllipse(textRect);

	painter->setPen(Qt::black);
	painter->setFont(font);
	painter->drawText(textRect, Qt::AlignCenter, numStr);

	painter->restore();
}
