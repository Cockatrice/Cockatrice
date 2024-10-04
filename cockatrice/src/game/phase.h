#ifndef PHASE_H
#define PHASE_H

#include <QString>
#include <QtCore>

class Phase
{
    Q_DECLARE_TR_FUNCTIONS(Phase)

public:
    QString name, color, soundFileName;
    Phase(const char *_name, QString _color, QString _soundFileName);
};

struct Phases
{
    const static int phaseTypesCount = 11;
    const static Phase unknownPhase;
    const static Phase phases[phaseTypesCount];

    static Phase getPhase(int);
};

#endif // PHASE_H
