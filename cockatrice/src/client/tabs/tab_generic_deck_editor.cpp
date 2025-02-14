#include "tab_generic_deck_editor.h"

#include "../../client/game_logic/abstract_client.h"
#include "../../client/tapped_out_interface.h"
#include "../../client/ui/widgets/cards/card_info_frame_widget.h"
#include "../../deck/deck_stats_interface.h"
#include "../../dialogs/dlg_load_deck_from_clipboard.h"
#include "../../game/cards/card_database_manager.h"
#include "../../game/cards/card_database_model.h"
#include "../../game/filters/filter_builder.h"
#include "../../game/filters/filter_tree_model.h"
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
#include <QComboBox>
#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QDockWidget>
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
#include <QTextBrowser>
#include <QTextEdit>
#include <QTextStream>
#include <QTreeView>
#include <QUrl>

static bool canBeCommander(const CardInfoPtr &cardInfo)
{
    return ((cardInfo->getCardType().contains("Legendary", Qt::CaseInsensitive) &&
             cardInfo->getCardType().contains("Creature", Qt::CaseInsensitive))) ||
           cardInfo->getText().contains("can be your commander", Qt::CaseInsensitive);
}

TabGenericDeckEditor::TabGenericDeckEditor(TabSupervisor *_tabSupervisor) : Tab(_tabSupervisor)
{
}

// Method uses to sync docks state with menu items state
bool TabGenericDeckEditor::eventFilter(QObject *o, QEvent *e)
{
    if (e->type() == QEvent::Close) {
        if (o == cardInfoDock) {
            aCardInfoDockVisible->setChecked(false);
            aCardInfoDockFloating->setEnabled(false);
        } else if (o == deckDock) {
            aDeckDockVisible->setChecked(false);
            aDeckDockFloating->setEnabled(false);
        } else if (o == filterDock) {
            aFilterDockVisible->setChecked(false);
            aFilterDockFloating->setEnabled(false);
        } else if (o == printingSelectorDock) {
            aPrintingSelectorDockVisible->setChecked(false);
            aPrintingSelectorDockFloating->setEnabled(false);
        }
    }
    if (o == this && e->type() == QEvent::Hide) {
        LayoutsSettings &layouts = SettingsCache::instance().layouts();
        layouts.setDeckEditorLayoutState(saveState());
        layouts.setDeckEditorGeometry(saveGeometry());
        layouts.setDeckEditorCardSize(cardInfoDock->size());
        layouts.setDeckEditorFilterSize(filterDock->size());
        layouts.setDeckEditorDeckSize(deckDock->size());
        layouts.setDeckEditorPrintingSelectorSize(printingSelectorDock->size());
    }
    return false;
}

void TabGenericDeckEditor::updateName(const QString &name)
{
    deckModel->getDeckList()->setName(name);
    setModified(true);
    setSaveStatus(true);
}

void TabGenericDeckEditor::updateComments()
{
    deckModel->getDeckList()->setComments(commentsEdit->toPlainText());
    setModified(true);
    setSaveStatus(true);
}

