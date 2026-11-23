#ifndef ORACLE_DATABASE_WRITER_H
#define ORACLE_DATABASE_WRITER_H

#include <QByteArray>
#include <QString>

namespace OracleDatabase
{
// Validate the complete downloaded document without allocating a card database.
// Empty string means valid; otherwise returns a diagnostic suitable for the wizard.
QString validateXml(const QByteArray &data);
// Never truncate the installed database: validate, write a sibling temporary file,
// check the complete write and atomically commit it.
bool saveXml(const QString &fileName, const QByteArray &data, QString *error = nullptr);
} // namespace OracleDatabase

#endif
