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

/**
 * Updates tags in the svg
 *
 * @param elem The svg
 * @param tagName tag with attribute to update
 * @param attrName attribute to be updated
 * @param idName id that the tag has to match
 * @param attrValue the value to update the attribute to
 */
void setAttrRecur(QDomElement &elem,
                  const QString &tagName,
                  const QString &attrName,
                  const QString &idName,
                  const QString &attrValue)
{
    if (elem.tagName().compare(tagName) == 0) {
        if (elem.attribute("id").compare(idName) == 0) {
            elem.setAttribute(attrName, attrValue);
        }
    }

    for (int i = 0; i < elem.childNodes().count(); i++) {
        if (!elem.childNodes().at(i).isElement()) {
            continue;
        }
        auto docElem = elem.childNodes().at(i).toElement();
        setAttrRecur(docElem, tagName, attrName, idName, attrValue);
    }
}

/**
 * Returns an icon of the svg that has its color filled in
 */
QIcon changeSVGColor(const QString &iconPath, const QString &colorLeft, const std::optional<QString> &colorRight)
{
    QFile file(iconPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Unable to open" << iconPath;
        return {};
    }

    const auto &baData = file.readAll();
    QDomDocument doc;
    doc.setContent(baData);

    auto docElem = doc.documentElement();

    SetAttrRecur(docElem, "path", "fill", "left", colorLeft);
    if (colorRight.has_value()) {
        SetAttrRecur(docElem, "path", "fill", "right", colorRight.value());
    }

    QSvgRenderer svgRenderer(doc.toByteArray());

    QPixmap pix(svgRenderer.defaultSize());
    pix.fill(Qt::transparent);

    QPainter pixPainter(&pix);
    svgRenderer.render(&pixPainter);
    QIcon myicon(pix);

    return myicon;
}

QPixmap UserLevelPixmapGenerator::generatePixmap(int height,
                                                 UserLevelFlags userLevel,
                                                 ServerInfo_User::PawnColorsOverride pawnColorsOverride,
                                                 bool isBuddy,
                                                 QString privLevel)
{
    return generateIcon(height, userLevel, pawnColorsOverride, isBuddy, privLevel).pixmap(height, height);
}

QIcon UserLevelPixmapGenerator::generateIcon(int height,
                                             UserLevelFlags userLevel,
                                             ServerInfo_User::PawnColorsOverride pawnColorsOverride,
                                             bool isBuddy,
                                             QString privLevel)
{
    std::optional<QString> colorLeft = std::nullopt;
    if (pawnColorsOverride.has_left_side()) {
        colorLeft = QString::fromStdString(pawnColorsOverride.left_side());
    }

    std::optional<QString> colorRight = std::nullopt;
    if (pawnColorsOverride.has_right_side()) {
        colorRight = QString::fromStdString(pawnColorsOverride.right_side());
    }

    // Has Color Override
    if (colorLeft.has_value()) {
        QString key = QString::number(height * 10000) + ":" + colorLeft.value_or("") + ":" + colorRight.value_or("");
        if (iconCache.contains(key)) {
            return iconCache.value(key);
        }

        QIcon icon;
        if (colorRight.has_value()) {
            icon = changeSVGColor("theme:usericons/pawn_double.svg", colorLeft.value(), colorRight);
        } else {
            icon = changeSVGColor("theme:usericons/pawn_single.svg", colorLeft.value(), colorRight);
        }

        iconCache.insert(key, icon);
        return icon;
    }

    // Has No Color Override
    QString key = QString::number(height * 10000) + ":" + (short)userLevel + ":" + (short)isBuddy + ":" + privLevel;
    if (iconCache.contains(key)) {
        return iconCache.value(key);
    }

    QString levelString;
    if (userLevel.testFlag(ServerInfo_User::IsAdmin)) {
        levelString = "admin";
        if (privLevel.toLower() == "vip") {
            levelString.append("_" + privLevel.toLower());
        }
    } else if (userLevel.testFlag(ServerInfo_User::IsModerator)) {
        levelString = "moderator";
        if (privLevel.toLower() == "vip") {
            levelString.append("_" + privLevel.toLower());
        }
    } else if (userLevel.testFlag(ServerInfo_User::IsRegistered)) {
        levelString = "registered";
        if (privLevel.toLower() != "none") {
            levelString.append("_" + privLevel.toLower());
        }
    } else {
        levelString = "normal";
    }

    if (isBuddy) {
        levelString.append("_buddy");
    }

    auto pixmap = QPixmap("theme:userlevels/" + levelString)
                      .scaled(height, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QIcon icon(pixmap);
    iconCache.insert(key, icon);
    return icon;
}

QMap<QString, QIcon> UserLevelPixmapGenerator::iconCache;

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
