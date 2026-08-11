/**
 * @file command_zone_menu.h
 * @ingroup GameGraphicsPlayers
 * @brief Submenu for command zone actions (cast counts, minimize/restore).
 */

#ifndef COCKATRICE_COMMAND_ZONE_MENU_H
#define COCKATRICE_COMMAND_ZONE_MENU_H

#include "abstract_player_component.h"

#include <QMenu>
#include <array>
#include <libcockatrice/utility/counter_ids.h>

class PlayerGraphicsItem;

/**
 * @class CommandZoneMenu
 * @brief Player submenu for managing cast counts and command zone display.
 *
 * Provides actions to increment/decrement each cast count, toggle cast counts
 * on/off, and minimize/restore the command zone display.
 *
 * @see PlayerMenu
 * @see CommandZone
 */
class CommandZoneMenu : public QMenu, public AbstractPlayerComponent
{
    Q_OBJECT

public:
    explicit CommandZoneMenu(PlayerGraphicsItem *player, QMenu *playerMenu);
    void retranslateUi() override;
    void setShortcutsActive() override;
    void setShortcutsInactive() override;

private:
    static constexpr int CastCountCount = CastCountIds::Count;

    std::array<QAction *, CastCountCount> aIncrease{};
    std::array<QAction *, CastCountCount> aDecrease{};
    std::array<QAction *, CastCountCount> aToggle{};
    QAction *aToggleMinimized = nullptr;

public slots:
    void updateCastCountActionStates();

private slots:
    void actToggleMinimized();

private:
    PlayerGraphicsItem *player;

    QString inc1ShortcutKey;
    QString dec1ShortcutKey;
    QString inc2ShortcutKey;
    QString dec2ShortcutKey;
};

#endif // COCKATRICE_COMMAND_ZONE_MENU_H
