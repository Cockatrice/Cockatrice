#include "library_menu.h"

#include "../../../client/settings/cache_settings.h"
#include "../../../client/settings/shortcuts_settings.h"
#include "../../../interface/widgets/tabs/tab_game.h"
#include "../../abstract_game.h"
#include "../player.h"
#include "../player_actions.h"

#include <QAction>
#include <QMenu>

LibraryMenu::LibraryMenu(Player *_player, QWidget *parent) : TearOffMenu(parent), player(_player)
{
    createDrawActions();
    createShuffleActions();
    createMoveActions();
    createViewActions();

    addAction(aDrawCard);
    addAction(aDrawCards);
    addAction(aUndoDraw);
    addSeparator();
    addAction(aShuffle);
    addSeparator();
    addAction(aViewLibrary);
    addAction(aViewTopCards);
    addAction(aViewBottomCards);
    addSeparator();

    mRevealLibrary = addMenu(QString());
    connect(mRevealLibrary, &QMenu::aboutToShow, this, &LibraryMenu::populateRevealLibraryMenuWithActivePlayers);

    mLendLibrary = addMenu(QString());
    connect(mLendLibrary, &QMenu::aboutToShow, this, &LibraryMenu::populateLendLibraryMenuWithActivePlayers);

    mRevealTopCard = addMenu(QString());
    connect(mRevealTopCard, &QMenu::aboutToShow, this, &LibraryMenu::populateRevealTopCardMenuWithActivePlayers);

    addAction(aAlwaysRevealTopCard);
    addAction(aAlwaysLookAtTopCard);
    addSeparator();
    topLibraryMenu = addTearOffMenu(QString());
    bottomLibraryMenu = addTearOffMenu(QString());
    addSeparator();
    addAction(aOpenDeckInDeckEditor);

    topLibraryMenu->addAction(aMoveTopToPlay);
    topLibraryMenu->addAction(aMoveTopToPlayFaceDown);
    topLibraryMenu->addAction(aMoveTopCardToBottom);
    topLibraryMenu->addSeparator();
    topLibraryMenu->addAction(aMoveTopCardToGraveyard);
    topLibraryMenu->addAction(aMoveTopCardsToGraveyard);
    topLibraryMenu->addAction(aMoveTopCardToExile);
    topLibraryMenu->addAction(aMoveTopCardsToExile);
    topLibraryMenu->addAction(aMoveTopCardsUntil);
    topLibraryMenu->addSeparator();
    topLibraryMenu->addAction(aShuffleTopCards);

    bottomLibraryMenu->addAction(aDrawBottomCard);
    bottomLibraryMenu->addAction(aDrawBottomCards);
    bottomLibraryMenu->addSeparator();
    bottomLibraryMenu->addAction(aMoveBottomToPlay);
    bottomLibraryMenu->addAction(aMoveBottomToPlayFaceDown);
    bottomLibraryMenu->addAction(aMoveBottomCardToTop);
    bottomLibraryMenu->addSeparator();
    bottomLibraryMenu->addAction(aMoveBottomCardToGraveyard);
    bottomLibraryMenu->addAction(aMoveBottomCardsToGraveyard);
    bottomLibraryMenu->addAction(aMoveBottomCardToExile);
    bottomLibraryMenu->addAction(aMoveBottomCardsToExile);
    bottomLibraryMenu->addSeparator();
    bottomLibraryMenu->addAction(aShuffleBottomCards);

    connect(player, &Player::resetTopCardMenuActions, this, &LibraryMenu::resetTopCardMenuActions);
    connect(player, &Player::deckChanged, this, &LibraryMenu::enableOpenInDeckEditorAction);

    retranslateUi();
}

void LibraryMenu::enableOpenInDeckEditorAction() const
{
    aOpenDeckInDeckEditor->setEnabled(true);
}

void LibraryMenu::resetTopCardMenuActions()
{
    aAlwaysRevealTopCard->setChecked(false);
    aAlwaysLookAtTopCard->setChecked(false);
}

