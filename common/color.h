#ifndef COLOR_H
#define COLOR_H

#ifdef QT_GUI_LIB
#include <QColor>
#endif

#include "pb/color.pb.h"

#ifdef QT_GUI_LIB
inline QColor convertColorToQColor(const color &c)
{
	return QColor(c.r(), c.g(), c.b());
}

inline color convertQColorToColor(const QColor &c)
{
	color result;
	result.set_r(c.red());
	result.set_g(c.green());
	result.set_b(c.blue());
	return result;
}
#endif

inline color makeColor(int r, int g, int b)
{
	color result;
	result.set_r(r);
	result.set_g(g);
	result.set_b(b);
	return result;
}

#endif