/**
 * @file abstract_tab_deck_editor.cpp
 * @brief Implementation of the AbstractTabDeckEditor class.
 *
 * Handles deck editor tab UI, deck management, card operations, clipboard
 * operations, printing, deck export, remote uploads, and interactions with
 * external services (DeckStats, TappedOut, etc.).
 */

#include "abstract_tab_deck_editor.h"

#include "../../../client/settings/cache_settings.h"
#include "../../../client/settings/shortcuts_settings.h"
#include "../../deck_loader/deck_loader.h"
#include "../cards/additional_info/deck_color_identity.h"
#include "../cards/card_info_frame_widget.h"
#include "../client/network/interfaces/deck_stats_interface.h"
#include "../client/network/interfaces/tapped_out_interface.h"
#include "../deck_editor/deck_editor_card_database_dock_widget.h"
#include "../deck_editor/deck_editor_card_info_dock_widget.h"
#include "../deck_editor/deck_editor_deck_dock_widget.h"
#include "../deck_editor/deck_editor_filter_dock_widget.h"
#include "../deck_editor/deck_editor_printing_selector_dock_widget.h"
#include "../deck_editor/deck_state_manager.h"
#include "../interface/card_picture_loader/card_picture_loader.h"
#include "../interface/widgets/dialogs/dlg_load_deck.h"
#include "../interface/widgets/dialogs/dlg_load_deck_from_clipboard.h"
#include "../interface/widgets/dialogs/dlg_load_deck_from_website.h"
#include "../interface/widgets/dialogs/dlg_share_deck.h"
#include "../menus/deck_editor_menu.h"
#include "../printing_selector/printing_selector.h"
#include "../utility/visibility_change_listener.h"
#include "libcockatrice/card/database/card_database_querier.h"
#include "libcockatrice/card/printing/exact_card.h"
#include "libcockatrice/deck_list/deck_list.h"
#include "libcockatrice/utility/card_ref.h"
#include "tab_supervisor.h"

#include <QAction>
#include <QCloseEvent>
#include <QDesktopServices>
#include <QDockWidget>
#include <QFileDialog>
#include <QFlags>
#include <QList>
#include <QMainWindow>
#include <QMenu>
#include <QMessageBox>
#include <QPointer>
#include <QPrintPreviewDialog>
#include <QPrinter>
#include <QPushButton>
#include <QSharedPointer>
#include <QTimer>
#include <QUrl>
#include <google/protobuf/stubs/port.h>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/deck_list/deck_file_format.h>
#include <libcockatrice/deck_list/loaded_deck.h>
#include <libcockatrice/models/database/card_database_model.h>
#include <libcockatrice/network/client/abstract/abstract_client.h>
#include <libcockatrice/protocol/pb/command_deck_upload.pb.h>
#include <libcockatrice/protocol/pb/response.pb.h>
#include <libcockatrice/protocol/pending_command.h>
#include <libcockatrice/settings/deck_editor_settings.h>
#include <libcockatrice/settings/paths_settings.h>
#include <libcockatrice/settings/recents_settings.h>
#include <libcockatrice/utility/string_limits.h>
#include <optional>
#include <string>

/**
 * @brief Constructs the AbstractTabDeckEditor.
 * Initializes all dock widgets and connects signals/slots.
 * @param _tabSupervisor The tab supervisor managing this tab.
 */