void LibraryMenu::createDrawActions()
{
    PlayerActions *playerActions = player->getPlayerActions();

    if (player->getPlayerInfo()->local || player->getPlayerInfo()->judge) {
        aDrawCard = new QAction(this);
        connect(aDrawCard, &QAction::triggered, playerActions, &PlayerActions::actDrawCard);
        aDrawCards = new QAction(this);
        connect(aDrawCards, &QAction::triggered, playerActions, &PlayerActions::actDrawCards);
        aUndoDraw = new QAction(this);
        connect(aUndoDraw, &QAction::triggered, playerActions, &PlayerActions::actUndoDraw);
        aDrawBottomCard = new QAction(this);
        connect(aDrawBottomCard, &QAction::triggered, playerActions, &PlayerActions::actDrawBottomCard);
        aDrawBottomCards = new QAction(this);
        connect(aDrawBottomCards, &QAction::triggered, playerActions, &PlayerActions::actDrawBottomCards);
    }
}

void LibraryMenu::createShuffleActions()
{
    PlayerActions *playerActions = player->getPlayerActions();

    if (player->getPlayerInfo()->local || player->getPlayerInfo()->judge) {
        aShuffle = new QAction(this);
        connect(aShuffle, &QAction::triggered, playerActions, &PlayerActions::actShuffle);
        aShuffleTopCards = new QAction(this);
        connect(aShuffleTopCards, &QAction::triggered, playerActions, &PlayerActions::actShuffleTop);
        aShuffleBottomCards = new QAction(this);
        connect(aShuffleBottomCards, &QAction::triggered, playerActions, &PlayerActions::actShuffleBottom);
    }
}

void LibraryMenu::createMoveActions()
{
    PlayerActions *playerActions = player->getPlayerActions();

    if (player->getPlayerInfo()->local || player->getPlayerInfo()->judge) {
        aMoveTopToPlay = new QAction(this);
        connect(aMoveTopToPlay, &QAction::triggered, playerActions, &PlayerActions::actMoveTopCardToPlay);
        aMoveTopToPlayFaceDown = new QAction(this);
        connect(aMoveTopToPlayFaceDown, &QAction::triggered, playerActions,
                &PlayerActions::actMoveTopCardToPlayFaceDown);
        aMoveTopCardToGraveyard = new QAction(this);
        connect(aMoveTopCardToGraveyard, &QAction::triggered, playerActions, &PlayerActions::actMoveTopCardToGrave);
        aMoveTopCardToExile = new QAction(this);
        connect(aMoveTopCardToExile, &QAction::triggered, playerActions, &PlayerActions::actMoveTopCardToExile);
        aMoveTopCardsToGraveyard = new QAction(this);
        connect(aMoveTopCardsToGraveyard, &QAction::triggered, playerActions, &PlayerActions::actMoveTopCardsToGrave);
        aMoveTopCardsToExile = new QAction(this);
        connect(aMoveTopCardsToExile, &QAction::triggered, playerActions, &PlayerActions::actMoveTopCardsToExile);
        aMoveTopCardsUntil = new QAction(this);
        connect(aMoveTopCardsUntil, &QAction::triggered, playerActions, &PlayerActions::actMoveTopCardsUntil);
        aMoveTopCardToBottom = new QAction(this);
        connect(aMoveTopCardToBottom, &QAction::triggered, playerActions, &PlayerActions::actMoveTopCardToBottom);

        aMoveBottomToPlay = new QAction(this);
        connect(aMoveBottomToPlay, &QAction::triggered, playerActions, &PlayerActions::actMoveBottomCardToPlay);
        aMoveBottomToPlayFaceDown = new QAction(this);
        connect(aMoveBottomToPlayFaceDown, &QAction::triggered, playerActions,
                &PlayerActions::actMoveBottomCardToPlayFaceDown);
        aMoveBottomCardToGraveyard = new QAction(this);
        connect(aMoveBottomCardToGraveyard, &QAction::triggered, playerActions,
                &PlayerActions::actMoveBottomCardToGrave);
        aMoveBottomCardToExile = new QAction(this);
        connect(aMoveBottomCardToExile, &QAction::triggered, playerActions, &PlayerActions::actMoveBottomCardToExile);
        aMoveBottomCardsToGraveyard = new QAction(this);
        connect(aMoveBottomCardsToGraveyard, &QAction::triggered, playerActions,
                &PlayerActions::actMoveBottomCardsToGrave);
        aMoveBottomCardsToExile = new QAction(this);
        connect(aMoveBottomCardsToExile, &QAction::triggered, playerActions, &PlayerActions::actMoveBottomCardsToExile);
        aMoveBottomCardToTop = new QAction(this);
        connect(aMoveBottomCardToTop, &QAction::triggered, playerActions, &PlayerActions::actMoveBottomCardToTop);
    }
}

