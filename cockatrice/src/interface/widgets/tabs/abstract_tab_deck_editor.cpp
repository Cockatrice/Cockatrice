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
#include "../client/network/interfaces/deck_stats_interface.h"
#include "../client/network/interfaces/tapped_out_interface.h"
#include "../deck_editor/deck_state_manager.h"
#include "../interface/card_picture_loader/card_picture_loader.h"
#include "../interface/pixel_map_generator.h"
#include "../interface/widgets/dialogs/dlg_load_deck.h"
#include "../interface/widgets/dialogs/dlg_load_deck_from_clipboard.h"
#include "../interface/widgets/dialogs/dlg_load_deck_from_website.h"
#include "tab_supervisor.h"

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QDesktopServices>
#include <QFileDialog>
#include <QHeaderView>
#include <QLineEdit>
#include <QMessageBox>
#include <QPrintPreviewDialog>
#include <QPrinter>
#include <QProcessEnvironment>
#include <QPushButton>
#include <QRegularExpression>
#include <QSplitter>
#include <QTreeView>
#include <QUrl>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/models/database/card_database_model.h>
#include <libcockatrice/network/client/abstract/abstract_client.h>
#include <libcockatrice/protocol/pb/command_deck_upload.pb.h>
#include <libcockatrice/protocol/pb/response.pb.h>
#include <libcockatrice/protocol/pending_command.h>
#include <libcockatrice/utility/trice_limits.h>

/**
 * @brief Constructs the AbstractTabDeckEditor.
 * Initializes all dock widgets and connects signals/slots.
 * @param _tabSupervisor The tab supervisor managing this tab.
 */