AbstractTabDeckEditor::AbstractTabDeckEditor(TabSupervisor *_tabSupervisor) : Tab(_tabSupervisor)
{
    setDockOptions(QMainWindow::AnimatedDocks | QMainWindow::AllowNestedDocks | QMainWindow::AllowTabbedDocks);

    deckStateManager = new DeckStateManager(this);

    databaseModel = new CardDatabaseModel(CardDatabaseManager::getInstance(), true, this);
    databaseModel->setObjectName("databaseModel");

    cardDatabaseDockWidget = new DeckEditorCardDatabaseDockWidget(this);
    deckDockWidget = new DeckEditorDeckDockWidget(this);
    cardInfoDockWidget = new DeckEditorCardInfoDockWidget(this);
    filterDockWidget = new DeckEditorFilterDockWidget(this);
    printingSelectorDockWidget = new DeckEditorPrintingSelectorDockWidget(this);

    // A save acknowledgment only belongs to the revision that was uploaded. Replacements
    // include undo/redo, so returning to identical text does not revive an old leave intent.
    connect(deckStateManager, &DeckStateManager::deckModified, this, [this] { ++deckRevision; });
    connect(deckStateManager, &DeckStateManager::deckReplaced, this, [this] { ++deckRevision; });

    // Connect deck signals to this tab
    connect(deckStateManager, &DeckStateManager::isModifiedChanged, this, &AbstractTabDeckEditor::onDeckModified);
    connect(deckDockWidget, &DeckEditorDeckDockWidget::selectedCardChanged, this, &AbstractTabDeckEditor::updateCard);

    // Connect filter signals
    connect(filterDockWidget, &DeckEditorFilterDockWidget::clearAllDatabaseFilters, cardDatabaseDockWidget,
            &DeckEditorCardDatabaseDockWidget::clearAllDatabaseFilters);

    // Connect shortcut changes
    connect(&SettingsCache::instance().shortcuts(), &ShortcutsSettings::shortCutChanged, this,
            &AbstractTabDeckEditor::refreshShortcuts);
}

void AbstractTabDeckEditor::registerDockWidget(QMenu *_viewMenu, QDockWidget *widget, const QSize &defaultSize)
{
    QMenu *menu = _viewMenu->addMenu(QString());

    QAction *aVisible = menu->addAction(QString());
    aVisible->setCheckable(true);

    QAction *aFloating = menu->addAction(QString());
    aFloating->setCheckable(true);
    aFloating->setEnabled(false);

    // user interaction
    connect(aVisible, &QAction::triggered, widget, [widget](bool checked) { widget->setVisible(checked); });
    connect(aFloating, &QAction::triggered, this, [widget](bool checked) { widget->setFloating(checked); });

    // sync aFloating's enabled state with aVisible's checked state
    connect(aVisible, &QAction::toggled, aFloating, [aFloating](bool checked) { aFloating->setEnabled(checked); });

    // sync aFloating with dockWidget's floating state
    connect(widget, &QDockWidget::topLevelChanged, aFloating,
            [aFloating](bool topLevel) { aFloating->setChecked(topLevel); });

    // sync aVisible with dockWidget's visible state
    auto filter = new VisibilityChangeListener(widget);
    connect(filter, &VisibilityChangeListener::visibilityChanged, aVisible,
            [aVisible](bool visible) { aVisible->setChecked(visible); });

    dockToActions.insert(widget, {menu, aVisible, aFloating, defaultSize});
}

void AbstractTabDeckEditor::updateCard(const ExactCard &card)
{
    cardInfoDockWidget->updateCard(card);
    printingSelectorDockWidget->printingSelector->setCard(card.getCardPtr());
}

void AbstractTabDeckEditor::updateCardInfo(const ExactCard &card)
{
    cardInfoDockWidget->updateCard(card);
}

/** @brief Placeholder: called when the deck changes. */
void AbstractTabDeckEditor::onDeckChanged()
{
}

/**
 * @brief Marks the tab as modified and updates the save menu status.
 */
void AbstractTabDeckEditor::onDeckModified()
{
    deckMenu->setSaveStatus(!deckStateManager->isBlankNewDeck());
    emit tabTextChanged(this, getTabText());
}

void AbstractTabDeckEditor::addCard(const ExactCard &card, const QString &zoneName)
{
    deckStateManager->addCard(card, zoneName);
}

