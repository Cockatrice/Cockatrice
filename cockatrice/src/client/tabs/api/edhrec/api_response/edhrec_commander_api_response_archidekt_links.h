#ifndef ARCHIDEKTENTRY_H
#define ARCHIDEKTENTRY_H

#include <QDebug>
#include <QJsonObject>
#include <QString>
#include <QVector>

// Represents a single Archidekt entry
class EdhrecCommanderApiResponseArchidektLink
{
public:
    QString c;
    int f = 0;
    int q = 0;
    QString u;

    void fromJson(const QJsonObject &json);
    void debugPrint() const;
};

// Represents the Archidekt section as a list of entries
class EdhrecCommanderApiResponseArchidektLinks
{
public:
    QVector<EdhrecCommanderApiResponseArchidektLink> entries;

    void fromJson(const QJsonArray &json);
    void debugPrint() const;
};

#endif // ARCHIDEKTENTRY_H
