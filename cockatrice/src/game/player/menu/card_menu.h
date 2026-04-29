/**
 * @file card_menu.h
 * @ingroup GameMenusCards
 * @brief TODO: Document this.
 */

#ifndef COCKATRICE_CARD_MENU_H
#define COCKATRICE_CARD_MENU_H

#include <QMenu>

class CardItem;
class Player;
class CardMenu : public QMenu
{
    Q_OBJECT

public:
    explicit CardMenu(Player *player, const CardItem *card, bool shortcutsActive);
    void removePlayer(Player *playerToRemove);
    void createTableMenu(bool canModifyCard);
    void createStackMenu(bool canModifyCard);
    void createGraveyardOrExileMenu(bool canModifyCard);
    void createHandOrCustomZoneMenu(bool canModifyCard);
    void createZonelessMenu(bool canModifyCard);

    QMenu *mCardCounters;

    QAction *aPlay, *aPlayFacedown;
    QAction *aRevealToAll;
    QAction *aHide;
    QAction *aClone;
    QAction *aSelectAll, *aSelectRow, *aSelectColumn, *aSelectAllMatching;
    QAction *aDrawArrow;
    QAction *aTap, *aDoesntUntap;
    QAction *aFlip, *aPeek;
    QAction *aAttach, *aUnattach;
    QAction *aSetAnnotation;

    QList<QAction *> aAddCounter, aSetCounter, aRemoveCounter;

private:
    Player *player;
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