void TabGenericDeckEditor::updateBannerCardComboBox()
{
    // Store the current text of the combo box
    QString currentText = bannerCardComboBox->currentText();

    // Block signals temporarily
    bool wasBlocked = bannerCardComboBox->blockSignals(true);

    // Clear the existing items in the combo box
    bannerCardComboBox->clear();

    // Prepare the new items with deduplication
    QSet<QPair<QString, QString>> bannerCardSet;
    InnerDecklistNode *listRoot = deckModel->getDeckList()->getRoot();
    for (int i = 0; i < listRoot->size(); i++) {
        InnerDecklistNode *currentZone = dynamic_cast<InnerDecklistNode *>(listRoot->at(i));
        for (int j = 0; j < currentZone->size(); j++) {
            DecklistCardNode *currentCard = dynamic_cast<DecklistCardNode *>(currentZone->at(j));
            if (!currentCard)
                continue;

            for (int k = 0; k < currentCard->getNumber(); ++k) {
                CardInfoPtr info = CardDatabaseManager::getInstance()->getCardByNameAndProviderId(
                    currentCard->getName(), currentCard->getCardProviderId());
                if (info) {
                    bannerCardSet.insert(
                        QPair<QString, QString>(currentCard->getName(), currentCard->getCardProviderId()));
                }
            }
        }
    }

    QList<QPair<QString, QString>> pairList = bannerCardSet.values();

    // Sort QList by the first() element of the QPair
    std::sort(pairList.begin(), pairList.end(), [](const QPair<QString, QString> &a, const QPair<QString, QString> &b) {
        return a.first.toLower() < b.first.toLower();
    });

    for (const auto &pair : pairList) {
        QVariantMap dataMap;
        dataMap["name"] = pair.first;
        dataMap["uuid"] = pair.second;

        bannerCardComboBox->addItem(pair.first, dataMap);
    }

    // Try to restore the previous selection by finding the currentText
    int restoredIndex = bannerCardComboBox->findText(currentText);
    if (restoredIndex != -1) {
        bannerCardComboBox->setCurrentIndex(restoredIndex);
    } else {
        // Add a placeholder "-" and set it as the current selection
        int bannerIndex = bannerCardComboBox->findText(deckModel->getDeckList()->getBannerCard().first);
        if (bannerIndex != -1) {
            bannerCardComboBox->setCurrentIndex(bannerIndex);
        } else {
            bannerCardComboBox->insertItem(0, "-");
            bannerCardComboBox->setCurrentIndex(0);
        }
    }

    // Restore the previous signal blocking state
    bannerCardComboBox->blockSignals(wasBlocked);
}

void TabGenericDeckEditor::setBannerCard(int /* changedIndex */)
{
    QVariantMap itemData = bannerCardComboBox->itemData(bannerCardComboBox->currentIndex()).toMap();
    deckModel->getDeckList()->setBannerCard(
        QPair<QString, QString>(itemData["name"].toString(), itemData["uuid"].toString()));
}

void TabGenericDeckEditor::updateCardInfo(CardInfoPtr _card)
{
    cardInfo->setCard(_card);
}

void TabGenericDeckEditor::updateCardInfoLeft(const QModelIndex &current, const QModelIndex & /*previous*/)
{
    cardInfo->setCard(current.sibling(current.row(), 0).data().toString());
}