void AbstractTabDeckEditor::decrementCard(const ExactCard &card, const QString &zoneName)
{
    deckStateManager->decrementCard(card, zoneName);
}

/**
 * @brief Opens a deck in this tab.
 * @param deck The deck
 */
void AbstractTabDeckEditor::openDeck(const LoadedDeck &deck)
{
    setDeck(deck);

    if (!deck.lastLoadInfo.fileName.isEmpty()) {
        SettingsCache::instance().recents().updateRecentlyOpenedDeckPaths(deck.lastLoadInfo.fileName);
    }
}

/**
 * @brief Sets the currently active deck.
 * @param _deck The deck
 */
void AbstractTabDeckEditor::setDeck(const LoadedDeck &_deck)
{
    deckStateManager->replaceDeck(_deck);
    CardPictureLoader::cacheCardPixmaps(CardDatabaseManager::query()->getCards(_deck.deckList.getCardRefList()));

    dockToActions.value(deckDockWidget).aVisible->setChecked(true);
    deckDockWidget->setVisible(dockToActions.value(deckDockWidget).aVisible->isChecked());
}

/** @brief Creates a new deck. Handles opening in new tab if needed. */
void AbstractTabDeckEditor::actNewDeck()
{
    auto deckOpenLocation = confirmOpen(false, [this] { actNewDeck(); });
    if (deckOpenLocation == CANCELLED) {
        return;
    }

    if (deckOpenLocation == NEW_TAB) {
        emit openDeckEditor(LoadedDeck());
        return;
    }

    cleanDeckAndResetModified();
}

/** @brief Clears the current deck and resets modified flag. */
void AbstractTabDeckEditor::cleanDeckAndResetModified()
{
    deckStateManager->clearDeck();
    deckMenu->setSaveStatus(false);
}

/**
 * @brief Displays the save confirmation dialog before loading a deck.
 * @param openInSameTabIfBlank Open in same tab if current tab is blank.
 * @return DeckOpenLocation enum indicating where to open the deck.
 */
AbstractTabDeckEditor::DeckOpenLocation AbstractTabDeckEditor::confirmOpen(const bool openInSameTabIfBlank,
                                                                         std::function<void()> afterSave)
{
    cancelPendingLeave();
    if (SettingsCache::instance().deckEditor().getOpenDeckInNewTab()) {
        if (openInSameTabIfBlank && deckStateManager->isBlankNewDeck()) {
            return SAME_TAB;
        } else {
            return NEW_TAB;
        }
    }

    if (!deckStateManager->isModified()) {
        return SAME_TAB;
    }

    tabSupervisor->setCurrentWidget(this);
    QMessageBox *msgBox = createSaveConfirmationWindow();
    QPushButton *newTabButton = msgBox->addButton(tr("Open in new tab"), QMessageBox::ApplyRole);
    QPointer<AbstractTabDeckEditor> guard(this);
    const quint64 revision = deckRevision;
    const quint64 intent = leaveIntent;
    int ret = msgBox->exec();
    if (!guard) {
        return CANCELLED;
    }
    const bool openNewTab = msgBox->clickedButton() == newTabButton;
    msgBox->deleteLater();
    if (revision != deckRevision || intent != leaveIntent) {
        return CANCELLED;
    }

    if (openNewTab) {
        return NEW_TAB;
    }

    switch (ret) {
        case QMessageBox::Save:
            return saveDeck(std::move(afterSave)) ? SAME_TAB : CANCELLED;
        case QMessageBox::Discard:
            return SAME_TAB;
        default:
            return CANCELLED;
    }
}

/**
 * @brief Creates the base save confirmation dialog.
 * @return Pointer to a QMessageBox.
 */
QMessageBox *AbstractTabDeckEditor::createSaveConfirmationWindow()
{
    QMessageBox *msgBox = new QMessageBox(this);
    msgBox->setIcon(QMessageBox::Warning);
    msgBox->setWindowTitle(tr("Are you sure?"));
    msgBox->setText(tr("The decklist has been modified.\nDo you want to save the changes?"));
    msgBox->setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    return msgBox;
}

