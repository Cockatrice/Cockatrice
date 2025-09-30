/**
 * @file phase.h
 * @ingroup GameLogic
 * @brief TODO: Document this.
 */

#ifndef PHASE_H
#define PHASE_H

#include <QApplication>
#include <QString>

class Phase
{
    Q_DECLARE_TR_FUNCTIONS(Phase)

    QString name;

public:
    QString color, soundFileName;
    Phase(const QString &_name, const QString &_color, const QString &_soundFileName);

    QString getName() const;
};

struct Phases
{
    const static int phaseTypesCount = 11;
    const static Phase unknownPhase;
    const static Phase phases[phaseTypesCount];

    static Phase getPhase(int);
};

#endif // PHASE_H
