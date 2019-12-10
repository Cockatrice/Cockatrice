#ifndef PHASE_H
#define PHASE_H

#include <QtCore/QString>

struct Phase
{
    QString name, color, soundFileName;
};

struct Phases
{
    const static int phaseTypesCount = 12;
    const static Phase phases[phaseTypesCount];

    static Phase getPhase(int);
};

#endif