void LibraryMenu::createViewActions()
{
    PlayerActions *playerActions = player->getPlayerActions();

    if (player->getPlayerInfo()->local || player->getPlayerInfo()->judge) {
        aViewLibrary = new QAction(this);
        connect(aViewLibrary, &QAction::triggered, playerActions, &PlayerActions::actViewLibrary);

        aViewTopCards = new QAction(this);
        connect(aViewTopCards, &QAction::triggered, playerActions, &PlayerActions::actViewTopCards);
        aViewBottomCards = new QAction(this);
        connect(aViewBottomCards, &QAction::triggered, playerActions, &PlayerActions::actViewBottomCards);
        aAlwaysRevealTopCard = new QAction(this);
        aAlwaysRevealTopCard->setCheckable(true);
        connect(aAlwaysRevealTopCard, &QAction::triggered, playerActions, &PlayerActions::actAlwaysRevealTopCard);
        aAlwaysLookAtTopCard = new QAction(this);
        aAlwaysLookAtTopCard->setCheckable(true);
        connect(aAlwaysLookAtTopCard, &QAction::triggered, playerActions, &PlayerActions::actAlwaysLookAtTopCard);
        aOpenDeckInDeckEditor = new QAction(this);
        aOpenDeckInDeckEditor->setEnabled(false);
        connect(aOpenDeckInDeckEditor, &QAction::triggered, playerActions, &PlayerActions::actOpenDeckInDeckEditor);
    }
}

void LibraryMenu::retranslateUi()
{
    setTitle(tr("&Library"));

    if (player->getPlayerInfo()->getLocalOrJudge()) {
        aViewLibrary->setText(tr("&View library"));
        aViewTopCards->setText(tr("View &top cards of library..."));
        aViewBottomCards->setText(tr("View bottom cards of library..."));
        mRevealLibrary->setTitle(tr("Reveal &library to..."));
        mLendLibrary->setTitle(tr("Lend library to..."));
        mRevealTopCard->setTitle(tr("Reveal &top cards to..."));
        topLibraryMenu->setTitle(tr("&Top of library..."));
        bottomLibraryMenu->setTitle(tr("&Bottom of library..."));
        aAlwaysRevealTopCard->setText(tr("&Always reveal top card"));
        aAlwaysLookAtTopCard->setText(tr("&Always look at top card"));
        aOpenDeckInDeckEditor->setText(tr("&Open deck in deck editor"));

        aDrawCard->setText(tr("&Draw card"));
        aDrawCards->setText(tr("D&raw cards..."));
        aUndoDraw->setText(tr("&Undo last draw"));

        aShuffle->setText(tr("Shuffle"));

        aMoveTopToPlay->setText(tr("&Play top card"));
        aMoveTopToPlayFaceDown->setText(tr("Play top card &face down"));
        aMoveTopCardToBottom->setText(tr("Put top card on &bottom"));
        aMoveTopCardToGraveyard->setText(tr("Move top card to grave&yard"));
        aMoveTopCardToExile->setText(tr("Move top card to e&xile"));
        aMoveTopCardsToGraveyard->setText(tr("Move top cards to &graveyard..."));
        aMoveTopCardsToExile->setText(tr("Move top cards to &exile..."));
        aMoveTopCardsUntil->setText(tr("Put top cards on stack &until..."));
        aShuffleTopCards->setText(tr("Shuffle top cards..."));

        aDrawBottomCard->setText(tr("&Draw bottom card"));
        aDrawBottomCards->setText(tr("D&raw bottom cards..."));
        aMoveBottomToPlay->setText(tr("&Play bottom card"));
        aMoveBottomToPlayFaceDown->setText(tr("Play bottom card &face down"));
        aMoveBottomCardToGraveyard->setText(tr("Move bottom card to grave&yard"));
        aMoveBottomCardToExile->setText(tr("Move bottom card to e&xile"));
        aMoveBottomCardsToGraveyard->setText(tr("Move bottom cards to &graveyard..."));
        aMoveBottomCardsToExile->setText(tr("Move bottom cards to &exile..."));
        aMoveBottomCardToTop->setText(tr("Put bottom card on &top"));
        aShuffleBottomCards->setText(tr("Shuffle bottom cards..."));
    }
}

