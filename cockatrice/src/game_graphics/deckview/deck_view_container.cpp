#include "deck_view_container.h"

#include "../../client/settings/cache_settings.h"
#include "../../client/settings/shortcuts_settings.h"
#include "../../interface/card_picture_loader/card_picture_loader.h"
#include "../../interface/deck_loader/deck_loader.h"
#include "../../interface/widgets/dialogs/dlg_load_deck.h"
#include "../../interface/widgets/dialogs/dlg_load_deck_from_clipboard.h"
#include "../../interface/widgets/dialogs/dlg_load_deck_from_website.h"
#include "../../interface/widgets/dialogs/dlg_load_remote_deck.h"
#include "../../interface/widgets/tabs/tab_game.h"
#include "../../interface/widgets/visual_deck_storage/visual_deck_storage_widget.h"
#include "deck_view.h"

#include <QMessageBox>
#include <libcockatrice/card/database/card_database.h>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/deck_list/playmat_resolver.h>
#include <libcockatrice/protocol/pb/command_deck_select.pb.h>
#include <libcockatrice/protocol/pb/command_ready_start.pb.h>
#include <libcockatrice/protocol/pb/command_set_playmat.pb.h>
#include <libcockatrice/protocol/pb/command_set_sideboard_lock.pb.h>
#include <libcockatrice/protocol/pb/command_set_sideboard_plan.pb.h>
#include <libcockatrice/protocol/pb/response_deck_download.pb.h>
#include <libcockatrice/protocol/pending_command.h>
#include <libcockatrice/settings/interface_settings.h>
#include <libcockatrice/settings/visual_deck_storage_settings.h>
#include <libcockatrice/utility/string_limits.h>

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
    loadFromClipboardButton = new QPushButton;
    loadFromWebsiteButton = new QPushButton;
    unloadDeckButton = new QPushButton;
    readyStartButton = new ToggleButton;
    forceStartGameButton = new QPushButton;
    sideboardLockButton = new ToggleButton;

    connect(loadLocalButton, &QPushButton::clicked, this, &DeckViewContainer::loadLocalDeck);
    connect(loadRemoteButton, &QPushButton::clicked, this, &DeckViewContainer::loadRemoteDeck);
    connect(loadFromClipboardButton, &QPushButton::clicked, this, &DeckViewContainer::loadFromClipboard);
    connect(loadFromWebsiteButton, &QPushButton::clicked, this, &DeckViewContainer::loadFromWebsite);
    connect(readyStartButton, &QPushButton::clicked, this, &DeckViewContainer::readyStart);
    connect(unloadDeckButton, &QPushButton::clicked, this, &DeckViewContainer::unloadDeck);
    connect(forceStartGameButton, &QPushButton::clicked, this, &DeckViewContainer::forceStart);
    connect(sideboardLockButton, &QPushButton::clicked, this, &DeckViewContainer::sideboardLockButtonClicked);
    connect(sideboardLockButton, &ToggleButton::stateChanged, this, &DeckViewContainer::updateSideboardLockButtonText);

    auto *buttonHBox = new QHBoxLayout;
    buttonHBox->addWidget(loadLocalButton);
    buttonHBox->addWidget(loadRemoteButton);
    buttonHBox->addWidget(loadFromClipboardButton);
    buttonHBox->addWidget(loadFromWebsiteButton);
    buttonHBox->addWidget(unloadDeckButton);
    buttonHBox->addWidget(readyStartButton);
    buttonHBox->addWidget(sideboardLockButton);
    buttonHBox->addWidget(forceStartGameButton);

    buttonHBox->setContentsMargins(11, 0, 11, 0);
    buttonHBox->addStretch();

    deckView = new DeckView;
    connect(deckView, &DeckView::newCardAdded, this, &DeckViewContainer::newCardAdded);
    connect(deckView, &DeckView::sideboardPlanChanged, this, &DeckViewContainer::sideboardPlanChanged);

    deckViewLayout = new QVBoxLayout;
    deckViewLayout->addLayout(buttonHBox);
    deckViewLayout->addWidget(deckView);
    deckViewLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(deckViewLayout);

    retranslateUi();
    connect(&SettingsCache::instance().shortcuts(), &ShortcutsSettings::shortCutChanged, this,
            &DeckViewContainer::refreshShortcuts);
    refreshShortcuts();

    connect(&SettingsCache::instance().visualDeckStorage(), &VisualDeckStorageSettings::visualDeckStorageInGameChanged,
            this, &DeckViewContainer::setVisualDeckStorageExists);

    connect(&SettingsCache::instance().userInterface(), &InterfaceSettings::playmatSettingsChanged, this,
            &DeckViewContainer::onPlaymatSettingsChanged);

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
    connect(visualDeckStorageWidget, &VisualDeckStorageWidget::openDeckEditor, parentGame, &TabGame::openDeckEditor);

    deckViewLayout->addWidget(visualDeckStorageWidget);
}

