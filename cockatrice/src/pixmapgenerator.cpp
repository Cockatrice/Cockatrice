#include "pixmapgenerator.h"
#include "pb/serverinfo_user.pb.h"
#include <QPainter>
#include <cmath>
#ifdef _WIN32
#include "round.h"
#endif /* _WIN32 */
#include <QDebug>

QMap<QString, QPixmap> PhasePixmapGenerator::pmCache;

QPixmap PhasePixmapGenerator::generatePixmap(int height, QString name)
{
    QString key = name + QString::number(height);
    if (pmCache.contains(key))
        return pmCache.value(key);

    QPixmap pixmap = QPixmap("theme:phases/" + name).scaled(height, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    pmCache.insert(key, pixmap);
    return pixmap;
}

QMap<QString, QPixmap> CounterPixmapGenerator::pmCache;

QPixmap CounterPixmapGenerator::generatePixmap(int height, QString name, bool highlight)
{
    if (highlight)
        name.append("_highlight");
    QString key = name + QString::number(height);
    if (pmCache.contains(key))
        return pmCache.value(key);

    QPixmap pixmap = QPixmap("theme:counters/" + name).scaled(height, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    if(pixmap.isNull())
    {
        name = "general";
        if (highlight)
            name.append("_highlight");
        pixmap = QPixmap("theme:counters/" + name).scaled(height, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    }

    pmCache.insert(key, pixmap);
    return pixmap;
}

QPixmap PingPixmapGenerator::generatePixmap(int size, int value, int max)
{
    int key = size * 1000000 + max * 1000 + value;
    if (pmCache.contains(key))
        return pmCache.value(key);

    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    QColor color;
    if ((max == -1) || (value == -1))
        color = Qt::black;
    else
        color.setHsv(120 * (1.0 - ((double) value / max)), 255, 255);

    QRadialGradient g(QPointF((double) pixmap.width() / 2, (double) pixmap.height() / 2), qMin(pixmap.width(), pixmap.height()) / 2.0);
    g.setColorAt(0, color);
    g.setColorAt(1, Qt::transparent);
    painter.fillRect(0, 0, pixmap.width(), pixmap.height(), QBrush(g));

    pmCache.insert(key, pixmap);

    return pixmap;
}

QMap<int, QPixmap> PingPixmapGenerator::pmCache;

QPixmap GenderPixmapGenerator::generatePixmap(int height)
{
     ServerInfo_User::Gender gender = ServerInfo_User::GenderUnknown;

    int key = gender * 100000 + height;
    if (pmCache.contains(key))
        return pmCache.value(key);

    QString genderStr;
    genderStr = "unknown";


    QPixmap pixmap = QPixmap("theme:genders/" + genderStr).scaled(height, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    pmCache.insert(key, pixmap);
    return pixmap;
}

QMap<int, QPixmap> GenderPixmapGenerator::pmCache;

QPixmap CountryPixmapGenerator::generatePixmap(int height, const QString &countryCode)
{
    if (countryCode.size() != 2)
        return QPixmap();
    QString key = countryCode + QString::number(height);
    if (pmCache.contains(key))
        return pmCache.value(key);

    int width = height * 2;
    QPixmap pixmap = QPixmap("theme:countries/" + countryCode.toLower()).scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    QPainter painter(&pixmap);
    painter.setPen(Qt::black);
    painter.drawRect(0, 0, pixmap.width() - 1, pixmap.height() - 1);

    pmCache.insert(key, pixmap);
    return pixmap;
}

QMap<QString, QPixmap> CountryPixmapGenerator::pmCache;

QPixmap UserLevelPixmapGenerator::generatePixmap(int height, UserLevelFlags userLevel, bool isBuddy)
{

    int key = height * 10000 + (int) userLevel + (int) isBuddy;
    if (pmCache.contains(key))
        return pmCache.value(key);

    QString levelString;
    if (userLevel.testFlag(ServerInfo_User::IsAdmin))
        levelString = "admin";
    else if (userLevel.testFlag(ServerInfo_User::IsModerator))
        levelString = "moderator";
    else if (userLevel.testFlag(ServerInfo_User::IsRegistered))
        levelString = "registered";
    else
        levelString = "normal";

    if (isBuddy)
        levelString.append("_buddy");

    QPixmap pixmap = QPixmap("theme:userlevels/" + levelString).scaled(height, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    pmCache.insert(key, pixmap);
    return pixmap;
}

QMap<int, QPixmap> UserLevelPixmapGenerator::pmCache;


QPixmap LockPixmapGenerator::generatePixmap(int height)
{

    int key = height;
    if (pmCache.contains(key))
        return pmCache.value(key);

    QPixmap pixmap = QPixmap("theme:icons/lock").scaled(height, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    pmCache.insert(key, pixmap);
    return pixmap;
}

QMap<int, QPixmap> LockPixmapGenerator::pmCache;