void LibraryMenu::populateRevealLibraryMenuWithActivePlayers()
{
    mRevealLibrary->clear();

    QAction *allPlayers = mRevealLibrary->addAction(tr("&All players"));
    allPlayers->setData(-1);
    connect(allPlayers, &QAction::triggered, this, &LibraryMenu::onRevealLibraryTriggered);

    mRevealLibrary->addSeparator();

    const auto &players = player->getGame()->getPlayerManager()->getPlayers().values();
    for (auto *other : players) {
        if (other == player)
            continue;
        QAction *a = mRevealLibrary->addAction(other->getPlayerInfo()->getName());
        a->setData(other->getPlayerInfo()->getId());
        connect(a, &QAction::triggered, this, &LibraryMenu::onRevealLibraryTriggered);
    }
}

void LibraryMenu::populateLendLibraryMenuWithActivePlayers()
{
    mLendLibrary->clear();

    const auto &players = player->getGame()->getPlayerManager()->getPlayers().values();
    for (auto *other : players) {
        if (other == player)
            continue;
        QAction *a = mLendLibrary->addAction(other->getPlayerInfo()->getName());
        a->setData(other->getPlayerInfo()->getId());
        connect(a, &QAction::triggered, this, &LibraryMenu::onLendLibraryTriggered);
    }
}

void LibraryMenu::populateRevealTopCardMenuWithActivePlayers()
{
    mRevealTopCard->clear();

    QAction *allPlayers = mRevealTopCard->addAction(tr("&All players"));
    allPlayers->setData(-1);
    connect(allPlayers, &QAction::triggered, this, &LibraryMenu::onRevealTopCardTriggered);

    mRevealTopCard->addSeparator();

    const auto &players = player->getGame()->getPlayerManager()->getPlayers().values();
    for (auto *other : players) {
        if (other == player)
            continue;
        QAction *a = mRevealTopCard->addAction(other->getPlayerInfo()->getName());
        a->setData(other->getPlayerInfo()->getId());
        connect(a, &QAction::triggered, this, &LibraryMenu::onRevealTopCardTriggered);
    }
}

void LibraryMenu::onRevealLibraryTriggered()
{
    if (auto *a = qobject_cast<QAction *>(sender())) {
        player->getPlayerActions()->actRevealLibrary(a->data().toInt());
    }
}

void LibraryMenu::onLendLibraryTriggered()
{
    if (auto *a = qobject_cast<QAction *>(sender())) {
        player->getPlayerActions()->actLendLibrary(a->data().toInt());
    }
}

void LibraryMenu::onRevealTopCardTriggered()
{
    if (auto *a = qobject_cast<QAction *>(sender())) {
        int deckSize = player->getDeckZone()->getCards().size();
        bool ok;
        int number = QInputDialog::getInt(player->getGame()->getTab(), tr("Reveal top cards of library"),
                                          tr("Number of cards: (max. %1)").arg(deckSize), defaultNumberTopCards, 1,
                                          deckSize, 1, &ok);
        if (ok) {
            player->getPlayerActions()->actRevealTopCards(a->data().toInt(), number);
            defaultNumberTopCards = number;
        }
    }
}

