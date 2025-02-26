#include "tab_generic_deck_editor.h"

#include "../../client/game_logic/abstract_client.h"
#include "../../client/tapped_out_interface.h"
#include "../../client/ui/widgets/cards/card_info_frame_widget.h"
#include "../../deck/deck_stats_interface.h"
#include "../../dialogs/dlg_load_deck_from_clipboard.h"
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
#include <QProcessEnvironment>
#include <QPushButton>
#include <QRegularExpression>
#include <QSplitter>
#include <QTextStream>
#include <QTreeView>
#include <QUrl>

TabGenericDeckEditor::TabGenericDeckEditor(TabSupervisor *_tabSupervisor) : Tab(_tabSupervisor)
{
}

// Method uses to sync docks state with menu items state
bool TabGenericDeckEditor::eventFilter(QObject *o, QEvent *e)
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

void TabGenericDeckEditor::updateCardInfo(CardInfoPtr _card)
{
    cardInfoDockWidget->cardInfo->setCard(_card);
}

void TabGenericDeckEditor::updateCardInfoLeft(const QModelIndex &current, const QModelIndex & /*previous*/)
{
    cardInfoDockWidget->cardInfo->setCard(current.sibling(current.row(), 0).data().toString());
}

void TabGenericDeckEditor::updateCardInfoRight(const QModelIndex &current, const QModelIndex & /*previous*/)
{
    if (!current.isValid())
        return;
    if (!current.model()->hasChildren(current.sibling(current.row(), 0))) {
        cardInfoDockWidget->cardInfo->setCard(current.sibling(current.row(), 1).data().toString(),
                                              current.sibling(current.row(), 4).data().toString());
    }
}

void TabGenericDeckEditor::updatePrintingSelectorDatabase(const QModelIndex &current, const QModelIndex & /*previous*/)
{
    const QString cardName = current.sibling(current.row(), 0).data().toString();
    const QString cardProviderID = CardDatabaseManager::getInstance()->getPreferredPrintingProviderIdForCard(cardName);

    if (!current.isValid()) {
        return;
    }

    if (!current.model()->hasChildren(current.sibling(current.row(), 0))) {
        printingSelectorDockWidget->printingSelector->setCard(
            CardDatabaseManager::getInstance()->getCardByNameAndProviderId(cardName, cardProviderID), DECK_ZONE_MAIN);
    }
}

void TabGenericDeckEditor::updatePrintingSelectorDeckView(const QModelIndex &current, const QModelIndex & /*previous*/)
{
    const QString cardName = current.sibling(current.row(), 1).data().toString();
    const QString cardProviderID = current.sibling(current.row(), 4).data().toString();
    const QModelIndex gparent = current.parent().parent();

    if (!gparent.isValid()) {
        return;
    }

    const QString zoneName = gparent.sibling(gparent.row(), 1).data(Qt::EditRole).toString();

    if (!current.isValid()) {
        return;
    }

    if (!current.model()->hasChildren(current.sibling(current.row(), 0))) {
        printingSelectorDockWidget->printingSelector->setCard(
            CardDatabaseManager::getInstance()->getCardByNameAndProviderId(cardName, cardProviderID), zoneName);
    }
}

void TabGenericDeckEditor::decklistCustomMenu(QPoint point)
{
    QMenu menu;
    const CardInfoPtr info = cardInfoDockWidget->cardInfo->getInfo();

    QAction *selectPrinting = menu.addAction(tr("Select Printing"));

    connect(selectPrinting, &QAction::triggered, this, &TabGenericDeckEditor::showPrintingSelector);

    menu.exec(deckDockWidget->deckView->mapToGlobal(point));
}

bool TabGenericDeckEditor::confirmClose()
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

