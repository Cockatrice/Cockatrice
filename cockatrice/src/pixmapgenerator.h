#ifndef PIXMAPGENERATOR_H
#define PIXMAPGENERATOR_H

#include <QPixmap>
#include <QMap>

#include "user_level.h"

class PhasePixmapGenerator {
private:
    static QMap<QString, QPixmap> pmCache;
public:
    static QPixmap generatePixmap(int size, QString name);
    static void clear() { pmCache.clear(); }
};

class CounterPixmapGenerator {
private:
    static QMap<QString, QPixmap> pmCache;
public:
    static QPixmap generatePixmap(int size, QString name, bool highlight);
    static void clear() { pmCache.clear(); }
};

class PingPixmapGenerator {
private:
    static QMap<int, QPixmap> pmCache;
public:
    static QPixmap generatePixmap(int size, int value, int max);
    static void clear() { pmCache.clear(); }
};

class GenderPixmapGenerator {
private:
    static QMap<int, QPixmap> pmCache;
public:
    static QPixmap generatePixmap(int height);
    static void clear() { pmCache.clear(); }
};

class CountryPixmapGenerator {
private:
    static QMap<QString, QPixmap> pmCache;
public:
    static QPixmap generatePixmap(int height, const QString &countryCode);
    static void clear() { pmCache.clear(); }
};

class UserLevelPixmapGenerator {
private:
    static QMap<QString, QPixmap> pmCache;
public:
    static QPixmap generatePixmap(int height, UserLevelFlags userLevel, bool isBuddy, QString privLevel = "NONE");
    static void clear() { pmCache.clear(); }
};

class LockPixmapGenerator {
private:
    static QMap<int, QPixmap> pmCache;
public:
    static QPixmap generatePixmap(int height);
    static void clear() { pmCache.clear(); }
};

#endif
