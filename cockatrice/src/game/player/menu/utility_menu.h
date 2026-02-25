/**
 * @file utility_menu.h
 * @ingroup GameMenusPlayers
 * @brief TODO: Document this.
 */

#ifndef COCKATRICE_UTILITY_MENU_H
#define COCKATRICE_UTILITY_MENU_H

#include "abstract_zone_menu.h"

#include <QMenu>

class Player;
class UtilityMenu : public QMenu, public AbstractZoneMenu
{
    Q_OBJECT
public slots:
    void populatePredefinedTokensMenu();
    void retranslateUi() override;
    void setShortcutsActive() override;
    void setShortcutsInactive() override;

public:
    explicit UtilityMenu(Player *player, QMenu *playerMenu);

    [[nodiscard]] bool createAnotherTokenActionExists() const
    {
        return aCreateAnotherToken != nullptr;
    }

    void setAndEnableCreateAnotherTokenAction(QString text)
    {
        aCreateAnotherToken->setText(text);
        aCreateAnotherToken->setEnabled(true);
    }

    QStringList getPredefinedTokens() const
    {
        return predefinedTokens;
    }

private:
    Player *player;
    QStringList predefinedTokens;

    QMenu *createPredefinedTokenMenu;

    QAction *aIncrementAllCardCounters;
    QAction *aUntapAll, *aRollDie;
    QAction *aCreateToken, *aCreateAnotherToken;
};

#endif // COCKATRICE_UTILITY_MENU_H