void TabGenericDeckEditor::closeRequest(bool forced)
{
    if (!forced && !confirmClose()) {
        return;
    }

    emit deckEditorClosing(this);
    close();
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
TabGenericDeckEditor::DeckOpenLocation TabGenericDeckEditor::confirmOpen(const bool openInSameTabIfBlank)
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
QMessageBox *TabGenericDeckEditor::createSaveConfirmationWindow()
{
    QMessageBox *msgBox = new QMessageBox(this);
    msgBox->setIcon(QMessageBox::Warning);
    msgBox->setWindowTitle(tr("Are you sure?"));
    msgBox->setText(tr("The decklist has been modified.\nDo you want to save the changes?"));
    msgBox->setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    return msgBox;
}

void TabGenericDeckEditor::actOpenRecent(const QString &fileName)
{
    auto deckOpenLocation = confirmOpen();

    if (deckOpenLocation == CANCELLED) {
        return;
    }

    openDeckFromFile(fileName, deckOpenLocation);
}

void TabGenericDeckEditor::saveDeckRemoteFinished(const Response &response)
{
    if (response.response_code() != Response::RespOk)
        QMessageBox::critical(this, tr("Error"), tr("The deck could not be saved."));
    else
        setModified(false);
}

bool TabGenericDeckEditor::actSaveDeck()
{
    DeckLoader *const deck = deckDockWidget->deckModel->getDeckList();
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
        connect(pend, SIGNAL(finished(Response, CommandContainer, QVariant)), this,
                SLOT(saveDeckRemoteFinished(Response)));
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

bool TabGenericDeckEditor::actSaveDeckAs()
{
    QFileDialog dialog(this, tr("Save deck"));
    dialog.setDirectory(SettingsCache::instance().getDeckPath());
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setDefaultSuffix("cod");
    dialog.setNameFilters(DeckLoader::fileNameFilters);
    dialog.selectFile(deckDockWidget->deckModel->getDeckList()->getName().trimmed() + ".cod");
    if (!dialog.exec())
        return false;

    QString fileName = dialog.selectedFiles().at(0);
    DeckLoader::FileFormat fmt = DeckLoader::getFormatFromName(fileName);

    if (!deckDockWidget->deckModel->getDeckList()->saveToFile(fileName, fmt)) {
        QMessageBox::critical(
            this, tr("Error"),
            tr("The deck could not be saved.\nPlease check that the directory is writable and try again."));
        return false;
    }
    setModified(false);

    SettingsCache::instance().recents().updateRecentlyOpenedDeckPaths(fileName);

    return true;
}

void TabGenericDeckEditor::actLoadDeckFromClipboard()
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

void TabGenericDeckEditor::actSaveDeckToClipboard()
{
    QString buffer;
    QTextStream stream(&buffer);
    deckDockWidget->deckModel->getDeckList()->saveToStream_Plain(stream);
    QApplication::clipboard()->setText(buffer, QClipboard::Clipboard);
    QApplication::clipboard()->setText(buffer, QClipboard::Selection);
}

void TabGenericDeckEditor::actSaveDeckToClipboardNoSetNameAndNumber()
{
    QString buffer;
    QTextStream stream(&buffer);
    deckDockWidget->deckModel->getDeckList()->saveToStream_Plain(stream, true, false);
    QApplication::clipboard()->setText(buffer, QClipboard::Clipboard);
    QApplication::clipboard()->setText(buffer, QClipboard::Selection);
}

void TabGenericDeckEditor::actSaveDeckToClipboardRaw()
{
    QString buffer;
    QTextStream stream(&buffer);
    deckDockWidget->deckModel->getDeckList()->saveToStream_Plain(stream, false);
    QApplication::clipboard()->setText(buffer, QClipboard::Clipboard);
    QApplication::clipboard()->setText(buffer, QClipboard::Selection);
}

void TabGenericDeckEditor::actSaveDeckToClipboardRawNoSetNameAndNumber()
{
    QString buffer;
    QTextStream stream(&buffer);
    deckDockWidget->deckModel->getDeckList()->saveToStream_Plain(stream, false, false);
    QApplication::clipboard()->setText(buffer, QClipboard::Clipboard);
    QApplication::clipboard()->setText(buffer, QClipboard::Selection);
}

void TabGenericDeckEditor::actPrintDeck()
{
    auto *dlg = new QPrintPreviewDialog(this);
    connect(dlg, SIGNAL(paintRequested(QPrinter *)), deckDockWidget->deckModel, SLOT(printDeckList(QPrinter *)));
    dlg->exec();
}

// Action called when export deck to decklist menu item is pressed.
void TabGenericDeckEditor::actExportDeckDecklist()
{
    // Get the decklist class for the deck.
    DeckLoader *const deck = deckDockWidget->deckModel->getDeckList();
    // create a string to load the decklist url into.
    QString decklistUrlString;
    // check if deck is not null
    if (deck) {
        // Get the decklist url string from the deck loader class.
        decklistUrlString = deck->exportDeckToDecklist();
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
        QMessageBox::critical(this, tr("Error"), tr("No deck was selected to be saved."));
    }
}

void TabGenericDeckEditor::actAnalyzeDeckDeckstats()
{
    auto *interface = new DeckStatsInterface(*databaseDisplayDockWidget->databaseModel->getDatabase(),
                                             this); // it deletes itself when done
    interface->analyzeDeck(deckDockWidget->deckModel->getDeckList());
}

void TabGenericDeckEditor::actAnalyzeDeckTappedout()
{
    auto *interface = new TappedOutInterface(*databaseDisplayDockWidget->databaseModel->getDatabase(),
                                             this); // it deletes itself when done
    interface->analyzeDeck(deckDockWidget->deckModel->getDeckList());
}

void TabGenericDeckEditor::addCardHelper(const CardInfoPtr info, QString zoneName)
{
    if (!info)
        return;
    if (info->getIsToken())
        zoneName = DECK_ZONE_TOKENS;

    QModelIndex newCardIndex = deckDockWidget->deckModel->addPreferredPrintingCard(info->getName(), zoneName, false);
    // recursiveExpand(newCardIndex);
    deckDockWidget->deckView->clearSelection();
    deckDockWidget->deckView->setCurrentIndex(newCardIndex);
    setModified(true);
    databaseDisplayDockWidget->searchEdit->setSelection(0, databaseDisplayDockWidget->searchEdit->text().length());
}

void TabGenericDeckEditor::actAddCardFromDatabase()
{
    actAddCard(databaseDisplayDockWidget->currentCardInfo());
}

void TabGenericDeckEditor::actAddCard(CardInfoPtr info)
{
    if (QApplication::keyboardModifiers() & Qt::ControlModifier)
        actAddCardToSideboard(info);
    else
        addCardHelper(info, DECK_ZONE_MAIN);
    deckMenu->setSaveStatus(true);
}

void TabGenericDeckEditor::actAddCardToSideboardFromDatabase()
{
    addCardHelper(databaseDisplayDockWidget->currentCardInfo(), DECK_ZONE_SIDE);
    deckMenu->setSaveStatus(true);
}

void TabGenericDeckEditor::actAddCardToSideboard(CardInfoPtr info)
{
    addCardHelper(info, DECK_ZONE_SIDE);
    deckMenu->setSaveStatus(true);
}

void TabGenericDeckEditor::decrementCardHelper(CardInfoPtr info, QString zoneName)
{
    if (!info)
        return;
    if (info->getIsToken())
        zoneName = DECK_ZONE_TOKENS;

    QString providerId = CardDatabaseManager::getInstance()->getSetInfoForCard(info).getProperty("uuid");
    QString collectorNumber = CardDatabaseManager::getInstance()->getSetInfoForCard(info).getProperty("num");

    QModelIndex idx = deckDockWidget->deckModel->findCard(info->getName(), zoneName, providerId, collectorNumber);
    if (!idx.isValid()) {
        return;
    }

    deckDockWidget->deckView->clearSelection();
    deckDockWidget->deckView->setCurrentIndex(idx);
    deckDockWidget->offsetCountAtIndex(idx, -1);
}

void TabGenericDeckEditor::actSwapCard(CardInfoPtr info, QString zoneName)
{
    QString providerId = CardDatabaseManager::getInstance()->getSetInfoForCard(info).getProperty("uuid");
    QString collectorNumber = CardDatabaseManager::getInstance()->getSetInfoForCard(info).getProperty("num");
    deckDockWidget->swapCard(
        deckDockWidget->deckModel->findCard(info->getName(), zoneName, providerId, collectorNumber));
}

void TabGenericDeckEditor::actDecrementCard(CardInfoPtr info)
{
    decrementCardHelper(info, DECK_ZONE_MAIN);
}

void TabGenericDeckEditor::actDecrementCardFromDatabase()
{
    decrementCardHelper(databaseDisplayDockWidget->currentCardInfo(), DECK_ZONE_MAIN);
}

void TabGenericDeckEditor::actDecrementCardFromSideboard(CardInfoPtr info)
{
    decrementCardHelper(info, DECK_ZONE_SIDE);
}

void TabGenericDeckEditor::actDecrementCardFromSideboardFromDatabase()
{
    decrementCardHelper(databaseDisplayDockWidget->currentCardInfo(), DECK_ZONE_SIDE);
}

void TabGenericDeckEditor::setDeck(DeckLoader *_deck)
{
    deckDockWidget->setDeck(_deck);
    PictureLoader::cacheCardPixmaps(
        CardDatabaseManager::getInstance()->getCards(deckDockWidget->deckModel->getDeckList()->getCardList()));
    setModified(false);

    // If they load a deck, make the deck list appear
    aDeckDockVisible->setChecked(true);
    deckDockWidget->setVisible(aDeckDockVisible->isChecked());
}

void TabGenericDeckEditor::setModified(bool _modified)
{
    modified = _modified;
    emit tabTextChanged(this, getTabText());
}

/**
 * @brief Returns true if this tab is a blank newly opened tab, as if it was just created with the `New Deck` action.
 */
bool TabGenericDeckEditor::isBlankNewDeck() const
{
    DeckLoader *const deck = deckDockWidget->deckModel->getDeckList();
    return !modified && deck->getLastFileName().isEmpty() && deck->getLastRemoteDeckId() == -1;
}

void TabGenericDeckEditor::filterTreeChanged(FilterTree *filterTree)
{
    databaseDisplayDockWidget->setFilterTree(filterTree);
}