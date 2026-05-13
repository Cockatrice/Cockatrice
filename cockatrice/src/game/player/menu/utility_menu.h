/**
 * @file utility_menu.h
 * @ingroup GameMenusPlayers
 */
//! \todo Document this file.

#ifndef COCKATRICE_UTILITY_MENU_H
#define COCKATRICE_UTILITY_MENU_H

#include "abstract_player_component.h"

#include <QMenu>

class PlayerLogic;
class UtilityMenu : public QMenu, public AbstractPlayerComponent
{
    Q_OBJECT
public slots:
    void populatePredefinedTokensMenu();
    void retranslateUi() override;
    void setShortcutsActive() override;
    void setShortcutsInactive() override;

public:
    explicit UtilityMenu(PlayerLogic *player, QMenu *playerMenu);

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
    PlayerLogic *player;
    QStringList predefinedTokens;

    QMenu *createPredefinedTokenMenu;

    QAction *aIncrementAllCardCounters;
    QAction *aUntapAll, *aRollDie, *aFlipCoin;
    QAction *aCreateToken, *aCreateAnotherToken;
};

#endif // COCKATRICE_UTILITY_MENU_H
