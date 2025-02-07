#include "deck_view_container.h"

#include "../../client/tabs/tab_game.h"
#include "../../client/ui/picture_loader/picture_loader.h"
#include "../../deck/deck_loader.h"
#include "../../dialogs/dlg_load_deck.h"
#include "../../dialogs/dlg_load_remote_deck.h"
#include "../../server/pending_command.h"
#include "../../settings/cache_settings.h"
#include "../cards/card_database.h"
#include "../cards/card_database_manager.h"
#include "../game_scene.h"
#include "deck_view.h"
#include "pb/command_deck_select.pb.h"
#include "pb/command_ready_start.pb.h"
#include "pb/command_set_sideboard_lock.pb.h"
#include "pb/command_set_sideboard_plan.pb.h"
#include "pb/response_deck_download.pb.h"
#include "trice_limits.h"

#include <QMessageBox>
#include <QMouseEvent>
#include <QToolButton>
#include <google/protobuf/descriptor.h>

ToggleButton::ToggleButton(QWidget *parent) : QPushButton(parent), state(false)
{
}

void ToggleButton::paintEvent(QPaintEvent *event)
{
    QPushButton::paintEvent(event);

    QPainter painter(this);
    QPen pen;
    pen.setWidth(3);
    pen.setJoinStyle(Qt::MiterJoin);
    pen.setColor(state ? Qt::green : Qt::red);
    painter.setPen(pen);
    painter.drawRect(QRect(1, 1, width() - 3, height() - 3));
}

void ToggleButton::setState(bool _state)
{
    state = _state;
    emit stateChanged();
    update();
}

DeckViewContainer::DeckViewContainer(int _playerId, TabGame *parent)
    : QWidget(nullptr), visualDeckStorageWidget(nullptr), parentGame(parent), playerId(_playerId)
{
    loadLocalButton = new QPushButton;
    loadRemoteButton = new QPushButton;
    unloadDeckButton = new QPushButton;
    readyStartButton = new ToggleButton;
    forceStartGameButton = new QPushButton;
    sideboardLockButton = new ToggleButton;

    connect(loadLocalButton, SIGNAL(clicked()), this, SLOT(loadLocalDeck()));
    connect(readyStartButton, SIGNAL(clicked()), this, SLOT(readyStart()));
    connect(unloadDeckButton, &QPushButton::clicked, this, &DeckViewContainer::unloadDeck);
    connect(forceStartGameButton, &QPushButton::clicked, this, &DeckViewContainer::forceStart);
    connect(sideboardLockButton, SIGNAL(clicked()), this, SLOT(sideboardLockButtonClicked()));
    connect(sideboardLockButton, SIGNAL(stateChanged()), this, SLOT(updateSideboardLockButtonText()));

    if (parentGame->getIsLocalGame()) {
        loadRemoteButton->setVisible(false);
        loadRemoteButton->setEnabled(false);
    } else {
        connect(loadRemoteButton, SIGNAL(clicked()), this, SLOT(loadRemoteDeck()));
    }

    auto *buttonHBox = new QHBoxLayout;
    buttonHBox->addWidget(loadLocalButton);
    buttonHBox->addWidget(loadRemoteButton);
    buttonHBox->addWidget(unloadDeckButton);
    buttonHBox->addWidget(readyStartButton);
    buttonHBox->addWidget(sideboardLockButton);
    buttonHBox->addWidget(forceStartGameButton);

    buttonHBox->setContentsMargins(11, 0, 11, 0);
    buttonHBox->addStretch();

    deckView = new DeckView;
    connect(deckView, SIGNAL(newCardAdded(AbstractCardItem *)), this, SIGNAL(newCardAdded(AbstractCardItem *)));
    connect(deckView, SIGNAL(sideboardPlanChanged()), this, SLOT(sideboardPlanChanged()));

    deckViewLayout = new QVBoxLayout;
    deckViewLayout->addLayout(buttonHBox);
    deckViewLayout->addWidget(deckView);
    deckViewLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(deckViewLayout);

    retranslateUi();
    connect(&SettingsCache::instance().shortcuts(), SIGNAL(shortCutChanged()), this, SLOT(refreshShortcuts()));
    refreshShortcuts();

    connect(&SettingsCache::instance(), &SettingsCache::visualDeckStorageInGameChanged, this,
            &DeckViewContainer::updateShowVisualDeckStorage);

    switchToDeckSelectView();
}

/**
 * Creates the VDS widget and inserts it into the layout. No-ops if the widget already exists
 */
