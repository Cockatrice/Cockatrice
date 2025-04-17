#include "edhrec_api_response_archidekt_links.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

void EdhrecApiResponseArchidektLink::fromJson(const QJsonObject &json)
{
    c = json.value("c").toString();
    f = json.value("f").toInt(0);
    q = json.value("q").toInt(0);
    u = json.value("u").toString();
}

void EdhrecApiResponseArchidektLink::debugPrint() const
{
    qDebug() << "  C:" << c;
    qDebug() << "  F:" << f;
    qDebug() << "  Q:" << q;
    qDebug() << "  U:" << u;
}

void EdhrecCommanderApiResponseArchidektLinks::fromJson(const QJsonArray &json)
{
    entries.clear();
    for (const QJsonValue &value : json) {
        if (value.isObject()) {
            QJsonObject entryJson = value.toObject();
            EdhrecApiResponseArchidektLink entry;
            entry.fromJson(entryJson);
            entries.append(entry);
        }
    }
}

void EdhrecCommanderApiResponseArchidektLinks::debugPrint() const
{
    qDebug() << "Archidekt Entries:";
    for (const auto &entry : entries) {
        entry.debugPrint();
    }
}
