#include "abstract_tab_deck_editor.h"

#include "../../client/game_logic/abstract_client.h"
#include "../../client/tapped_out_interface.h"
#include "../../client/ui/widgets/cards/card_info_frame_widget.h"
#include "../../deck/deck_stats_interface.h"
#include "../../dialogs/dlg_load_deck.h"
#include "../../dialogs/dlg_load_deck_from_clipboard.h"
#include "../../dialogs/dlg_load_deck_from_website.h"
#include "../../game/cards/card_database_manager.h"
#include "../../game/cards/card_database_model.h"
#include "../../server/pending_command.h"
#include "../../settings/cache_settings.h"
#include "../ui/picture_loader/picture_loader.h"
#include "../ui/pixel_map_generator.h"
#include "pb/command_deck_upload.pb.h"
#include "pb/response.pb.h"
#include "tab_supervisor.h"
#include "trice_limits.h"

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QCloseEvent>
#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>
#include <QHeaderView>
#include <QLineEdit>
#include <QMenuBar>
#include <QMessageBox>
#include <QPrintPreviewDialog>
#include <QPrinter>
#include <QProcessEnvironment>
#include <QPushButton>
#include <QRegularExpression>
#include <QSplitter>
#include <QTextStream>
#include <QTreeView>
#include <QUrl>

AbstractTabDeckEditor::AbstractTabDeckEditor(TabSupervisor *_tabSupervisor) : Tab(_tabSupervisor)
{
    setDockOptions(QMainWindow::AnimatedDocks | QMainWindow::AllowNestedDocks | QMainWindow::AllowTabbedDocks);

    databaseDisplayDockWidget = new DeckEditorDatabaseDisplayWidget(this);
    deckDockWidget = new DeckEditorDeckDockWidget(this);
    cardInfoDockWidget = new DeckEditorCardInfoDockWidget(this);
    filterDockWidget = new DeckEditorFilterDockWidget(this);
    printingSelectorDockWidget = new DeckEditorPrintingSelectorDockWidget(this);

    connect(deckDockWidget, &DeckEditorDeckDockWidget::deckChanged, this, &AbstractTabDeckEditor::onDeckChanged);
    connect(deckDockWidget, &DeckEditorDeckDockWidget::deckModified, this, &AbstractTabDeckEditor::onDeckModified);
    connect(deckDockWidget, &DeckEditorDeckDockWidget::cardChanged, this, &AbstractTabDeckEditor::updateCard);
    connect(this, &AbstractTabDeckEditor::decrementCard, deckDockWidget, &DeckEditorDeckDockWidget::actDecrementCard);
    connect(databaseDisplayDockWidget, &DeckEditorDatabaseDisplayWidget::cardChanged, this,
            &AbstractTabDeckEditor::updateCard);
    connect(databaseDisplayDockWidget, &DeckEditorDatabaseDisplayWidget::addCardToMainDeck, this,
            &AbstractTabDeckEditor::actAddCard);
    connect(databaseDisplayDockWidget, &DeckEditorDatabaseDisplayWidget::addCardToSideboard, this,
            &AbstractTabDeckEditor::actAddCardToSideboard);
    connect(databaseDisplayDockWidget, &DeckEditorDatabaseDisplayWidget::decrementCardFromMainDeck, this,
            &AbstractTabDeckEditor::actDecrementCard);
    connect(databaseDisplayDockWidget, &DeckEditorDatabaseDisplayWidget::decrementCardFromSideboard, this,
            &AbstractTabDeckEditor::actDecrementCardFromSideboard);

    connect(filterDockWidget, &DeckEditorFilterDockWidget::clearAllDatabaseFilters, databaseDisplayDockWidget,
            &DeckEditorDatabaseDisplayWidget::clearAllDatabaseFilters);

    connect(&SettingsCache::instance().shortcuts(), &ShortcutsSettings::shortCutChanged, this,
            &AbstractTabDeckEditor::refreshShortcuts);
}

void AbstractTabDeckEditor::updateCard(const ExactCard &card)
{
    cardInfoDockWidget->updateCard(card);
    printingSelectorDockWidget->printingSelector->setCard(card.getCardPtr(), DECK_ZONE_MAIN);
}

