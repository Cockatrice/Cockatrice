#ifndef COCKATRICE_INTENT_OPEN_LOCAL_DECK_H
#define COCKATRICE_INTENT_OPEN_LOCAL_DECK_H

#include "intent.h"

#include <QString>

class TabSupervisor;

class IntentOpenLocalDeck : public Intent
{
    Q_OBJECT

public:
    IntentOpenLocalDeck(TabSupervisor *_tabSupervisor, const QString &_file);

protected:
    bool checkPrecondition() const override;
    void onPreconditionSatisfied() override;
    void onPreconditionNotSatisfied() override;

private:
    TabSupervisor *tabSupervisor;
    QString file;
};

#endif // COCKATRICE_INTENT_OPEN_LOCAL_DECK_H
