#ifndef PINGPIXMAPGENERATOR_H
#define PINGPIXMAPGENERATOR_H

#include <QPixmap>
#include <QMap>

class PingPixmapGenerator {
private:
	static QMap<int, QPixmap> pmCache;
public:
	static QPixmap generatePixmap(int size, int value, int max);
};

#endif
