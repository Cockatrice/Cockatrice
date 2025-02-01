#include "pixel_map_generator.h"

#include "pb/serverinfo_user.pb.h"

#include <QApplication>
#include <QDebug>
#include <QDomDocument>
#include <QFile>
#include <QPainter>
#include <QPalette>
#include <QSvgRenderer>
#include <QtGui/qicon.h>

QMap<QString, QPixmap> PhasePixmapGenerator::pmCache;

QPixmap PhasePixmapGenerator::generatePixmap(int height, QString name)
{
    QString key = name + QString::number(height);
    if (pmCache.contains(key))
        return pmCache.value(key);

    QPixmap pixmap =
        QPixmap("theme:phases/" + name).scaled(height, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);

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

    QPixmap pixmap =
        QPixmap("theme:counters/" + name).scaled(height, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    if (pixmap.isNull()) {
        name = "general";
        if (highlight)
            name.append("_highlight");
        pixmap =
            QPixmap("theme:counters/" + name).scaled(height, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
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
        color.setHsv(120 * (1.0 - ((double)value / max)), 255, 255);

    QRadialGradient g(QPointF((double)pixmap.width() / 2, (double)pixmap.height() / 2),
                      qMin(pixmap.width(), pixmap.height()) / 2.0);
    g.setColorAt(0, color);
    g.setColorAt(1, Qt::transparent);
    painter.fillRect(0, 0, pixmap.width(), pixmap.height(), QBrush(g));

    pmCache.insert(key, pixmap);

    return pixmap;
}

QMap<int, QPixmap> PingPixmapGenerator::pmCache;

QPixmap CountryPixmapGenerator::generatePixmap(int height, const QString &countryCode)
{
    if (countryCode.size() != 2)
        return QPixmap();
    QString key = countryCode + QString::number(height);
    if (pmCache.contains(key))
        return pmCache.value(key);

    int width = height * 2;
    QPixmap pixmap = QPixmap("theme:countries/" + countryCode.toLower())
                         .scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    QPainter painter(&pixmap);
    painter.setPen(Qt::black);
    painter.drawRect(0, 0, pixmap.width() - 1, pixmap.height() - 1);

    pmCache.insert(key, pixmap);
    return pixmap;
}

QMap<QString, QPixmap> CountryPixmapGenerator::pmCache;

void SetAttrRecur(QDomElement &elem, QString tagName, QString attrName, QString attrValue)
{
    // if it has the tagname then overwritte desired attribute
    if (elem.tagName().compare(tagName) == 0) {
        elem.setAttribute(attrName, attrValue);
    }
    // loop all children
    for (int i = 0; i < elem.childNodes().count(); i++) {
        if (!elem.childNodes().at(i).isElement()) {
            continue;
        }
        QDomElement docElem = elem.childNodes().at(i).toElement(); //<-- make const "variable"
        SetAttrRecur(docElem, tagName, attrName, attrValue);
    }
}

QIcon changeSVGColor(const QString &iconPath, const QString &color)
{
    // open svg resource load contents to qbytearray
    QFile file(iconPath);
    if (!file.open(QIODevice::ReadOnly))
        return {};
    QByteArray baData = file.readAll();
    // load svg contents to xml document and edit contents
    QDomDocument doc;
    doc.setContent(baData);
    // recurivelly change color
    QDomElement docElem = doc.documentElement(); //<-- make const "variable"
    qDebug() << docElem.text();
    SetAttrRecur(docElem, "path", "fill", color);
    // create svg renderer with edited contents
    QSvgRenderer svgRenderer(doc.toByteArray());
    // create pixmap target (could be a QImage)
    QPixmap pix(svgRenderer.defaultSize());
    pix.fill(Qt::transparent);
    // create painter to act over pixmap
    QPainter pixPainter(&pix);
    // use renderer to render over painter which paints on pixmap
    svgRenderer.render(&pixPainter);
    QIcon myicon(pix);
    return myicon;
}

QPixmap UserLevelPixmapGenerator::generatePixmap(int height, UserLevelFlags userLevel, bool isBuddy, QString privLevel)
{
    return generateIcon(height, userLevel, isBuddy, privLevel).pixmap(QSize());
}

QIcon UserLevelPixmapGenerator::generateIcon(int height, UserLevelFlags userLevel, bool isBuddy, QString privLevel)
{

    QString key = QString::number(height * 10000) + ":" + (short)userLevel + ":" + (short)isBuddy + ":" + privLevel;
    /*if (pmCache.contains(key))
        return pmCache.value(key);
        */

    QString levelString;
    if (userLevel.testFlag(ServerInfo_User::IsAdmin)) {
        levelString = "admin";
        if (privLevel.toLower() == "vip")
            levelString.append("_" + privLevel.toLower());
    } else if (userLevel.testFlag(ServerInfo_User::IsModerator)) {
        levelString = "moderator";
        if (privLevel.toLower() == "vip")
            levelString.append("_" + privLevel.toLower());
    } else if (userLevel.testFlag(ServerInfo_User::IsRegistered)) {
        levelString = "registered";
        if (privLevel.toLower() != "none")
            levelString.append("_" + privLevel.toLower());
    } else
        levelString = "normal";

    if (isBuddy)
        levelString.append("_buddy");

    QString color = "#FF3399";

    QIcon icon =
        changeSVGColor("/Users/Ricky/Documents/GitHub/Cockatrice/cockatrice/resources/userlevels/base.svg", color);

    return icon;
}

QMap<QString, QPixmap> UserLevelPixmapGenerator::pmCache;

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

const QPixmap loadColorAdjustedPixmap(QString name)
{
    if (qApp->palette().windowText().color().lightness() > 200) {
        QImage img(name);
        img.invertPixels();
        QPixmap result;
        result.convertFromImage(img);
        return result;
    } else {
        return QPixmap(name);
    }
}