/**
 * @brief Loads a deck from file using a dialog.
 * Displays a save confirmation if needed.
 */
void AbstractTabDeckEditor::actLoadDeck()
{
    auto deckOpenLocation = confirmOpen(true, [this] { actLoadDeck(); });
    if (deckOpenLocation == CANCELLED) {
        return;
    }

    DlgLoadDeck dialog(this);
    if (!dialog.exec()) {
        return;
    }

    QString fileName = dialog.selectedFiles().at(0);
    openDeckFromFile(fileName, deckOpenLocation);
}

/**
 * @brief Opens a recently used deck file.
 * @param fileName Path to the deck file.
 */
void AbstractTabDeckEditor::actOpenRecent(const QString &fileName)
{
    auto deckOpenLocation = confirmOpen(true, [this, fileName] { actOpenRecent(fileName); });
    if (deckOpenLocation == CANCELLED) {
        return;
    }

    openDeckFromFile(fileName, deckOpenLocation);
}

/**
 * @brief Actually opens a deck from file.
 * @param fileName Path to the deck file.
 * @param deckOpenLocation Where to open the deck (same or new tab).
 */
void AbstractTabDeckEditor::openDeckFromFile(const QString &fileName, DeckOpenLocation deckOpenLocation)
{
    DeckFileFormat::Format fmt = DeckFileFormat::getFormatFromName(fileName);

    std::optional<LoadedDeck> deckOpt = DeckLoader::loadFromFile(fileName, fmt, true);
    if (deckOpt) {
        if (deckOpenLocation == NEW_TAB) {
            emit openDeckEditor(deckOpt.value());
        } else {
            deckMenu->setSaveStatus(false);
            openDeck(deckOpt.value());
        }
    } else {
        QMessageBox::critical(this, tr("Error"), tr("Could not open deck at %1").arg(fileName));
    }
    deckMenu->setSaveStatus(true);
}

/**
 * @brief Saves the current deck.
 * If remote deck, sends upload command.
 * @return True if save succeeded, false otherwise.
 */
bool AbstractTabDeckEditor::actSaveDeck()
{
    return saveDeck();
}

bool AbstractTabDeckEditor::saveDeck(std::function<void()> afterSave)
{
    QPointer<AbstractTabDeckEditor> guard(this);
    const quint64 revision = deckRevision;
    const quint64 intent = leaveIntent;
    const auto loadedDeck = deckStateManager->toLoadedDeck();
    if (loadedDeck.lastLoadInfo.remoteDeckId != LoadedDeck::LoadInfo::NON_REMOTE_ID) {
        QString deckString = loadedDeck.deckList.writeToString_Native();
        if (deckString.length() > MAX_FILE_LENGTH) {
            QMessageBox::critical(this, tr("Error"), tr("Could not save remote deck"));
            return false;
        }

        RemoteSave request{deckRevision, loadedDeck.lastLoadInfo, deckString,
                           getDeckColorIdentity(loadedDeck.deckList, CardDatabaseManager::query()), std::move(afterSave)};
        if (activeRemoteSave) {
            // Coalesce identical saves and serialize newer snapshots. Only the newest
            // explicitly requested snapshot needs another upload.
            if (matchesCurrentDeck(*activeRemoteSave)) {
                if (request.afterSave) {
                    activeRemoteSave->afterSave = std::move(request.afterSave);
                }
            } else if (queuedRemoteSave && matchesCurrentDeck(*queuedRemoteSave)) {
                if (request.afterSave) {
                    queuedRemoteSave->afterSave = std::move(request.afterSave);
                }
            } else {
                queuedRemoteSave = std::move(request);
            }
        } else {
            startRemoteSave(std::move(request));
        }
        // Enqueueing a command is not a successful save. The matching acknowledgment
        // will resume an explicitly requested open/close operation, if still current.
        return false;
    }
    if (loadedDeck.lastLoadInfo.fileName.isEmpty()) {
        return actSaveDeckAs();
    }

    const bool saved = DeckLoader::saveToFile(loadedDeck);
    if (!guard) {
        return false;
    }
    if (saved) {
        if (intent != leaveIntent || !matchesCurrentDeck(revision, loadedDeck.lastLoadInfo)) {
            return false;
        }
        deckStateManager->setModified(false);
        return guard && intent == leaveIntent && matchesCurrentDeck(revision, loadedDeck.lastLoadInfo) &&
               !deckStateManager->isModified();
    }

    QMessageBox::critical(
        this, tr("Error"),
        tr("The deck could not be saved.\nPlease check that the directory is writable and try again."));
    return false;
}