void DeckViewContainer::retranslateUi()
{
    loadLocalButton->setText(tr("Load deck..."));
    loadRemoteButton->setText(tr("Load remote deck..."));
    loadFromClipboardButton->setText(tr("Load from clipboard..."));
    loadFromWebsiteButton->setText(tr("Load from website..."));
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
    if (SettingsCache::instance().visualDeckStorage().getVisualDeckStorageInGame()) {
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
    setVisibility(loadRemoteButton, !parentGame->getGame()->getGameState()->getIsLocalGame());
    setVisibility(loadFromClipboardButton, true);
    setVisibility(loadFromWebsiteButton, true);
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
    setVisibility(loadFromClipboardButton, false);
    setVisibility(loadFromWebsiteButton, false);
    setVisibility(unloadDeckButton, true);
    setVisibility(readyStartButton, true);
    setVisibility(sideboardLockButton, true);

    if (parentGame->getGame()->isHost()) {
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
    loadFromClipboardButton->setShortcut(shortcuts.getSingleShortcut("DeckViewContainer/loadFromClipboardButton"));
    loadFromWebsiteButton->setShortcut(shortcuts.getSingleShortcut("DeckViewContainer/loadFromWebsiteButton"));
    unloadDeckButton->setShortcut(shortcuts.getSingleShortcut("DeckViewContainer/unloadDeckButton"));
    readyStartButton->setShortcut(shortcuts.getSingleShortcut("DeckViewContainer/readyStartButton"));
    sideboardLockButton->setShortcut(shortcuts.getSingleShortcut("DeckViewContainer/sideboardLockButton"));
    forceStartGameButton->setShortcut(shortcuts.getSingleShortcut("DeckViewContainer/forceStartGameButton"));
}

/**
 * Updates the existence of the embedded Visual Deck Storage, destroying or creating it if needed.
 * Note that this change is temporary; the VDS may get recreated when the view transitions to the deck select state,
 * depending on current settings.
 */
void DeckViewContainer::setVisualDeckStorageExists(bool exists)
{
    if (exists) {
        // view mode state isn't stored in a field, so we determine state by checking the button
        if (loadLocalButton->isEnabled()) {
            // We only need to handle the setting changing while in deck select state; tryCreate already gets called
            // when switching from deck loaded to deck select state
            tryCreateVisualDeckStorageWidget();
            visualDeckStorageWidget->setHidden(false);
            deckView->setHidden(true);
        }
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
    if (!dialog.exec()) {
        return;
    }

    loadDeckFromFile(dialog.selectedFiles().at(0));
}

void DeckViewContainer::loadDeckFromFile(const QString &filePath)
{
    DeckFileFormat::Format fmt = DeckFileFormat::getFormatFromName(filePath);

    std::optional<LoadedDeck> deckOpt = DeckLoader::loadFromFile(filePath, fmt, true);

    if (!deckOpt) {
        QMessageBox::critical(this, tr("Error"), tr("The selected file could not be loaded."));
        return;
    }

    loadDeckFromDeckList(deckOpt.value().deckList);
}

void DeckViewContainer::loadDeckFromDeckList(const DeckList &deck)
{
    currentDeck = deck;

    QString deckString = deck.writeToString_Native();

    if (deckString.length() > MAX_FILE_LENGTH) {
        QMessageBox::critical(this, tr("Error"), tr("Deck is greater than maximum file size."));
        return;
    }

    Command_DeckSelect cmd;
    cmd.set_deck(deckString.toStdString());
    PendingCommand *pend = parentGame->getGame()->getGameEventHandler()->prepareGameCommand(cmd);
    connect(pend, &PendingCommand::finished, this, &DeckViewContainer::deckSelectFinished);
    parentGame->getGame()->getGameEventHandler()->sendGameCommand(pend, playerId);

    resolveAndSendPlaymat();
}

void DeckViewContainer::resolveAndSendPlaymat()
{
    if (currentDeck.getCardRefList().isEmpty() && currentDeck.getPlaymat().card.isEmpty()) {
        return;
    }

    const auto &settings = SettingsCache::instance().userInterface();
    const auto fallbackBehavior = static_cast<PlaymatFallbackMode>(settings.getPlaymatFallbackBehavior());

    QList<PlaymatInfo> fallbackList = settings.getPlaymatFallbackList();

    // In random mode with 2+ entries, remove the last-resolved mat to avoid repeats.
    if (fallbackBehavior == PlaymatFallbackModeRandom && fallbackList.size() > 1) {
        fallbackList.removeAll(lastResolvedPlaymat);
    }

    const PlaymatInfo resolved =
        resolvePlaymatForDeck(currentDeck, fallbackList, static_cast<PlaymatMode>(settings.getPlaymatMode()),
                              fallbackBehavior, playmatRotationIndex);

    lastResolvedPlaymat = resolved;

    Command_SetPlaymat playmatCmd;
    auto *pp = playmatCmd.mutable_playmat_params();
    pp->set_card_name(resolved.card.name.toStdString());
    pp->set_card_provider_id(resolved.card.providerId.toStdString());
    pp->set_margin_pct_l(resolved.params.marginPctL);
    pp->set_margin_pct_r(resolved.params.marginPctR);
    pp->set_vertical_offset(resolved.params.verticalOffset);
    pp->set_zoom(resolved.params.zoom);
    PendingCommand *playmatPend = parentGame->getGame()->getGameEventHandler()->prepareGameCommand(playmatCmd);
    parentGame->getGame()->getGameEventHandler()->sendGameCommand(playmatPend, playerId);
}

void DeckViewContainer::onPlaymatSettingsChanged()
{
    resolveAndSendPlaymat();
}

void DeckViewContainer::advancePlaymatRotation()
{
    playmatRotationIndex++;
}

void DeckViewContainer::loadRemoteDeck()
{
    DlgLoadRemoteDeck dlg(parentGame->getGame()->getClientForPlayer(playerId), this);
    if (dlg.exec()) {
        Command_DeckSelect cmd;
        cmd.set_deck_id(dlg.getDeckId());
        PendingCommand *pend = parentGame->getGame()->getGameEventHandler()->prepareGameCommand(cmd);
        connect(pend, &PendingCommand::finished, this, &DeckViewContainer::deckSelectFinished);
        parentGame->getGame()->getGameEventHandler()->sendGameCommand(pend, playerId);
    }
}

void DeckViewContainer::loadFromClipboard()
{
    auto dlg = DlgLoadDeckFromClipboard(this);

    if (!dlg.exec()) {
        return;
    }

    DeckList deck = dlg.getDeckList();
    loadDeckFromDeckList(deck);
}

void DeckViewContainer::loadFromWebsite()
{
    auto dlg = DlgLoadDeckFromWebsite(this);

    if (!dlg.exec()) {
        return;
    }

    DeckList deck = dlg.getDeck();
    loadDeckFromDeckList(deck);
}

void DeckViewContainer::deckSelectFinished(const Response &r)
{
    const Response_DeckDownload &resp = r.GetExtension(Response_DeckDownload::ext);
    DeckList newDeck = DeckList(QString::fromStdString(resp.deck()));
    CardPictureLoader::cacheCardPixmaps(CardDatabaseManager::query()->getCards(newDeck.getCardRefList()));
    setDeck(newDeck);
    switchToDeckLoadedView();
}

void DeckViewContainer::readyStart()
{
    sendReadyStartCommand(!readyStartButton->getState());
}

void DeckViewContainer::forceStart()
{
    const auto msg = tr("Are you sure you want to force start?\nThis will kick all non-ready players from the game.");
    const auto res =
        QMessageBox::question(this, tr("Cockatrice"), msg, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    if (res == QMessageBox::No) {
        return;
    }

    Command_ReadyStart cmd;
    cmd.set_force_start(true);
    cmd.set_ready(true);
    parentGame->getGame()->getGameEventHandler()->sendGameCommand(cmd, playerId);
}

void DeckViewContainer::sideboardLockButtonClicked()
{
    Command_SetSideboardLock cmd;
    cmd.set_locked(sideboardLockButton->getState());

    parentGame->getGame()->getGameEventHandler()->sendGameCommand(cmd, playerId);
}

void DeckViewContainer::sideboardPlanChanged()
{
    Command_SetSideboardPlan cmd;
    const QList<MoveCard_ToZone> &newPlan = deckView->getSideboardPlan();
    for (const auto &i : newPlan) {
        cmd.add_move_list()->CopyFrom(i);
    }
    parentGame->getGame()->getGameEventHandler()->sendGameCommand(cmd, playerId);
}

/**
 * Sends the basic ReadyStart command.
 */
void DeckViewContainer::sendReadyStartCommand(bool ready)
{
    if (ready) {
        resolveAndSendPlaymat();
    }

    Command_ReadyStart cmd;
    cmd.set_ready(ready);
    parentGame->getGame()->getGameEventHandler()->sendGameCommand(cmd, playerId);
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
    if (locked) {
        deckView->resetSideboardPlan();
    }
}

void DeckViewContainer::setDeck(const DeckList &deck)
{
    currentDeck = deck;
    deckView->setDeck(deck);
    switchToDeckLoadedView();
}