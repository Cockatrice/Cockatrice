#ifndef COCKATRICE_TALLY_MENU_H
#define COCKATRICE_TALLY_MENU_H

#include "../../../interface/widgets/menus/tearoff_menu.h"
#include "../../tally/tally.h"
#include "abstract_player_component.h"

#include <QMenu>

class TallyMenu : public TearOffMenu, public AbstractPlayerComponent
{
    Q_OBJECT

public:
    TallyMenu();

    void setShortcutsActive() override;
    void setShortcutsInactive() override;
    void retranslateUi() override;

private:
    QActionGroup *actionGroup = nullptr;

    QAction *aTallyNone = nullptr;
    QAction *aTallySubtypes = nullptr;
    QAction *aTallyTotalPower = nullptr;

    QAction *createTallyAction(TallyType tallyType);
};

#endif // COCKATRICE_TALLY_MENU_H
