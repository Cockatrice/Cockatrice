#ifndef CAPITALIZECOUNTERNAME_H
#define CAPITALIZECOUNTERNAME_H

#include <QString>
#include <QtCore>

class CapitalizeCounterName
{
    Q_DECLARE_TR_FUNCTIONS(CapitalizedCounterName)

public:
    static QString getDisplayName(QString _name)
    {
        if (_name == "life") {
            return QString("Life");
        }
        if (_name == "w") {
            return QString("W");
        }
        if (_name == "u") {
            return QString("U");
        }
        if (_name == "b") {
            return QString("B");
        }
        if (_name == "r") {
            return QString("R");
        }
        if (_name == "g") {
            return QString("G");
        }
        if (_name == "x") {
            return QString("C");
        }
        if (_name == "storm") {
            return QString("Storm");
        }
        return QString("Unknown");
    }
};

#endif