void TabGenericDeckEditor::updateCardInfoRight(const QModelIndex &current, const QModelIndex & /*previous*/)
{
    if (!current.isValid())
        return;
    if (!current.model()->hasChildren(current.sibling(current.row(), 0))) {
        cardInfo->setCard(current.sibling(current.row(), 1).data().toString(),
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
        printingSelector->setCard(
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
        printingSelector->setCard(
            CardDatabaseManager::getInstance()->getCardByNameAndProviderId(cardName, cardProviderID), zoneName);
    }
}

void TabGenericDeckEditor::updateSearch(const QString &search)
{
    databaseDisplayModel->setStringFilter(search);
    QModelIndexList sel = databaseView->selectionModel()->selectedRows();
    if (sel.isEmpty() && databaseDisplayModel->rowCount())
        databaseView->selectionModel()->setCurrentIndex(databaseDisplayModel->index(0, 0),
                                                        QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
}

void TabGenericDeckEditor::databaseCustomMenu(QPoint point)
{
    QMenu menu;
    const CardInfoPtr info = currentCardInfo();

    if (info) {
        // add to deck and sideboard options
        QAction *addToDeck, *addToSideboard, *selectPrinting, *edhRecCommander, *edhRecCard;
        addToDeck = menu.addAction(tr("Add to Deck"));
        addToSideboard = menu.addAction(tr("Add to Sideboard"));
        selectPrinting = menu.addAction(tr("Select Printing"));
        if (canBeCommander(info)) {
            edhRecCommander = menu.addAction(tr("Show on EDHREC (Commander)"));
            connect(edhRecCommander, &QAction::triggered, this,
                    [this, info] { this->tabSupervisor->addEdhrecTab(info, true); });
        }
        edhRecCard = menu.addAction(tr("Show on EDHREC (Card)"));

        connect(addToDeck, SIGNAL(triggered()), this, SLOT(actAddCard()));
        connect(addToSideboard, SIGNAL(triggered()), this, SLOT(actAddCardToSideboard()));
        connect(selectPrinting, &QAction::triggered, this, [this, info] { this->showPrintingSelector(); });
        connect(edhRecCard, &QAction::triggered, this, [this, info] { this->tabSupervisor->addEdhrecTab(info); });

        // filling out the related cards submenu
        auto *relatedMenu = new QMenu(tr("Show Related cards"));
        menu.addMenu(relatedMenu);
        auto relatedCards = info->getAllRelatedCards();
        if (relatedCards.isEmpty()) {
            relatedMenu->setDisabled(true);
        } else {
            for (const CardRelation *rel : relatedCards) {
                const QString &relatedCardName = rel->getName();
                QAction *relatedCard = relatedMenu->addAction(relatedCardName);
                connect(relatedCard, &QAction::triggered, cardInfo,
                        [this, relatedCardName] { cardInfo->setCard(relatedCardName); });
            }
        }
        menu.exec(databaseView->mapToGlobal(point));
    }
}

void TabGenericDeckEditor::decklistCustomMenu(QPoint point)
{
    QMenu menu;
    const CardInfoPtr info = cardInfo->getInfo();

    QAction *selectPrinting = menu.addAction(tr("Select Printing"));

    connect(selectPrinting, &QAction::triggered, this, &TabGenericDeckEditor::showPrintingSelector);

    menu.exec(deckView->mapToGlobal(point));
}

void TabGenericDeckEditor::updateHash()
{
    hashLabel->setText(deckModel->getDeckList()->getDeckHash());
}

void TabGenericDeckEditor::updateRecentlyOpened()
{
    loadRecentDeckMenu->clear();
    for (const auto &deckPath : SettingsCache::instance().recents().getRecentlyOpenedDeckPaths()) {
        QAction *aRecentlyOpenedDeck = new QAction(deckPath, this);
        loadRecentDeckMenu->addAction(aRecentlyOpenedDeck);
        connect(aRecentlyOpenedDeck, &QAction::triggered, this,
                [=, this] { actOpenRecent(aRecentlyOpenedDeck->text()); });
    }
    loadRecentDeckMenu->addSeparator();
    loadRecentDeckMenu->addAction(aClearRecents);
    aClearRecents->setEnabled(SettingsCache::instance().recents().getRecentlyOpenedDeckPaths().length() > 0);
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

void TabGenericDeckEditor::actClearRecents()
{
    SettingsCache::instance().recents().clearRecentlyOpenedDeckPaths();
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
    DeckLoader *const deck = deckModel->getDeckList();
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
    dialog.selectFile(deckModel->getDeckList()->getName().trimmed() + ".cod");
    if (!dialog.exec())
        return false;

    QString fileName = dialog.selectedFiles().at(0);
    DeckLoader::FileFormat fmt = DeckLoader::getFormatFromName(fileName);

    if (!deckModel->getDeckList()->saveToFile(fileName, fmt)) {
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

    setSaveStatus(true);
}

void TabGenericDeckEditor::actSaveDeckToClipboard()
{
    QString buffer;
    QTextStream stream(&buffer);
    deckModel->getDeckList()->saveToStream_Plain(stream);
    QApplication::clipboard()->setText(buffer, QClipboard::Clipboard);
    QApplication::clipboard()->setText(buffer, QClipboard::Selection);
}

void TabGenericDeckEditor::actSaveDeckToClipboardNoSetNameAndNumber()
{
    QString buffer;
    QTextStream stream(&buffer);
    deckModel->getDeckList()->saveToStream_Plain(stream, true, false);
    QApplication::clipboard()->setText(buffer, QClipboard::Clipboard);
    QApplication::clipboard()->setText(buffer, QClipboard::Selection);
}

void TabGenericDeckEditor::actSaveDeckToClipboardRaw()
{
    QString buffer;
    QTextStream stream(&buffer);
    deckModel->getDeckList()->saveToStream_Plain(stream, false);
    QApplication::clipboard()->setText(buffer, QClipboard::Clipboard);
    QApplication::clipboard()->setText(buffer, QClipboard::Selection);
}

void TabGenericDeckEditor::actSaveDeckToClipboardRawNoSetNameAndNumber()
{
    QString buffer;
    QTextStream stream(&buffer);
    deckModel->getDeckList()->saveToStream_Plain(stream, false, false);
    QApplication::clipboard()->setText(buffer, QClipboard::Clipboard);
    QApplication::clipboard()->setText(buffer, QClipboard::Selection);
}

void TabGenericDeckEditor::actPrintDeck()
{
    auto *dlg = new QPrintPreviewDialog(this);
    connect(dlg, SIGNAL(paintRequested(QPrinter *)), deckModel, SLOT(printDeckList(QPrinter *)));
    dlg->exec();
}

// Action called when export deck to decklist menu item is pressed.
void TabGenericDeckEditor::actExportDeckDecklist()
{
    // Get the decklist class for the deck.
    DeckLoader *const deck = deckModel->getDeckList();
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
    auto *interface = new DeckStatsInterface(*databaseModel->getDatabase(),
                                             this); // it deletes itself when done
    interface->analyzeDeck(deckModel->getDeckList());
}

void TabGenericDeckEditor::actAnalyzeDeckTappedout()
{
    auto *interface = new TappedOutInterface(*databaseModel->getDatabase(),
                                             this); // it deletes itself when done
    interface->analyzeDeck(deckModel->getDeckList());
}

void TabGenericDeckEditor::actClearFilterAll()
{
    databaseDisplayModel->clearFilterAll();
    searchEdit->setText("");
}

void TabGenericDeckEditor::actClearFilterOne()
{
    QModelIndexList selIndexes = filterView->selectionModel()->selectedIndexes();
    for (QModelIndex idx : selIndexes) {
        filterModel->removeRow(idx.row(), idx.parent());
    }
}

/**
 * Gets the index of all the currently selected card nodes in the decklist table.
 * The list is in reverse order of the visual selection, so that rows can be deleted while iterating over them.
 *
 * @return A model index list containing all selected card nodes
 */
QModelIndexList TabGenericDeckEditor::getSelectedCardNodes() const
{
    auto selectedRows = deckView->selectionModel()->selectedRows();

    const auto notLeafNode = [this](const auto &index) { return deckModel->hasChildren(index); };
    selectedRows.erase(std::remove_if(selectedRows.begin(), selectedRows.end(), notLeafNode), selectedRows.end());

    std::reverse(selectedRows.begin(), selectedRows.end());
    return selectedRows;
}

CardInfoPtr TabGenericDeckEditor::currentCardInfo() const
{
    const QModelIndex currentIndex = databaseView->selectionModel()->currentIndex();
    if (!currentIndex.isValid()) {
        return {};
    }

    const QString cardName = currentIndex.sibling(currentIndex.row(), 0).data().toString();

    return CardDatabaseManager::getInstance()->getCard(cardName);
}

void TabGenericDeckEditor::recursiveExpand(const QModelIndex &index)
{
    if (index.parent().isValid())
        recursiveExpand(index.parent());
    deckView->expand(index);
}

void TabGenericDeckEditor::actSwapCard()
{
    auto selectedRows = getSelectedCardNodes();

    // hack to maintain the old reselection behavior when currently selected row of a single-selection gets deleted
    // TODO: remove the hack and also handle reselection when all rows of a multi-selection gets deleted
    if (selectedRows.length() == 1) {
        deckView->setSelectionMode(QAbstractItemView::SingleSelection);
    }

    bool isModified = false;
    for (const auto &currentIndex : selectedRows) {
        if (swapCard(currentIndex)) {
            isModified = true;
        }
    }

    deckView->setSelectionMode(QAbstractItemView::ExtendedSelection);

    if (isModified) {
        setModified(true);
        setSaveStatus(true);
    }

    update();
}

/**
 * Swaps the card at the index between the maindeck and sideboard
 *
 * @param currentIndex The index to swap.
 * @return True if the swap was successful
 */
bool TabGenericDeckEditor::swapCard(const QModelIndex &currentIndex)
{
    if (!currentIndex.isValid())
        return false;
    const QString cardName = currentIndex.sibling(currentIndex.row(), 1).data().toString();
    const QString cardProviderID = currentIndex.sibling(currentIndex.row(), 4).data().toString();
    const QModelIndex gparent = currentIndex.parent().parent();

    if (!gparent.isValid())
        return false;

    const QString zoneName = gparent.sibling(gparent.row(), 1).data(Qt::EditRole).toString();
    offsetCountAtIndex(currentIndex, -1);
    const QString otherZoneName = zoneName == DECK_ZONE_MAIN ? DECK_ZONE_SIDE : DECK_ZONE_MAIN;

    // Third argument (true) says create the card no matter what, even if not in DB
    QModelIndex newCardIndex = deckModel->addCard(
        cardName, CardDatabaseManager::getInstance()->getSpecificSetForCard(cardName, cardProviderID), otherZoneName,
        true);
    recursiveExpand(newCardIndex);

    return true;
}

void TabGenericDeckEditor::addCardHelper(const CardInfoPtr info, QString zoneName)
{
    if (!info)
        return;
    if (info->getIsToken())
        zoneName = DECK_ZONE_TOKENS;

    QModelIndex newCardIndex = deckModel->addPreferredPrintingCard(info->getName(), zoneName, false);
    recursiveExpand(newCardIndex);
    deckView->clearSelection();
    deckView->setCurrentIndex(newCardIndex);
    setModified(true);
    searchEdit->setSelection(0, searchEdit->text().length());
}

void TabGenericDeckEditor::actAddCard()
{
    if (QApplication::keyboardModifiers() & Qt::ControlModifier)
        actAddCardToSideboard();
    else
        addCardHelper(currentCardInfo(), DECK_ZONE_MAIN);
    setSaveStatus(true);
}

void TabGenericDeckEditor::actAddCardToSideboard()
{
    addCardHelper(currentCardInfo(), DECK_ZONE_SIDE);
    setSaveStatus(true);
}

void TabGenericDeckEditor::actRemoveCard()
{
    auto selectedRows = getSelectedCardNodes();

    // hack to maintain the old reselection behavior when currently selected row of a single-selection gets deleted
    // TODO: remove the hack and also handle reselection when all rows of a multi-selection gets deleted
    if (selectedRows.length() == 1) {
        deckView->setSelectionMode(QAbstractItemView::SingleSelection);
    }

    bool isModified = false;
    for (const auto &index : selectedRows) {
        if (!index.isValid() || deckModel->hasChildren(index)) {
            continue;
        }
        deckModel->removeRow(index.row(), index.parent());
        isModified = true;
    }

    deckView->setSelectionMode(QAbstractItemView::ExtendedSelection);

    if (isModified) {
        DeckLoader *const deck = deckModel->getDeckList();
        setSaveStatus(!deck->isEmpty());
        setModified(true);
    }
}

void TabGenericDeckEditor::offsetCountAtIndex(const QModelIndex &idx, int offset)
{
    if (!idx.isValid() || deckModel->hasChildren(idx)) {
        return;
    }

    const QModelIndex numberIndex = idx.sibling(idx.row(), 0);
    const int count = deckModel->data(numberIndex, Qt::EditRole).toInt();
    const int new_count = count + offset;
    if (new_count <= 0)
        deckModel->removeRow(idx.row(), idx.parent());
    else
        deckModel->setData(numberIndex, new_count, Qt::EditRole);
    setModified(true);
}

void TabGenericDeckEditor::actIncrement()
{
    auto selectedRows = getSelectedCardNodes();

    for (const auto &index : selectedRows) {
        offsetCountAtIndex(index, 1);
    }
}

void TabGenericDeckEditor::actDecrement()
{
    auto selectedRows = getSelectedCardNodes();

    // hack to maintain the old reselection behavior when currently selected row of a single-selection gets deleted
    // TODO: remove the hack and also handle reselection when all rows of a multi-selection gets deleted
    if (selectedRows.length() == 1) {
        deckView->setSelectionMode(QAbstractItemView::SingleSelection);
    }

    for (const auto &index : selectedRows) {
        offsetCountAtIndex(index, -1);
    }

    deckView->setSelectionMode(QAbstractItemView::ExtendedSelection);
}

void TabGenericDeckEditor::decrementCardHelper(QString zoneName)
{
    const CardInfoPtr info = currentCardInfo();

    if (!info)
        return;
    if (info->getIsToken())
        zoneName = DECK_ZONE_TOKENS;

    QModelIndex idx = deckModel->findCard(info->getName(), zoneName);
    if (!idx.isValid()) {
        return;
    }
    deckView->clearSelection();
    deckView->setCurrentIndex(idx);
    offsetCountAtIndex(idx, -1);
}

void TabGenericDeckEditor::actDecrementCard()
{
    decrementCardHelper(DECK_ZONE_MAIN);
}

void TabGenericDeckEditor::actDecrementCardFromSideboard()
{
    decrementCardHelper(DECK_ZONE_SIDE);
}

void TabGenericDeckEditor::copyDatabaseCellContents()
{
    auto _data = databaseView->selectionModel()->currentIndex().data();
    QApplication::clipboard()->setText(_data.toString());
}

void TabGenericDeckEditor::filterViewCustomContextMenu(const QPoint &point)
{
    QMenu menu;
    QAction *action;
    QModelIndex idx;

    idx = filterView->indexAt(point);
    if (!idx.isValid())
        return;

    action = menu.addAction(QString("delete"));
    action->setData(point);
    connect(&menu, SIGNAL(triggered(QAction *)), this, SLOT(filterRemove(QAction *)));
    menu.exec(filterView->mapToGlobal(point));
}

void TabGenericDeckEditor::filterRemove(QAction *action)
{
    QPoint point;
    QModelIndex idx;

    point = action->data().toPoint();
    idx = filterView->indexAt(point);
    if (!idx.isValid())
        return;

    filterModel->removeRow(idx.row(), idx.parent());
}

void TabGenericDeckEditor::saveDbHeaderState()
{
    SettingsCache::instance().layouts().setDeckEditorDbHeaderState(databaseView->header()->saveState());
}

void TabGenericDeckEditor::setSaveStatus(bool newStatus)
{
    aSaveDeck->setEnabled(newStatus);
    aSaveDeckAs->setEnabled(newStatus);
    aSaveDeckToClipboard->setEnabled(newStatus);
    aSaveDeckToClipboardNoSetNameAndNumber->setEnabled(newStatus);
    aSaveDeckToClipboardRaw->setEnabled(newStatus);
    aSaveDeckToClipboardRawNoSetNameAndNumber->setEnabled(newStatus);
    saveDeckToClipboardMenu->setEnabled(newStatus);
    aPrintDeck->setEnabled(newStatus);
    analyzeDeckMenu->setEnabled(newStatus);
}

void TabGenericDeckEditor::showSearchSyntaxHelp()
{

    QFile file("theme:help/search.md");

    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        return;
    }

    QTextStream in(&file);
    QString text = in.readAll();
    file.close();

    // Poor Markdown Converter
    auto opts = QRegularExpression::MultilineOption;
    text = text.replace(QRegularExpression("^(###)(.*)", opts), "<h3>\\2</h3>")
               .replace(QRegularExpression("^(##)(.*)", opts), "<h2>\\2</h2>")
               .replace(QRegularExpression("^(#)(.*)", opts), "<h1>\\2</h1>")
               .replace(QRegularExpression("^------*", opts), "<hr />")
               .replace(QRegularExpression(R"(\[([^[]+)\]\(([^\)]+)\))", opts), R"(<a href='\2'>\1</a>)");

    auto browser = new QTextBrowser;
    browser->setParent(this, Qt::Window | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint |
                                 Qt::WindowCloseButtonHint | Qt::WindowFullscreenButtonHint);
    browser->setWindowTitle("Search Help");
    browser->setReadOnly(true);
    browser->setMinimumSize({500, 600});

    QString sheet = QString("a { text-decoration: underline; color: rgb(71,158,252) };");
    browser->document()->setDefaultStyleSheet(sheet);

    browser->setHtml(text);
    connect(browser, &QTextBrowser::anchorClicked, [this](const QUrl &link) { searchEdit->setText(link.fragment()); });
    browser->show();
}

void TabGenericDeckEditor::setDeck(DeckLoader *_deck)
{
    deckModel->setDeckList(_deck);

    nameEdit->setText(deckModel->getDeckList()->getName());
    commentsEdit->setText(deckModel->getDeckList()->getComments());
    bannerCardComboBox->setCurrentText(deckModel->getDeckList()->getBannerCard().first);
    updateBannerCardComboBox();
    updateHash();
    deckModel->sort(deckView->header()->sortIndicatorSection(), deckView->header()->sortIndicatorOrder());
    deckView->expandAll();
    setModified(false);

    PictureLoader::cacheCardPixmaps(
        CardDatabaseManager::getInstance()->getCards(deckModel->getDeckList()->getCardList()));
    deckView->expandAll();
    setModified(false);

    deckTagsDisplayWidget->connectDeckList(deckModel->getDeckList());

    // If they load a deck, make the deck list appear
    aDeckDockVisible->setChecked(true);
    deckDock->setVisible(aDeckDockVisible->isChecked());
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
    DeckLoader *const deck = deckModel->getDeckList();
    return !modified && deck->getLastFileName().isEmpty() && deck->getLastRemoteDeckId() == -1;
}