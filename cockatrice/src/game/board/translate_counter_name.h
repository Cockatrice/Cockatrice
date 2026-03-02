/**
 * @file translate_counter_name.h
 * @ingroup GameGraphicsPlayers
 * @brief TODO: Document this.
 */

#ifndef TRANSLATECOUNTERNAME_H
#define TRANSLATECOUNTERNAME_H

#include <QString>
#include <QtCore>

class TranslateCounterName
{
    Q_DECLARE_TR_FUNCTIONS(TranslateCounterName)

    static const QMap<QString, QString> translated;

public:
    static QString getDisplayName(const QString &name)
    {
        if (translated.contains(name)) {
            return tr(translated[name].toLatin1());
        } else {
            return name;
        }
    }
};

#endif // TRANSLATECOUNTERNAME_H
