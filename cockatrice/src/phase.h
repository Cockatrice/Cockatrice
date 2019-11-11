#ifndef COCKATRICE_PHASE_H
#define COCKATRICE_PHASE_H

#include <QtCore/QString>

struct Phase
{
    const static QString names[12];
    const static QString colors[12];
    const static QString soundFileNames[12];

    QString getName();
    QString getColor();
    QString getSoundFileName();

    int index;
};

#endif //COCKATRICE_PHASE_H
