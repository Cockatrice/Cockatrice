/**
 * @file move_menu.h
 * @ingroup GameMenusZones
 * @brief TODO: Document this.
 */

#ifndef COCKATRICE_MOVE_MENU_H
#define COCKATRICE_MOVE_MENU_H
#include <QMenu>

class Player;
class MoveMenu : public QMenu
{
    Q_OBJECT

public:
    explicit MoveMenu(Player *player);
    void setShortcutsActive();
    void retranslateUi();

    QAction *aMoveToTopLibrary = nullptr;
    QAction *aMoveToXfromTopOfLibrary = nullptr;
    QAction *aMoveToBottomLibrary = nullptr;

    QAction *aMoveToHand = nullptr;
    QAction *aMoveToGraveyard = nullptr;
    QAction *aMoveToExile = nullptr;
};

#endif // COCKATRICE_MOVE_MENU_H