void AbstractTabDeckEditor::onDeckChanged()
{
}

void AbstractTabDeckEditor::onDeckModified()
{
    setModified(!isBlankNewDeck());
    deckMenu->setSaveStatus(!isBlankNewDeck());
}

void AbstractTabDeckEditor::addCardHelper(const ExactCard &card, QString zoneName)
{
    if (!card)
        return;
    if (card.getInfo().getIsToken())
        zoneName = DECK_ZONE_TOKENS;

    QModelIndex newCardIndex = deckDockWidget->deckModel->addCard(card, zoneName);
    // recursiveExpand(newCardIndex);
    deckDockWidget->deckView->clearSelection();
    deckDockWidget->deckView->setCurrentIndex(newCardIndex);
    setModified(true);
    databaseDisplayDockWidget->searchEdit->setSelection(0, databaseDisplayDockWidget->searchEdit->text().length());
}

void AbstractTabDeckEditor::actAddCard(const ExactCard &card)
{
    if (QApplication::keyboardModifiers() & Qt::ControlModifier)
        actAddCardToSideboard(card);
    else
        addCardHelper(card, DECK_ZONE_MAIN);
    deckMenu->setSaveStatus(true);
}

void AbstractTabDeckEditor::actAddCardToSideboard(const ExactCard &card)
{
    addCardHelper(card, DECK_ZONE_SIDE);
    deckMenu->setSaveStatus(true);
}

void AbstractTabDeckEditor::actDecrementCard(const ExactCard &card)
{
    emit decrementCard(card, DECK_ZONE_MAIN);
}

void AbstractTabDeckEditor::actDecrementCardFromSideboard(const ExactCard &card)
{
    emit decrementCard(card, DECK_ZONE_SIDE);
}

void AbstractTabDeckEditor::actSwapCard(const ExactCard &card, const QString &zoneName)
{
    QString providerId = card.getPrinting().getUuid();
    QString collectorNumber = card.getPrinting().getProperty("num");

    QModelIndex foundCard = deckDockWidget->deckModel->findCard(card.getName(), zoneName, providerId, collectorNumber);
    if (!foundCard.isValid()) {
        foundCard = deckDockWidget->deckModel->findCard(card.getName(), zoneName);
    }

    deckDockWidget->swapCard(foundCard);
}

/**
 * Opens the deck in this tab.
 * @param deck The deck. Takes ownership of the object
 */
void AbstractTabDeckEditor::openDeck(DeckLoader *deck)
{
    setDeck(deck);

    if (!deck->getLastFileName().isEmpty()) {
        SettingsCache::instance().recents().updateRecentlyOpenedDeckPaths(deck->getLastFileName());
    }
}

/**
 * Sets the currently active deck for this tab
 * @param _deck The deck. Takes ownership of the object
 */
void AbstractTabDeckEditor::setDeck(DeckLoader *_deck)
{
    deckDockWidget->setDeck(_deck);
    PictureLoader::cacheCardPixmaps(CardDatabaseManager::getInstance()->getCards(getDeckList()->getCardRefList()));
    setModified(false);

    // If they load a deck, make the deck list appear
    aDeckDockVisible->setChecked(true);
    deckDockWidget->setVisible(aDeckDockVisible->isChecked());
}

DeckLoader *AbstractTabDeckEditor::getDeckList() const
{
    return deckDockWidget->getDeckList();
}

void AbstractTabDeckEditor::setModified(bool _modified)
{
    modified = _modified;
    emit tabTextChanged(this, getTabText());
}

/**
 * @brief Returns true if this tab is a blank newly opened tab, as if it was just created with the `New Deck` action.
 */
bool AbstractTabDeckEditor::isBlankNewDeck() const
{
    DeckLoader *deck = getDeckList();
    return !modified && deck->hasNotBeenLoaded();
}

