/**
 * @file move_menu.h
 * @ingroup GameMenusZones
 */
//! \todo Document this file.

#ifndef COCKATRICE_MOVE_MENU_H
#define COCKATRICE_MOVE_MENU_H
#include <QMenu>

class PlayerLogic;
class MoveMenu : public QMenu
{
    Q_OBJECT

public:
    explicit MoveMenu(PlayerLogic *player);
    void setShortcutsActive();
    void retranslateUi();

    QAction *aMoveToTopLibrary = nullptr;
    QAction *aMoveToXfromTopOfLibrary = nullptr;
    QAction *aMoveToBottomLibrary = nullptr;

    QAction *aMoveToHand = nullptr;
    QAction *aMoveToTable = nullptr;
    QAction *aMoveToGraveyard = nullptr;
    QAction *aMoveToExile = nullptr;
};

#endif // COCKATRICE_MOVE_MENU_H
