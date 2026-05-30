/**
 * @file pt_menu.h
 * @ingroup GameMenusCards
 */
//! \todo Document this file.

#ifndef COCKATRICE_PT_MENU_H
#define COCKATRICE_PT_MENU_H
#include <QMenu>

class PlayerLogic;
class PtMenu : public QMenu
{

    Q_OBJECT

public:
    explicit PtMenu(PlayerLogic *player);
    void retranslateUi();
    void setShortcutsActive();

    QAction *aIncP = nullptr;
    QAction *aDecP = nullptr;
    QAction *aFlowP = nullptr;
    QAction *aIncT = nullptr;
    QAction *aDecT = nullptr;
    QAction *aFlowT = nullptr;

    QAction *aIncPT = nullptr;
    QAction *aDecPT = nullptr;
    QAction *aSetPT = nullptr;
    QAction *aResetPT = nullptr;
};

#endif // COCKATRICE_PT_MENU_H
