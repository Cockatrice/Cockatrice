#ifndef COLOR_H
#define COLOR_H

#ifdef QT_GUI_LIB
#include <QColor>
#endif

class Color {
private:
	int value;
public:
	Color(int _value = 0) : value(_value) { }
	Color(int r, int g, int b) : value(r * 65536 + g * 256 + b) { }
	int getValue() const { return value; }
#ifdef QT_GUI_LIB
	Color(const QColor &_color)
	{
		value = _color.red() * 65536 + _color.green() * 256 + _color.blue();
	}
	QColor getQColor() const
	{
		return QColor(value / 65536, (value % 65536) / 256, value % 256);
	}
#endif
};

#endif