void DeckViewContainer::tryCreateVisualDeckStorageWidget()
{
    if (visualDeckStorageWidget) {
        return;
    }

    visualDeckStorageWidget = new VisualDeckStorageWidget(this);
    connect(visualDeckStorageWidget, &VisualDeckStorageWidget::deckLoadRequested, this,
            &DeckViewContainer::loadDeckFromFile);

    deckViewLayout->addWidget(visualDeckStorageWidget);
}

void DeckViewContainer::retranslateUi()
{
    loadLocalButton->setText(tr("Load deck..."));
    loadRemoteButton->setText(tr("Load remote deck..."));
    unloadDeckButton->setText(tr("Unload deck"));
    readyStartButton->setText(tr("Ready to start"));
    forceStartGameButton->setText(tr("Force start"));
    updateSideboardLockButtonText();
}

static void setVisibility(QPushButton *button, bool visible)
{
    button->setHidden(!visible);
    button->setEnabled(visible);
}

void DeckViewContainer::switchToDeckSelectView()
{
    if (SettingsCache::instance().getVisualDeckStorageInGame()) {
        deckView->setHidden(true);

        tryCreateVisualDeckStorageWidget();
        visualDeckStorageWidget->setHidden(false);
    } else {
        deckView->setHidden(false);
        if (visualDeckStorageWidget) {
            visualDeckStorageWidget->setHidden(true);
        }
    }

    deckViewLayout->update();

    setVisibility(loadLocalButton, true);
    setVisibility(loadRemoteButton, !parentGame->getIsLocalGame());
    setVisibility(unloadDeckButton, false);
    setVisibility(readyStartButton, false);
    setVisibility(sideboardLockButton, false);
    setVisibility(forceStartGameButton, false);

    readyStartButton->setState(false);
    sideboardLockButton->setState(false);

    deckView->setLocked(true);

    sendReadyStartCommand(false);
}

void DeckViewContainer::switchToDeckLoadedView()
{
    deckView->setHidden(false);
    if (visualDeckStorageWidget) {
        visualDeckStorageWidget->setHidden(true);
    }

    deckViewLayout->update();

    setVisibility(loadLocalButton, false);
    setVisibility(loadRemoteButton, false);
    setVisibility(unloadDeckButton, true);
    setVisibility(readyStartButton, true);
    setVisibility(sideboardLockButton, true);

    if (parentGame->isHost()) {
        setVisibility(forceStartGameButton, true);
    }
}

void DeckViewContainer::updateSideboardLockButtonText()
{
    if (sideboardLockButton->getState()) {
        sideboardLockButton->setText(tr("Sideboard unlocked"));
    } else {
        sideboardLockButton->setText(tr("Sideboard locked"));
    }
    // setting text on a button removes its shortcut
    ShortcutsSettings &shortcuts = SettingsCache::instance().shortcuts();
    sideboardLockButton->setShortcut(shortcuts.getSingleShortcut("DeckViewContainer/sideboardLockButton"));
}

void DeckViewContainer::refreshShortcuts()
{
    ShortcutsSettings &shortcuts = SettingsCache::instance().shortcuts();
    loadLocalButton->setShortcut(shortcuts.getSingleShortcut("DeckViewContainer/loadLocalButton"));
    loadRemoteButton->setShortcut(shortcuts.getSingleShortcut("DeckViewContainer/loadRemoteButton"));
    readyStartButton->setShortcut(shortcuts.getSingleShortcut("DeckViewContainer/readyStartButton"));
    sideboardLockButton->setShortcut(shortcuts.getSingleShortcut("DeckViewContainer/sideboardLockButton"));
}

/**
 * Update VDS existence when settings change
 */
void DeckViewContainer::updateShowVisualDeckStorage(bool enabled)
{
    if (enabled) {
        tryCreateVisualDeckStorageWidget();
        // view mode state isn't stored in a field, so we determine state by checking the button
        bool isDeckSelectView = loadLocalButton->isEnabled();
        visualDeckStorageWidget->setHidden(!isDeckSelectView);
        deckView->setHidden(isDeckSelectView);
    } else {
        if (visualDeckStorageWidget) {
            visualDeckStorageWidget->deleteLater();
            visualDeckStorageWidget = nullptr;
        }
        deckView->setHidden(false);
    }

    deckViewLayout->update();
}

void DeckViewContainer::unloadDeck()
{
    deckView->clearDeck();
    switchToDeckSelectView();
}