void AbstractTabDeckEditor::actNewDeck()
{
    auto deckOpenLocation = confirmOpen(false);

    if (deckOpenLocation == CANCELLED) {
        return;
    }

    if (deckOpenLocation == NEW_TAB) {
        emit openDeckEditor(nullptr);
        return;
    }

    cleanDeckAndResetModified();
}

void AbstractTabDeckEditor::cleanDeckAndResetModified()
{
    deckMenu->setSaveStatus(false);
    deckDockWidget->cleanDeck();
    setModified(false);
}

/**
 * @brief Displays the save confirmation dialogue that is shown before loading a deck, if required. Takes into
 * account the `openDeckInNewTab` settting.
 *
 * @param openInSameTabIfBlank Open the deck in the same tab instead of a new tab if the current tab is completely
 * blank. Only relevant when the `openDeckInNewTab` setting is enabled.
 *
 * @returns An enum that indicates if and where to load the deck
 */
AbstractTabDeckEditor::DeckOpenLocation AbstractTabDeckEditor::confirmOpen(const bool openInSameTabIfBlank)
{
    // handle `openDeckInNewTab` setting
    if (SettingsCache::instance().getOpenDeckInNewTab()) {
        if (openInSameTabIfBlank && isBlankNewDeck()) {
            return SAME_TAB;
        } else {
            return NEW_TAB;
        }
    }

    // early return if deck is unmodified
    if (!modified) {
        return SAME_TAB;
    }

    // do the save confirmation dialogue
    tabSupervisor->setCurrentWidget(this);

    QMessageBox *msgBox = createSaveConfirmationWindow();
    QPushButton *newTabButton = msgBox->addButton(tr("Open in new tab"), QMessageBox::ApplyRole);

    int ret = msgBox->exec();

    // `exec()` returns an opaque value if a non-standard button was clicked.
    // Directly check if newTabButton was clicked before switching over the standard buttons.
    if (msgBox->clickedButton() == newTabButton) {
        return NEW_TAB;
    }

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
 * @brief Creates the base save confirmation dialogue box.
 *
 * @returns A QMessageBox that can be further modified
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

void AbstractTabDeckEditor::actLoadDeck()
{
    auto deckOpenLocation = confirmOpen();

    if (deckOpenLocation == CANCELLED) {
        return;
    }

    DlgLoadDeck dialog(this);
    if (!dialog.exec())
        return;

    QString fileName = dialog.selectedFiles().at(0);
    openDeckFromFile(fileName, deckOpenLocation);
    deckDockWidget->updateBannerCardComboBox();
}

void AbstractTabDeckEditor::actOpenRecent(const QString &fileName)
{
    auto deckOpenLocation = confirmOpen();

    if (deckOpenLocation == CANCELLED) {
        return;
    }

    openDeckFromFile(fileName, deckOpenLocation);
}

/**
 * Actually opens the deck from file
 * @param fileName The path of the deck to open
 * @param deckOpenLocation Which tab to open the deck
 */
void AbstractTabDeckEditor::openDeckFromFile(const QString &fileName, DeckOpenLocation deckOpenLocation)
{
    DeckLoader::FileFormat fmt = DeckLoader::getFormatFromName(fileName);

    auto *l = new DeckLoader;
    if (l->loadFromFile(fileName, fmt, true)) {
        if (deckOpenLocation == NEW_TAB) {
            emit openDeckEditor(l);
            l->deleteLater();
        } else {
            deckMenu->setSaveStatus(false);
            openDeck(l);
        }
    } else {
        l->deleteLater();
        QMessageBox::critical(this, tr("Error"), tr("Could not open deck at %1").arg(fileName));
    }
    deckMenu->setSaveStatus(true);
}

bool AbstractTabDeckEditor::actSaveDeck()
{
    DeckLoader *const deck = getDeckList();
    if (deck->getLastRemoteDeckId() != -1) {
        QString deckString = deck->writeToString_Native();
        if (deckString.length() > MAX_FILE_LENGTH) {
            QMessageBox::critical(this, tr("Error"), tr("Could not save remote deck"));
            return false;
        }

        Command_DeckUpload cmd;
        cmd.set_deck_id(static_cast<google::protobuf::uint32>(deck->getLastRemoteDeckId()));
        cmd.set_deck_list(deckString.toStdString());

        PendingCommand *pend = AbstractClient::prepareSessionCommand(cmd);
        connect(pend, &PendingCommand::finished, this, &AbstractTabDeckEditor::saveDeckRemoteFinished);
        tabSupervisor->getClient()->sendCommand(pend);

        return true;
    } else if (deck->getLastFileName().isEmpty())
        return actSaveDeckAs();
    else if (deck->saveToFile(deck->getLastFileName(), deck->getLastFileFormat())) {
        setModified(false);
        return true;
    }
    QMessageBox::critical(
        this, tr("Error"),
        tr("The deck could not be saved.\nPlease check that the directory is writable and try again."));
    return false;
}

bool AbstractTabDeckEditor::actSaveDeckAs()
{
    QFileDialog dialog(this, tr("Save deck"));
    dialog.setDirectory(SettingsCache::instance().getDeckPath());
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setDefaultSuffix("cod");
    dialog.setNameFilters(DeckLoader::FILE_NAME_FILTERS);
    dialog.selectFile(getDeckList()->getName().trimmed());

    if (!dialog.exec())
        return false;

    QString fileName = dialog.selectedFiles().at(0);
    DeckLoader::FileFormat fmt = DeckLoader::getFormatFromName(fileName);

    if (!getDeckList()->saveToFile(fileName, fmt)) {
        QMessageBox::critical(
            this, tr("Error"),
            tr("The deck could not be saved.\nPlease check that the directory is writable and try again."));
        return false;
    }
    setModified(false);

    SettingsCache::instance().recents().updateRecentlyOpenedDeckPaths(fileName);

    return true;
}

void AbstractTabDeckEditor::saveDeckRemoteFinished(const Response &response)
{
    if (response.response_code() != Response::RespOk)
        QMessageBox::critical(this, tr("Error"), tr("The deck could not be saved."));
    else
        setModified(false);
}

void AbstractTabDeckEditor::actLoadDeckFromClipboard()
{
    auto deckOpenLocation = confirmOpen();

    if (deckOpenLocation == CANCELLED) {
        return;
    }

    DlgLoadDeckFromClipboard dlg(this);
    if (!dlg.exec())
        return;

    if (deckOpenLocation == NEW_TAB) {
        emit openDeckEditor(dlg.getDeckList());
    } else {
        setDeck(dlg.getDeckList());
        setModified(true);
    }

    deckMenu->setSaveStatus(true);
}

void AbstractTabDeckEditor::editDeckInClipboard(bool annotated)
{
    DlgEditDeckInClipboard dlg(*getDeckList(), annotated, this);
    if (!dlg.exec())
        return;

    setDeck(dlg.getDeckList());
    setModified(true);

    deckMenu->setSaveStatus(true);
}

void AbstractTabDeckEditor::actEditDeckInClipboard()
{
    editDeckInClipboard(true);
}

void AbstractTabDeckEditor::actEditDeckInClipboardRaw()
{
    editDeckInClipboard(false);
}

void AbstractTabDeckEditor::actSaveDeckToClipboard()
{
    getDeckList()->saveToClipboard(true, true);
}

void AbstractTabDeckEditor::actSaveDeckToClipboardNoSetInfo()
{
    getDeckList()->saveToClipboard(true, false);
}

void AbstractTabDeckEditor::actSaveDeckToClipboardRaw()
{
    getDeckList()->saveToClipboard(false, true);
}

void AbstractTabDeckEditor::actSaveDeckToClipboardRawNoSetInfo()
{
    getDeckList()->saveToClipboard(false, false);
}

void AbstractTabDeckEditor::actPrintDeck()
{
    auto *dlg = new QPrintPreviewDialog(this);
    connect(dlg, &QPrintPreviewDialog::paintRequested, deckDockWidget->deckModel, &DeckListModel::printDeckList);
    dlg->exec();
}

void AbstractTabDeckEditor::actLoadDeckFromWebsite()
{
    auto deckOpenLocation = confirmOpen();

    if (deckOpenLocation == CANCELLED) {
        return;
    }

    DlgLoadDeckFromWebsite dlg(this);
    if (!dlg.exec())
        return;

    if (deckOpenLocation == NEW_TAB) {
        emit openDeckEditor(dlg.getDeck());
    } else {
        setDeck(dlg.getDeck());
        setModified(true);
    }

    deckMenu->setSaveStatus(true);
}

void AbstractTabDeckEditor::exportToDecklistWebsite(DeckLoader::DecklistWebsite website)
{
    // check if deck is not null
    if (DeckLoader *const deck = getDeckList()) {
        // Get the decklist url string from the deck loader class.
        QString decklistUrlString = deck->exportDeckToDecklist(website);
        // Check to make sure the string isn't empty.
        if (QString::compare(decklistUrlString, "", Qt::CaseInsensitive) == 0) {
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
    } else {
        // if there's no deck loader object, return an error
        QMessageBox::critical(this, tr("Error"), tr("No deck was selected to be exported."));
    }
}

/**
 * Exports the deck to www.decklist.org (the old website)
 */
void AbstractTabDeckEditor::actExportDeckDecklist()
{
    exportToDecklistWebsite(DeckLoader::DecklistOrg);
}

/**
 * Exports the deck to www.decklist.xyz (the new website)
 */
void AbstractTabDeckEditor::actExportDeckDecklistXyz()
{
    exportToDecklistWebsite(DeckLoader::DecklistXyz);
}

void AbstractTabDeckEditor::actAnalyzeDeckDeckstats()
{
    auto *interface = new DeckStatsInterface(*databaseDisplayDockWidget->databaseModel->getDatabase(),
                                             this); // it deletes itself when done
    interface->analyzeDeck(getDeckList());
}

void AbstractTabDeckEditor::actAnalyzeDeckTappedout()
{
    auto *interface = new TappedOutInterface(*databaseDisplayDockWidget->databaseModel->getDatabase(),
                                             this); // it deletes itself when done
    interface->analyzeDeck(getDeckList());
}

void AbstractTabDeckEditor::filterTreeChanged(FilterTree *filterTree)
{
    databaseDisplayDockWidget->setFilterTree(filterTree);
}

// Method uses to sync docks state with menu items state
bool AbstractTabDeckEditor::eventFilter(QObject *o, QEvent *e)
{
    if (e->type() == QEvent::Close) {
        if (o == cardInfoDockWidget) {
            aCardInfoDockVisible->setChecked(false);
            aCardInfoDockFloating->setEnabled(false);
        } else if (o == deckDockWidget) {
            aDeckDockVisible->setChecked(false);
            aDeckDockFloating->setEnabled(false);
        } else if (o == filterDockWidget) {
            aFilterDockVisible->setChecked(false);
            aFilterDockFloating->setEnabled(false);
        } else if (o == printingSelectorDockWidget) {
            aPrintingSelectorDockVisible->setChecked(false);
            aPrintingSelectorDockFloating->setEnabled(false);
        }
    }
    if (o == this && e->type() == QEvent::Hide) {
        LayoutsSettings &layouts = SettingsCache::instance().layouts();
        layouts.setDeckEditorLayoutState(saveState());
        layouts.setDeckEditorGeometry(saveGeometry());
        layouts.setDeckEditorCardSize(cardInfoDockWidget->size());
        layouts.setDeckEditorFilterSize(filterDockWidget->size());
        layouts.setDeckEditorDeckSize(deckDockWidget->size());
        layouts.setDeckEditorPrintingSelectorSize(printingSelectorDockWidget->size());
    }
    return false;
}

bool AbstractTabDeckEditor::confirmClose()
{
    if (modified) {
        tabSupervisor->setCurrentWidget(this);
        int ret = createSaveConfirmationWindow()->exec();
        if (ret == QMessageBox::Save)
            return actSaveDeck();
        else if (ret == QMessageBox::Cancel)
            return false;
    }
    return true;
}

void AbstractTabDeckEditor::closeRequest(bool forced)
{
    if (!forced && !confirmClose()) {
        return;
    }

    emit deckEditorClosing(this);
    close();
}