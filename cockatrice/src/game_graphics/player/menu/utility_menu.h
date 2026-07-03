/**
 * @file utility_menu.h
 * @ingroup GameMenusPlayers
 */
//! \todo Document this file.

#ifndef COCKATRICE_UTILITY_MENU_H
#define COCKATRICE_UTILITY_MENU_H

#include "abstract_player_component.h"

#include <QMenu>
#include <libcockatrice/card/card_info.h>

class PlayerGraphicsItem;
class UtilityMenu : public QMenu, public AbstractPlayerComponent
{
    Q_OBJECT
public slots:
    void populatePredefinedTokensMenu();
    void setLastToken(CardInfoPtr lastToken);
    void retranslateUi() override;
    void setShortcutsActive() override;
    void setShortcutsInactive() override;

public:
    explicit UtilityMenu(PlayerGraphicsItem *player, QMenu *playerMenu);

    [[nodiscard]] bool createAnotherTokenActionExists() const
    {
        return aCreateAnotherToken != nullptr;
    }

    void setAndEnableCreateAnotherTokenAction(QString text)
    {
        aCreateAnotherToken->setText(tr("C&reate another %1 token").arg(text));
        aCreateAnotherToken->setEnabled(true);
    }

    QStringList getPredefinedTokens() const
    {
        return predefinedTokens;
    }

private:
    PlayerGraphicsItem *player;
    QStringList predefinedTokens;

    QMenu *createPredefinedTokenMenu;

    QAction *aIncrementAllCardCounters;
    QAction *aUntapAll, *aRollDie, *aFlipCoin;
    QAction *aCreateToken, *aCreateAnotherToken;
};

#endif // COCKATRICE_UTILITY_MENU_H
