#ifndef ARCHIDEKTENTRY_H
#define ARCHIDEKTENTRY_H

#include <QDebug>
#include <QJsonObject>
#include <QString>
#include <QVector>

/**
 * @class EdhrecApiResponseArchidektLink
 * @ingroup ApiResponses
 * @brief Represents a single Archidekt entry
 */
class EdhrecApiResponseArchidektLink
{
public:
    QString c;
    int f = 0;
    int q = 0;
    QString u;

    void fromJson(const QJsonObject &json);
    void debugPrint() const;
};

/**
 * @class EdhrecCommanderApiResponseArchidektLinks
 * @ingroup ApiResponses
 * @brief Represents the Archidekt section as a list of entries
 */
class EdhrecCommanderApiResponseArchidektLinks
{
public:
    QVector<EdhrecApiResponseArchidektLink> entries;

    void fromJson(const QJsonArray &json);
    void debugPrint() const;
};

#endif // ARCHIDEKTENTRY_H