AbstractTabDeckEditor::AbstractTabDeckEditor(TabSupervisor *_tabSupervisor) : Tab(_tabSupervisor)
{
    setDockOptions(QMainWindow::AnimatedDocks | QMainWindow::AllowNestedDocks | QMainWindow::AllowTabbedDocks);

    deckStateManager = new DeckStateManager(this);

    cardDatabaseDockWidget = new DeckEditorCardDatabaseDockWidget(this);
    deckDockWidget = new DeckEditorDeckDockWidget(this);
    cardInfoDockWidget = new DeckEditorCardInfoDockWidget(this);
    filterDockWidget = new DeckEditorFilterDockWidget(this);
    printingSelectorDockWidget = new DeckEditorPrintingSelectorDockWidget(this);
    connect(&SettingsCache::instance(), &SettingsCache::overrideAllCardArtWithPersonalPreferenceChanged, this, [this] {
        printingSelectorDockWidget->setHidden(SettingsCache::instance().getOverrideAllCardArtWithPersonalPreference());
    });

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

/**
 * @brief Updates the card info dock and printing selector.
 * @param card The card to display.
 */
void AbstractTabDeckEditor::updateCard(const ExactCard &card)
{
    cardInfoDockWidget->updateCard(card);
    printingSelectorDockWidget->printingSelector->setCard(card.getCardPtr());
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

/**
 * @brief Helper for adding a card to a deck zone.
 * @param card Card to add.
 * @param zoneName Zone to add the card to.
 */
void AbstractTabDeckEditor::addCardHelper(const ExactCard &card, const QString &zoneName)
{
    deckStateManager->addCard(card, zoneName);

    cardDatabaseDockWidget->highlightAllSearchEdit();
}

/**
 * @brief Adds a card to the main deck or sideboard depending on Ctrl key.
 */
void AbstractTabDeckEditor::actAddCard(const ExactCard &card)
{
    if (QApplication::keyboardModifiers() & Qt::ControlModifier)
        actAddCardToSideboard(card);
    else
        addCardHelper(card, DECK_ZONE_MAIN);

    deckMenu->setSaveStatus(true);
}

/** @brief Adds a card to the sideboard explicitly. */
void AbstractTabDeckEditor::actAddCardToSideboard(const ExactCard &card)
{
    addCardHelper(card, DECK_ZONE_SIDE);
    deckMenu->setSaveStatus(true);
}

/** @brief Decrements a card from the main deck. */
void AbstractTabDeckEditor::actDecrementCard(const ExactCard &card)
{
    deckStateManager->decrementCard(card, DECK_ZONE_MAIN);
}

/** @brief Decrements a card from the sideboard. */
void AbstractTabDeckEditor::actDecrementCardFromSideboard(const ExactCard &card)
{
    deckStateManager->decrementCard(card, DECK_ZONE_SIDE);
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

    aDeckDockVisible->setChecked(true);
    deckDockWidget->setVisible(aDeckDockVisible->isChecked());
}

/** @brief Creates a new deck. Handles opening in new tab if needed. */
void AbstractTabDeckEditor::actNewDeck()
{
    auto deckOpenLocation = confirmOpen(false);
    if (deckOpenLocation == CANCELLED)
        return;

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
AbstractTabDeckEditor::DeckOpenLocation AbstractTabDeckEditor::confirmOpen(const bool openInSameTabIfBlank)
{
    if (SettingsCache::instance().getOpenDeckInNewTab()) {
        if (openInSameTabIfBlank && deckStateManager->isBlankNewDeck())
            return SAME_TAB;
        else
            return NEW_TAB;
    }

    if (!deckStateManager->isModified())
        return SAME_TAB;

    tabSupervisor->setCurrentWidget(this);
    QMessageBox *msgBox = createSaveConfirmationWindow();
    QPushButton *newTabButton = msgBox->addButton(tr("Open in new tab"), QMessageBox::ApplyRole);
    int ret = msgBox->exec();

    if (msgBox->clickedButton() == newTabButton)
        return NEW_TAB;

    switch (ret) {
        case QMessageBox::Save:
            return actSaveDeck() ? SAME_TAB : CANCELLED;
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
    auto deckOpenLocation = confirmOpen();
    if (deckOpenLocation == CANCELLED)
        return;

    DlgLoadDeck dialog(this);
    if (!dialog.exec())
        return;

    QString fileName = dialog.selectedFiles().at(0);
    openDeckFromFile(fileName, deckOpenLocation);
}

/**
 * @brief Opens a recently used deck file.
 * @param fileName Path to the deck file.
 */
void AbstractTabDeckEditor::actOpenRecent(const QString &fileName)
{
    auto deckOpenLocation = confirmOpen();
    if (deckOpenLocation == CANCELLED)
        return;

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

    auto l = DeckLoader(this);
    if (l.loadFromFile(fileName, fmt, true)) {
        if (deckOpenLocation == NEW_TAB) {
            emit openDeckEditor(l.getDeck());
        } else {
            deckMenu->setSaveStatus(false);
            openDeck(l.getDeck());
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
    const auto loadedDeck = deckStateManager->toLoadedDeck();
    if (loadedDeck.lastLoadInfo.remoteDeckId != LoadedDeck::LoadInfo::NON_REMOTE_ID) {
        QString deckString = loadedDeck.deckList.writeToString_Native();
        if (deckString.length() > MAX_FILE_LENGTH) {
            QMessageBox::critical(this, tr("Error"), tr("Could not save remote deck"));
            return false;
        }

        Command_DeckUpload cmd;
        cmd.set_deck_id(static_cast<google::protobuf::uint32>(loadedDeck.lastLoadInfo.remoteDeckId));
        cmd.set_deck_list(deckString.toStdString());

        PendingCommand *pend = AbstractClient::prepareSessionCommand(cmd);
        connect(pend, &PendingCommand::finished, this, &AbstractTabDeckEditor::saveDeckRemoteFinished);
        tabSupervisor->getClient()->sendCommand(pend);

        return true;
    }
    if (loadedDeck.lastLoadInfo.fileName.isEmpty())
        return actSaveDeckAs();

    auto deckLoader = DeckLoader(this);
    deckLoader.setDeck(loadedDeck);
    if (deckLoader.saveToFile(loadedDeck.lastLoadInfo.fileName, loadedDeck.lastLoadInfo.fileFormat)) {
        deckStateManager->setModified(false);
        return true;
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
    LoadedDeck loadedDeck = deckStateManager->toLoadedDeck();

    QFileDialog dialog(this, tr("Save deck"));
    dialog.setDirectory(SettingsCache::instance().getDeckPath());
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setDefaultSuffix("cod");
    dialog.setNameFilters(DeckLoader::FILE_NAME_FILTERS);
    dialog.selectFile(loadedDeck.deckList.getName().trimmed());

    if (!dialog.exec())
        return false;

    QString fileName = dialog.selectedFiles().at(0);
    DeckFileFormat::Format fmt = DeckFileFormat::getFormatFromName(fileName);

    DeckLoader deckLoader = DeckLoader(this);
    deckLoader.setDeck(loadedDeck);
    if (!deckLoader.saveToFile(fileName, fmt)) {
        QMessageBox::critical(
            this, tr("Error"),
            tr("The deck could not be saved.\nPlease check that the directory is writable and try again."));
        return false;
    }

    deckStateManager->setLastLoadInfo({.fileName = fileName, .fileFormat = fmt});

    deckStateManager->setModified(false);
    SettingsCache::instance().recents().updateRecentlyOpenedDeckPaths(fileName);
    return true;
}

/**
 * @brief Callback for remote deck save completion.
 * @param response Server response.
 */
void AbstractTabDeckEditor::saveDeckRemoteFinished(const Response &response)
{
    if (response.response_code() != Response::RespOk)
        QMessageBox::critical(this, tr("Error"), tr("The deck could not be saved."));
    else
        deckStateManager->setModified(false);
}

/**
 * @brief Loads a deck from clipboard.
 * Displays confirmation if the tab is modified.
 */
void AbstractTabDeckEditor::actLoadDeckFromClipboard()
{
    auto deckOpenLocation = confirmOpen();
    if (deckOpenLocation == CANCELLED)
        return;

    DlgLoadDeckFromClipboard dlg(this);
    if (!dlg.exec())
        return;

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
    if (!dlg.exec())
        return;

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
    auto deckOpenLocation = confirmOpen();
    if (deckOpenLocation == CANCELLED)
        return;

    DlgLoadDeckFromWebsite dlg(this);
    if (!dlg.exec())
        return;

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
    auto *interface = new DeckStatsInterface(*cardDatabaseDockWidget->getDatabase(), this);
    interface->analyzeDeck(deckStateManager->getDeckList());
}

/** @brief Analyzes the deck using TappedOut. */
void AbstractTabDeckEditor::actAnalyzeDeckTappedout()
{
    auto *interface = new TappedOutInterface(*cardDatabaseDockWidget->getDatabase(), this);
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
    if (deckStateManager->isModified()) {
        tabSupervisor->setCurrentWidget(this);
        int ret = createSaveConfirmationWindow()->exec();
        if (ret == QMessageBox::Save)
            return actSaveDeck();
        else if (ret == QMessageBox::Cancel)
            return false;
    }
    return true;
}

/** @brief Handles close requests from outside (tab manager). */
bool AbstractTabDeckEditor::closeRequest()
{
    if (!confirmClose())
        return false;
    return close();
}