/**
 * @brief Saves the deck to a user-selected file.
 * @return True if save succeeded.
 */
bool AbstractTabDeckEditor::actSaveDeckAs()
{
    cancelPendingLeave();
    const quint64 revision = deckRevision;
    const quint64 intent = leaveIntent;
    const auto loadedDeck = deckStateManager->toLoadedDeck();
    QPointer<AbstractTabDeckEditor> guard(this);
    QPointer<QFileDialog> dialog = new QFileDialog(this, tr("Save deck"));
    dialog->setDirectory(SettingsCache::instance().paths().getDeckPath());
    dialog->setAcceptMode(QFileDialog::AcceptSave);
    dialog->setDefaultSuffix("cod");
    dialog->setNameFilters(DeckLoader::FILE_NAME_FILTERS);
    dialog->selectFile(loadedDeck.deckList.getName().trimmed());

    const int result = dialog->exec();
    if (!guard || !dialog) {
        return false;
    }
    const QStringList selectedFiles = dialog->selectedFiles();
    dialog->deleteLater();
    if (result != QDialog::Accepted || selectedFiles.isEmpty()) {
        return false;
    }
    const QString fileName = selectedFiles.first();
    if (intent != leaveIntent || !matchesCurrentDeck(revision, loadedDeck.lastLoadInfo)) {
        auto *message = new QMessageBox(QMessageBox::Warning, tr("Deck changed"),
                                       tr("The deck changed while choosing a save location. Nothing was written to %1. "
                                          "Save the current deck again.").arg(fileName),
                                       QMessageBox::Ok, this);
        message->setAttribute(Qt::WA_DeleteOnClose);
        message->open();
        return false;
    }

    DeckFileFormat::Format fmt = DeckFileFormat::getFormatFromName(fileName);
    std::optional<LoadedDeck::LoadInfo> infoOpt = DeckLoader::saveToFile(loadedDeck.deckList, fileName, fmt);
    if (!guard) {
        return false;
    }
    if (!infoOpt) {
        QMessageBox::critical(
            this, tr("Error"),
            tr("The deck could not be saved.\nPlease check that the directory is writable and try again."));
        return false;
    }

    if (!guard || intent != leaveIntent || !matchesCurrentDeck(revision, loadedDeck.lastLoadInfo)) {
        return false;
    }
    deckStateManager->setLastLoadInfo(infoOpt.value());
    deckStateManager->setModified(false);
    SettingsCache::instance().recents().updateRecentlyOpenedDeckPaths(fileName);
    return guard && intent == leaveIntent && matchesCurrentDeck(revision, infoOpt.value()) &&
           !deckStateManager->isModified();
}

/**
 * @brief Opens the deck share dialog with the current deck preselected.
 */
void AbstractTabDeckEditor::actShareDeck()
{
    AbstractClient *client = tabSupervisor->getServerClient();
    if (client->getStatus() != StatusLoggedIn) {
        QMessageBox::information(this, tr("Share deck"), tr("You must be connected to the server to share a deck."));
        return;
    }

    const QSharedPointer<DeckList> deck = deckStateManager->getDeckListShared();
    if (deck->isBlankDeck()) {
        QMessageBox::information(this, tr("Share deck"), tr("The deck is empty. Add cards before sharing it."));
        return;
    }

    DlgShareDeck shareDialog(client, deck, this);
    shareDialog.exec();
}

