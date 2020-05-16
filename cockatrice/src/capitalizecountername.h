#ifndef CAPITALIZECOUNTERNAME_H
#define CAPITALIZECOUNTERNAME_H

#include <QString>
#include <QtCore>

class CapitalizeCounterName
{
    Q_DECLARE_TR_FUNCTIONS(CapitalizeCounterName)

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

#endif // CAPITALIZECOUNTERNAME_H
