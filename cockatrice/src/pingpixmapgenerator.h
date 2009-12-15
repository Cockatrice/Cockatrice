#ifndef PINGPIXMAPGENERATOR_H
#define PINGPIXMAPGENERATOR_H

#include <QPixmap>

class PingPixmapGenerator {
public:
	static QPixmap generatePixmap(int size, int value, int max);
};

#endif