bool AbstractTabDeckEditor::matchesCurrentDeck(const RemoteSave &request) const
{
    return matchesCurrentDeck(request.revision, request.destination);
}

bool AbstractTabDeckEditor::matchesCurrentDeck(quint64 revision, const LoadedDeck::LoadInfo &destination) const
{
    const auto current = deckStateManager->toLoadedDeck().lastLoadInfo;
    return revision == deckRevision && destination.remoteDeckId == current.remoteDeckId &&
           destination.fileName == current.fileName && destination.fileFormat == current.fileFormat;
}

void AbstractTabDeckEditor::cancelPendingLeave()
{
    ++leaveIntent;
    if (activeRemoteSave) {
        activeRemoteSave->afterSave = {};
    }
    if (queuedRemoteSave) {
        queuedRemoteSave->afterSave = {};
    }
}

void AbstractTabDeckEditor::startRemoteSave(RemoteSave request)
{
    const quint64 requestId = ++remoteSaveId;
    activeRemoteSave = std::move(request);
    AbstractClient *client = tabSupervisor->getServerClient();
    if (!client || client->getStatus() != StatusLoggedIn) {
        finishRemoteSave(requestId, false);
        return;
    }

    Command_DeckUpload cmd;
    cmd.set_deck_id(static_cast<google::protobuf::uint32>(activeRemoteSave->destination.remoteDeckId));
    cmd.set_deck_list(activeRemoteSave->deckString.toStdString());
    cmd.set_color_identity(activeRemoteSave->colorIdentity.toStdString());
    PendingCommand *pending = AbstractClient::prepareSessionCommand(cmd);
    connect(pending, &PendingCommand::finished, this, [this, requestId](const Response &response) {
        finishRemoteSave(requestId, response.response_code() == Response::RespOk);
    });
    // A discarded command must release the queue too. A normal response consumes its
    // request first, so the following destruction notification cannot finish it twice.
    connect(pending, &QObject::destroyed, this, [this, requestId] { finishRemoteSave(requestId, false); });
    client->sendCommand(pending);
}

void AbstractTabDeckEditor::finishRemoteSave(quint64 requestId, bool succeeded)
{
    if (!activeRemoteSave || requestId != remoteSaveId) {
        return;
    }
    RemoteSave completed = std::move(*activeRemoteSave);
    activeRemoteSave.reset();
    auto next = std::move(queuedRemoteSave);
    queuedRemoteSave.reset();

    if (!succeeded) {
        auto *message = new QMessageBox(QMessageBox::Critical, tr("Error"),
                                       tr("The remote deck could not be saved. Please check your connection and try again."),
                                       QMessageBox::Ok, this);
        message->setAttribute(Qt::WA_DeleteOnClose);
        message->open();
        return;
    }

    QPointer<AbstractTabDeckEditor> guard(this);
    const quint64 intent = leaveIntent;
    if (matchesCurrentDeck(completed)) {
        deckStateManager->setModified(false);
        if (!guard) {
            return;
        }
        if (completed.afterSave) {
            // Leave the command callback before closing the tab or opening a modal
            // dialog. Check again after the queued turn and after modified listeners.
            QTimer::singleShot(0, this, [this, intent, completed = std::move(completed)] {
                if (intent == leaveIntent && matchesCurrentDeck(completed) && !deckStateManager->isModified()) {
                    completed.afterSave();
                }
            });
        }
    }
    if (next && !activeRemoteSave) {
        startRemoteSave(std::move(*next));
    }
}

/**
 * @brief Loads a deck from clipboard.
 * Displays confirmation if the tab is modified.
 */
