/**
 * @file pixel_map_generator.h
 * @ingroup UI
 * @brief TODO: Document this.
 */

#ifndef PIXMAPGENERATOR_H
#define PIXMAPGENERATOR_H

#include <QIcon>
#include <QLoggingCategory>
#include <QMap>
#include <QPixmap>
#include <libcockatrice/network/server/remote/user_level.h>

inline Q_LOGGING_CATEGORY(PixelMapGeneratorLog, "pixel_map_generator");

class PhasePixmapGenerator
{
private:
    static QMap<QString, QPixmap> pmCache;

public:
    static QPixmap generatePixmap(int size, QString name);
    static void clear()
    {
        pmCache.clear();
    }
};

class CounterPixmapGenerator
{
private:
    static QMap<QString, QPixmap> pmCache;

public:
    static QPixmap generatePixmap(int size, QString name, bool highlight);
    static void clear()
    {
        pmCache.clear();
    }
};

class PingPixmapGenerator
{
private:
    static QMap<int, QPixmap> pmCache;

public:
    static QPixmap generatePixmap(int size, int value, int max);
    static void clear()
    {
        pmCache.clear();
    }
};

class CountryPixmapGenerator
{
private:
    static QMap<QString, QPixmap> pmCache;

public:
    static QPixmap generatePixmap(int height, const QString &countryCode);
    static void clear()
    {
        pmCache.clear();
    }
};

class UserLevelPixmapGenerator
{
private:
    static QMap<QString, QIcon> iconCache;

    static QIcon generateIconDefault(int height, UserLevelFlags userLevel, bool isBuddy, const QString &privLevel);
    static QIcon generateIconWithColorOverride(int height,
                                               bool isBuddy,
                                               const UserLevelFlags &userLevelFlags,
                                               const QString &privLevel,
                                               const std::optional<QString> &colorLeft,
                                               const std::optional<QString> &colorRight);

public:
    static QPixmap generatePixmap(int height,
                                  UserLevelFlags userLevel,
                                  ServerInfo_User::PawnColorsOverride pawnColors,
                                  bool isBuddy,
                                  const QString &privLevel);

    static QIcon generateIcon(int minHeight,
                              UserLevelFlags userLevel,
                              ServerInfo_User::PawnColorsOverride pawnColors,
                              bool isBuddy,
                              const QString &privLevel);
    static void clear()
    {
        iconCache.clear();
    }
};

class LockPixmapGenerator
{
private:
    static QMap<int, QPixmap> pmCache;

public:
    static QPixmap generatePixmap(int height);
    static void clear()
    {
        pmCache.clear();
    }
};

class DropdownIconPixmapGenerator
{
private:
    static QMap<QString, QPixmap> pmCache;

public:
    static QPixmap generatePixmap(int height, bool expanded);
    static void clear()
    {
        pmCache.clear();
    }
};

QPixmap loadColorAdjustedPixmap(const QString &name);

#endif
