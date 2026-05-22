/**
 * @file card_menu.h
 * @ingroup GameMenusCards
 */
//! \todo Document this file.

#ifndef COCKATRICE_CARD_MENU_H
#define COCKATRICE_CARD_MENU_H

#include <QMenu>
#include <libcockatrice/utility/card_ref.h>

class CardItem;
class PlayerGraphicsItem;
class PlayerLogic;
class CardMenu : public QMenu
{
    Q_OBJECT

signals:
    void cardInfoRequested(const CardRef &cardRef);

public:
    explicit CardMenu(PlayerGraphicsItem *player, const CardItem *card, bool shortcutsActive);
    void removePlayer(PlayerLogic *playerToRemove);
    void createTableMenu(bool canModifyCard);
    void createStackMenu(bool canModifyCard);
    void createGraveyardOrExileMenu(bool canModifyCard);
    void createHandOrCustomZoneMenu(bool canModifyCard);
    void createZonelessMenu(bool canModifyCard);

    QMenu *mCardCounters;

    QAction *aPlay, *aPlayFacedown;
    QAction *
        aPlayAndIncreaseTax; ///< Plays card and increments the primary commander tax counter (CounterIds::CommanderTax)
    QAction *aPlayAndIncreasePartnerTax;
    QAction *aRevealToAll;
    QAction *aHide;
    QAction *aClone;
    QAction *aSelectAll, *aSelectRow, *aSelectColumn;
    QAction *aDrawArrow;
    QAction *aTap, *aDoesntUntap;
    QAction *aFlip, *aPeek;
    QAction *aAttach, *aUnattach;
    QAction *aSetAnnotation;
    QAction *aReduceLifeByPower;

    QList<QAction *> aAddCounter, aSetCounter, aRemoveCounter;

private:
    PlayerGraphicsItem *player;
    const CardItem *card;
    QList<QPair<QString, int>> playersInfo;
    bool shortcutsActive;

    void addRelatedCardActions();
    void retranslateUi();
    void initContextualPlayersMenu(QMenu *menu, QAction *allPlayersAction);
    void setShortcutsActive();
    void addRelatedCardView();
};

#endif // COCKATRICE_CARD_MENU_H