void LibraryMenu::setShortcutsActive()
{
    ShortcutsSettings &shortcuts = SettingsCache::instance().shortcuts();

    aViewLibrary->setShortcuts(shortcuts.getShortcut("Player/aViewLibrary"));
    aViewTopCards->setShortcuts(shortcuts.getShortcut("Player/aViewTopCards"));
    aViewBottomCards->setShortcuts(shortcuts.getShortcut("Player/aViewBottomCards"));
    aDrawCard->setShortcuts(shortcuts.getShortcut("Player/aDrawCard"));
    aDrawCards->setShortcuts(shortcuts.getShortcut("Player/aDrawCards"));
    aUndoDraw->setShortcuts(shortcuts.getShortcut("Player/aUndoDraw"));
    aShuffle->setShortcuts(shortcuts.getShortcut("Player/aShuffle"));
    aShuffleTopCards->setShortcuts(shortcuts.getShortcut("Player/aShuffleTopCards"));
    aShuffleBottomCards->setShortcuts(shortcuts.getShortcut("Player/aShuffleBottomCards"));
    aAlwaysRevealTopCard->setShortcuts(shortcuts.getShortcut("Player/aAlwaysRevealTopCard"));
    aAlwaysLookAtTopCard->setShortcuts(shortcuts.getShortcut("Player/aAlwaysLookAtTopCard"));
    aMoveTopToPlay->setShortcuts(shortcuts.getShortcut("Player/aMoveTopToPlay"));
    aMoveTopToPlayFaceDown->setShortcuts(shortcuts.getShortcut("Player/aMoveTopToPlayFaceDown"));
    aMoveTopCardToGraveyard->setShortcuts(shortcuts.getShortcut("Player/aMoveTopCardToGraveyard"));
    aMoveTopCardsToGraveyard->setShortcuts(shortcuts.getShortcut("Player/aMoveTopCardsToGraveyard"));
    aMoveTopCardToExile->setShortcuts(shortcuts.getShortcut("Player/aMoveTopCardToExile"));
    aMoveTopCardsToExile->setShortcuts(shortcuts.getShortcut("Player/aMoveTopCardsToExile"));
    aMoveTopCardsUntil->setShortcuts(shortcuts.getShortcut("Player/aMoveTopCardsUntil"));
    aMoveTopCardToBottom->setShortcuts(shortcuts.getShortcut("Player/aMoveTopCardToBottom"));
    aDrawBottomCard->setShortcuts(shortcuts.getShortcut("Player/aDrawBottomCard"));
    aDrawBottomCards->setShortcuts(shortcuts.getShortcut("Player/aDrawBottomCards"));
    aMoveBottomToPlay->setShortcuts(shortcuts.getShortcut("Player/aMoveBottomToPlay"));
    aMoveBottomToPlayFaceDown->setShortcuts(shortcuts.getShortcut("Player/aMoveBottomToPlayFaceDown"));
    aMoveBottomCardToGraveyard->setShortcuts(shortcuts.getShortcut("Player/aMoveBottomCardToGrave"));
    aMoveBottomCardsToGraveyard->setShortcuts(shortcuts.getShortcut("Player/aMoveBottomCardsToGrave"));
    aMoveBottomCardToExile->setShortcuts(shortcuts.getShortcut("Player/aMoveBottomCardToExile"));
    aMoveBottomCardsToExile->setShortcuts(shortcuts.getShortcut("Player/aMoveBottomCardsToExile"));
    aMoveBottomCardToTop->setShortcuts(shortcuts.getShortcut("Player/aMoveBottomCardToTop"));
}

void LibraryMenu::setShortcutsInactive()
{
    aViewLibrary->setShortcut(QKeySequence());
    aViewTopCards->setShortcut(QKeySequence());
    aViewBottomCards->setShortcut(QKeySequence());
    aDrawCard->setShortcut(QKeySequence());
    aDrawCards->setShortcut(QKeySequence());
    aUndoDraw->setShortcut(QKeySequence());
    aShuffle->setShortcut(QKeySequence());
    aShuffleTopCards->setShortcut(QKeySequence());
    aShuffleBottomCards->setShortcut(QKeySequence());
    aAlwaysRevealTopCard->setShortcut(QKeySequence());
    aAlwaysLookAtTopCard->setShortcut(QKeySequence());
    aMoveTopToPlay->setShortcut(QKeySequence());
    aMoveTopToPlayFaceDown->setShortcut(QKeySequence());
    aMoveTopCardToGraveyard->setShortcut(QKeySequence());
    aMoveTopCardsToGraveyard->setShortcut(QKeySequence());
    aMoveTopCardToExile->setShortcut(QKeySequence());
    aMoveTopCardsToExile->setShortcut(QKeySequence());
    aMoveTopCardsUntil->setShortcut(QKeySequence());
    aDrawBottomCard->setShortcut(QKeySequence());
    aDrawBottomCards->setShortcut(QKeySequence());
    aMoveBottomToPlay->setShortcut(QKeySequence());
    aMoveBottomToPlayFaceDown->setShortcut(QKeySequence());
    aMoveBottomCardToGraveyard->setShortcut(QKeySequence());
    aMoveBottomCardsToGraveyard->setShortcut(QKeySequence());
    aMoveBottomCardToExile->setShortcut(QKeySequence());
    aMoveBottomCardsToExile->setShortcut(QKeySequence());
}
