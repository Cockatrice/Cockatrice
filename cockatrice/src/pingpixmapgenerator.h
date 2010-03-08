#ifndef PINGPIXMAPGENERATOR_H
#define PINGPIXMAPGENERATOR_H

#include <QPixmap>
#include <QMap>

class PingPixmapGenerator {
private:
	QMap<int, QPixmap> pmCache;
public:
	QPixmap generatePixmap(int size, int value, int max);
};

extern PingPixmapGenerator *pingPixmapGenerator;

#endif