void AbstractTabDeckEditor::actLoadDeckFromClipboard()
{
    auto deckOpenLocation = confirmOpen(true, [this] { actLoadDeckFromClipboard(); });
    if (deckOpenLocation == CANCELLED) {
        return;
    }

    DlgLoadDeckFromClipboard dlg(this);
    if (!dlg.exec()) {
        return;
    }

    if (deckOpenLocation == NEW_TAB) {
        emit openDeckEditor({.deckList = dlg.getDeckList()});
    } else {
        setDeck({.deckList = dlg.getDeckList()});
        deckStateManager->setModified(true);
    }

    deckMenu->setSaveStatus(true);
}

/**
 * @brief Opens the deck editor to edit clipboard contents.
 * @param annotated If true, edits with annotations.
 */
void AbstractTabDeckEditor::editDeckInClipboard(bool annotated)
{
    LoadedDeck loadedDeck = deckStateManager->toLoadedDeck();
    DlgEditDeckInClipboard dlg(loadedDeck.deckList, annotated, this);
    if (!dlg.exec()) {
        return;
    }

    setDeck({dlg.getDeckList(), loadedDeck.lastLoadInfo});
    deckStateManager->setModified(true);
    deckMenu->setSaveStatus(true);
}

/** @brief Slot for editing deck in clipboard with annotations. */
void AbstractTabDeckEditor::actEditDeckInClipboard()
{
    editDeckInClipboard(true);
}

/** @brief Slot for editing deck in clipboard without annotations. */
void AbstractTabDeckEditor::actEditDeckInClipboardRaw()
{
    editDeckInClipboard(false);
}

/** @brief Saves deck to clipboard with set info and annotation. */
void AbstractTabDeckEditor::actSaveDeckToClipboard()
{
    DeckLoader::saveToClipboard(deckStateManager->getDeckList(), true, true);
}

/** @brief Saves deck to clipboard with annotation, without set info. */
void AbstractTabDeckEditor::actSaveDeckToClipboardNoSetInfo()
{
    DeckLoader::saveToClipboard(deckStateManager->getDeckList(), true, false);
}

/** @brief Saves deck to clipboard without annotations, with set info. */
void AbstractTabDeckEditor::actSaveDeckToClipboardRaw()
{
    DeckLoader::saveToClipboard(deckStateManager->getDeckList(), false, true);
}

/** @brief Saves deck to clipboard without annotations or set info. */
void AbstractTabDeckEditor::actSaveDeckToClipboardRawNoSetInfo()
{
    DeckLoader::saveToClipboard(deckStateManager->getDeckList(), false, false);
}

/** @brief Prints the deck using a QPrintPreviewDialog. */
void AbstractTabDeckEditor::actPrintDeck()
{
    auto *dlg = new QPrintPreviewDialog(this);
    connect(dlg, &QPrintPreviewDialog::paintRequested, this,
            [this](QPrinter *printer) { DeckLoader::printDeckList(printer, deckStateManager->getDeckList()); });
    dlg->exec();
}

/**
 * @brief Loads a deck from a website.
 */
void AbstractTabDeckEditor::actLoadDeckFromWebsite()
{
    auto deckOpenLocation = confirmOpen(true, [this] { actLoadDeckFromWebsite(); });
    if (deckOpenLocation == CANCELLED) {
        return;
    }

    DlgLoadDeckFromWebsite dlg(this);
    if (!dlg.exec()) {
        return;
    }

    if (deckOpenLocation == NEW_TAB) {
        emit openDeckEditor({.deckList = dlg.getDeck()});
    } else {
        setDeck({.deckList = dlg.getDeck()});
        deckStateManager->setModified(true);
    }

    deckMenu->setSaveStatus(true);
}

/**
 * @brief Exports the deck to a decklist website.
 * @param website Website to export to.
 */
