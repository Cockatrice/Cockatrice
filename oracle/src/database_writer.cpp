#include "database_writer.h"

#include <QCoreApplication>
#include <QIODevice>
#include <QSaveFile>
#include <QXmlStreamReader>

namespace OracleDatabase
{
QString validateXml(const QByteArray &data)
{
    QXmlStreamReader xml(data);
    if (!xml.readNextStartElement() || xml.name() != QLatin1String("cockatrice_carddatabase")) {
        return QCoreApplication::translate("OracleDatabase", "The download is not a Cockatrice card database.");
    }
    const auto version = xml.attributes().value("version");
    if (version != QLatin1String("3") && version != QLatin1String("4")) {
        return QCoreApplication::translate("OracleDatabase", "The downloaded card database version is not supported.");
    }
    while (!xml.atEnd()) {
        xml.readNext();
    }
    if (xml.hasError()) {
        return QCoreApplication::translate("OracleDatabase",
                                           "The downloaded card database is incomplete or invalid: %1")
            .arg(xml.errorString());
    }
    return {};
}

bool saveXml(const QString &fileName, const QByteArray &data, QString *error)
{
    const QString validationError = validateXml(data);
    if (!validationError.isEmpty()) {
        if (error) {
            *error = validationError;
        }
        return false;
    }
    QSaveFile file(fileName);
    if (!file.open(QIODevice::WriteOnly) || file.write(data) != data.size() || !file.commit()) {
        if (error) {
            *error = file.errorString();
        }
        return false;
    }
    if (error) {
        error->clear();
    }
    return true;
}
} // namespace OracleDatabase