void DeckViewContainer::loadLocalDeck()
{
    DlgLoadDeck dialog(this);
    if (!dialog.exec())
        return;

    loadDeckFromFile(dialog.selectedFiles().at(0));
}

void DeckViewContainer::loadDeckFromFile(const QString &filePath)
{
    DeckLoader::FileFormat fmt = DeckLoader::getFormatFromName(filePath);
    QString deckString;
    DeckLoader deck;

    bool error = !deck.loadFromFile(filePath, fmt, true);
    if (!error) {
        deckString = deck.writeToString_Native();
        error = deckString.length() > MAX_FILE_LENGTH;
    }
    if (error) {
        QMessageBox::critical(this, tr("Error"), tr("The selected file could not be loaded."));
        return;
    }

    Command_DeckSelect cmd;
    cmd.set_deck(deckString.toStdString());
    PendingCommand *pend = parentGame->prepareGameCommand(cmd);
    connect(pend, SIGNAL(finished(Response, CommandContainer, QVariant)), this,
            SLOT(deckSelectFinished(const Response &)));
    parentGame->sendGameCommand(pend, playerId);
}

void DeckViewContainer::loadRemoteDeck()
{
    DlgLoadRemoteDeck dlg(parentGame->getClientForPlayer(playerId), this);
    if (dlg.exec()) {
        Command_DeckSelect cmd;
        cmd.set_deck_id(dlg.getDeckId());
        PendingCommand *pend = parentGame->prepareGameCommand(cmd);
        connect(pend, SIGNAL(finished(Response, CommandContainer, QVariant)), this,
                SLOT(deckSelectFinished(const Response &)));
        parentGame->sendGameCommand(pend, playerId);
    }
}

void DeckViewContainer::deckSelectFinished(const Response &r)
{
    const Response_DeckDownload &resp = r.GetExtension(Response_DeckDownload::ext);
    DeckLoader newDeck(QString::fromStdString(resp.deck()));
    // TODO CHANGE THIS TO BE SELECTED BY UUID
    PictureLoader::cacheCardPixmaps(CardDatabaseManager::getInstance()->getCards(newDeck.getCardList()));
    setDeck(newDeck);
    switchToDeckLoadedView();
}

void DeckViewContainer::readyStart()
{
    sendReadyStartCommand(!readyStartButton->getState());
}

void DeckViewContainer::forceStart()
{
    Command_ReadyStart cmd;
    cmd.set_force_start(true);
    cmd.set_ready(true);
    parentGame->sendGameCommand(cmd, playerId);
}

void DeckViewContainer::sideboardLockButtonClicked()
{
    Command_SetSideboardLock cmd;
    cmd.set_locked(sideboardLockButton->getState());

    parentGame->sendGameCommand(cmd, playerId);
}

void DeckViewContainer::sideboardPlanChanged()
{
    Command_SetSideboardPlan cmd;
    const QList<MoveCard_ToZone> &newPlan = deckView->getSideboardPlan();
    for (const auto &i : newPlan)
        cmd.add_move_list()->CopyFrom(i);
    parentGame->sendGameCommand(cmd, playerId);
}

/**
 * Sends the basic ReadyStart command.
 */
void DeckViewContainer::sendReadyStartCommand(bool ready)
{
    Command_ReadyStart cmd;
    cmd.set_ready(ready);
    parentGame->sendGameCommand(cmd, playerId);
}

/**
 * Updates the buttons to make the client-side ready state match the given state.
 *
 * Notably, this method only updates the client and *does not* send a ReadyStart command to the server.
 * This method is intended to be called upon receiving the response from a ReadyStart command.
 */
void DeckViewContainer::setReadyStart(bool ready)
{
    readyStartButton->setState(ready);
    deckView->setLocked(ready || !sideboardLockButton->getState());
    sideboardLockButton->setEnabled(!readyStartButton->getState() && readyStartButton->isEnabled());
}

/**
 * Sends a ReadyStart command with ready=true to the server
 */
void DeckViewContainer::readyAndUpdate()
{
    sendReadyStartCommand(true);
}

void DeckViewContainer::setSideboardLocked(bool locked)
{
    sideboardLockButton->setState(!locked);
    deckView->setLocked(readyStartButton->getState() || !sideboardLockButton->getState());
    if (locked)
        deckView->resetSideboardPlan();
}

void DeckViewContainer::setDeck(const DeckLoader &deck)
{
    deckView->setDeck(deck);
    switchToDeckLoadedView();
}