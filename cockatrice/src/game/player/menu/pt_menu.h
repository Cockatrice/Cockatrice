/**
 * @file pt_menu.h
 * @ingroup GameMenusCards
 * @brief TODO: Document this.
 */

#ifndef COCKATRICE_PT_MENU_H
#define COCKATRICE_PT_MENU_H
#include <QMenu>

class Player;
class PtMenu : public QMenu
{

    Q_OBJECT

public:
    explicit PtMenu(Player *player);
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
