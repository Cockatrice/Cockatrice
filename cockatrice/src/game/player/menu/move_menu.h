/**
 * @file move_menu.h
 * @ingroup GameMenusZones
 * @brief Submenu for moving cards between zones.
 *
 * MoveMenu provides actions for moving selected cards to various game zones
 * (library, hand, graveyard, exile, command zone, partner zone). The menu
 * dynamically shows/hides command zone actions based on whether the server
 * has command zones enabled for the current game format.
 */

#ifndef COCKATRICE_MOVE_MENU_H
#define COCKATRICE_MOVE_MENU_H
#include <QMenu>

class Player;

/**
 * @class MoveMenu
 * @brief A context submenu containing actions to move cards between zones.
 *
 * This menu is displayed as a submenu of the card context menu and provides
 * quick access to common card movement operations. Actions include moving
 * cards to the library (top/bottom/specific position), hand, graveyard,
 * exile, and command zones.
 *
 * The command zone actions (aMoveToCommandZone, aMoveToPartnerZone) are
 * automatically hidden when the current game doesn't support command zones
 * (i.e., non-Commander format games).
 *
 * @see PlayerActions
 * @see CardMenu
 */
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
    QAction *aMoveToCommandZone = nullptr;
    QAction *aMoveToPartnerZone = nullptr;
    QAction *aMoveToCompanionZone = nullptr;
    QAction *aMoveToBackgroundZone = nullptr;
    QAction *aMoveToTable = nullptr;

private:
    Player *player;
    QMenu *commandSubmenu = nullptr;
};

#endif // COCKATRICE_MOVE_MENU_H