void AbstractTabDeckEditor::exportToDecklistWebsite(DeckLoader::DecklistWebsite website)
{
    QString decklistUrlString = DeckLoader::exportDeckToDecklist(deckStateManager->getDeckList(), website);
    // Check to make sure the string isn't empty.
    if (decklistUrlString.isEmpty()) {
        // Show an error if the deck is empty, and return.
        QMessageBox::critical(this, tr("Error"), tr("There are no cards in your deck to be exported"));
        return;
    }

    // Encode the string recieved from the model to make sure all characters are encoded.
    // first we put it into a qurl object
    QUrl decklistUrl = QUrl(decklistUrlString);
    // we get the correctly encoded url.
    decklistUrlString = decklistUrl.toEncoded();
    // We open the url in the user's default browser
    QDesktopServices::openUrl(decklistUrlString);
}

/** @brief Exports deck to www.decklist.org. */
void AbstractTabDeckEditor::actExportDeckDecklist()
{
    exportToDecklistWebsite(DeckLoader::DecklistOrg);
}

/** @brief Exports deck to www.decklist.xyz. */
void AbstractTabDeckEditor::actExportDeckDecklistXyz()
{
    exportToDecklistWebsite(DeckLoader::DecklistXyz);
}

/** @brief Analyzes the deck using DeckStats. */
void AbstractTabDeckEditor::actAnalyzeDeckDeckstats()
{
    auto *interface = new DeckStatsInterface(this);
    interface->analyzeDeck(deckStateManager->getDeckList());
}

/** @brief Analyzes the deck using TappedOut. */
void AbstractTabDeckEditor::actAnalyzeDeckTappedout()
{
    auto *interface = new TappedOutInterface(this);
    interface->analyzeDeck(deckStateManager->getDeckList());
}

/** @brief Applies a new filter tree to the database display. */
void AbstractTabDeckEditor::filterTreeChanged(FilterTree *filterTree)
{
    cardDatabaseDockWidget->setFilterTree(filterTree);
}

/**
 * @brief Handles the close event of the tab.
 * @param event Close event.
 */
void AbstractTabDeckEditor::closeEvent(QCloseEvent *event)
{
    emit deckEditorClosing(this);
    event->accept();
}

/** @brief Shows a confirmation dialog before closing. */
bool AbstractTabDeckEditor::confirmClose()
{
    return confirmCloseWithContinuation({});
}

bool AbstractTabDeckEditor::confirmCloseWithContinuation(std::function<void()> afterSave)
{
    cancelPendingLeave();
    if (deckStateManager->isModified()) {
        tabSupervisor->setCurrentWidget(this);
        QPointer<AbstractTabDeckEditor> guard(this);
        const quint64 revision = deckRevision;
        const quint64 intent = leaveIntent;
        auto *message = createSaveConfirmationWindow();
        int ret = message->exec();
        if (!guard) {
            return false;
        }
        message->deleteLater();
        if (revision != deckRevision || intent != leaveIntent) {
            return false;
        }
        if (ret == QMessageBox::Save) {
            return saveDeck(std::move(afterSave));
        } else if (ret != QMessageBox::Discard) {
            return false;
        }
    }
    return true;
}

/** @brief Handles close requests from outside (tab manager). */
bool AbstractTabDeckEditor::closeRequest()
{
    if (!confirmCloseWithContinuation([this] { closeRequest(); })) {
        return false;
    }
    return close();
}

void AbstractTabDeckEditor::showPrintingSelector()
{
    printingSelectorDockWidget->printingSelector->setCard(cardInfoDockWidget->cardInfo->getCard().getCardPtr());
    printingSelectorDockWidget->printingSelector->updateDisplay();
    printingSelectorDockWidget->setVisible(true);
}

void AbstractTabDeckEditor::openEdhrecTab(const CardInfoPtr &info, bool isCommander)
{
    getTabSupervisor()->addEdhrecTab(info, isCommander);
}
