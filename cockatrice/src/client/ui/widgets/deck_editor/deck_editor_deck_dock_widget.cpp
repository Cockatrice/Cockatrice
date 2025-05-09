#include "deck_editor_deck_dock_widget.h"

#include "../../../../game/cards/card_database_manager.h"
#include "../../../../settings/cache_settings.h"

#include <QComboBox>
#include <QDockWidget>
#include <QHeaderView>
#include <QLabel>
#include <QSplitter>
#include <QTextEdit>
#include <trice_limits.h>

DeckEditorDeckDockWidget::DeckEditorDeckDockWidget(AbstractTabDeckEditor *parent)
    : QDockWidget(parent), deckEditor(parent)
{
    setObjectName("deckDock");

    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);

    installEventFilter(deckEditor);
    connect(this, &DeckEditorDeckDockWidget::topLevelChanged, deckEditor, &AbstractTabDeckEditor::dockTopLevelChanged);
    createDeckDock();
}

void DeckEditorDeckDockWidget::createDeckDock()
{
    deckModel = new DeckListModel(this);
    deckModel->setObjectName("deckModel");
    connect(deckModel, &DeckListModel::deckHashChanged, this, &DeckEditorDeckDockWidget::updateHash);
    deckView = new QTreeView();
    deckView->setObjectName("deckView");
    deckView->setModel(deckModel);
    deckView->setUniformRowHeights(true);
    deckView->setSortingEnabled(true);
    deckView->sortByColumn(1, Qt::AscendingOrder);
    deckView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    deckView->installEventFilter(&deckViewKeySignals);
    deckView->setContextMenuPolicy(Qt::CustomContextMenu);
    deckView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    connect(deckView->selectionModel(), &QItemSelectionModel::currentRowChanged, this,
            &DeckEditorDeckDockWidget::updateCard);
    connect(deckView, &QTreeView::doubleClicked, this, &DeckEditorDeckDockWidget::actSwapCard);
    connect(deckView, &QTreeView::customContextMenuRequested, this, &DeckEditorDeckDockWidget::decklistCustomMenu);
    connect(&deckViewKeySignals, &KeySignals::onShiftS, this, &DeckEditorDeckDockWidget::actSwapCard);
    connect(&deckViewKeySignals, &KeySignals::onEnter, this, &DeckEditorDeckDockWidget::actIncrement);
    connect(&deckViewKeySignals, &KeySignals::onCtrlAltEqual, this, &DeckEditorDeckDockWidget::actIncrement);
    connect(&deckViewKeySignals, &KeySignals::onCtrlAltMinus, this, &DeckEditorDeckDockWidget::actDecrementSelection);
    connect(&deckViewKeySignals, &KeySignals::onShiftRight, this, &DeckEditorDeckDockWidget::actIncrement);
    connect(&deckViewKeySignals, &KeySignals::onShiftLeft, this, &DeckEditorDeckDockWidget::actDecrementSelection);
    connect(&deckViewKeySignals, &KeySignals::onDelete, this, &DeckEditorDeckDockWidget::actRemoveCard);

    nameLabel = new QLabel();
    nameLabel->setObjectName("nameLabel");
    nameEdit = new LineEditUnfocusable;
    nameEdit->setMaxLength(MAX_NAME_LENGTH);
    nameEdit->setObjectName("nameEdit");
    nameLabel->setBuddy(nameEdit);
    connect(nameEdit, &LineEditUnfocusable::textChanged, this, &DeckEditorDeckDockWidget::updateName);

    quickSettingsWidget = new SettingsButtonWidget(this);

    showBannerCardCheckBox = new QCheckBox();
    showBannerCardCheckBox->setObjectName("showBannerCardCheckBox");
    showBannerCardCheckBox->setChecked(SettingsCache::instance().getDeckEditorBannerCardComboBoxVisible());
    connect(showBannerCardCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance(),
            &SettingsCache::setDeckEditorBannerCardComboBoxVisible);
    connect(&SettingsCache::instance(), &SettingsCache::deckEditorBannerCardComboBoxVisibleChanged, this,
            &DeckEditorDeckDockWidget::updateShowBannerCardComboBox);

    showTagsWidgetCheckBox = new QCheckBox();
    showTagsWidgetCheckBox->setObjectName("showTagsWidgetCheckBox");
    showTagsWidgetCheckBox->setChecked(SettingsCache::instance().getDeckEditorTagsWidgetVisible());
    connect(showTagsWidgetCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance(),
            &SettingsCache::setDeckEditorTagsWidgetVisible);
    connect(&SettingsCache::instance(), &SettingsCache::deckEditorTagsWidgetVisibleChanged, this,
            &DeckEditorDeckDockWidget::updateShowTagsWidget);

    quickSettingsWidget->addSettingsWidget(showBannerCardCheckBox);
    quickSettingsWidget->addSettingsWidget(showTagsWidgetCheckBox);

    commentsLabel = new QLabel();
    commentsLabel->setObjectName("commentsLabel");
    commentsEdit = new QTextEdit;
    commentsEdit->setAcceptRichText(false);
    commentsEdit->setMinimumHeight(nameEdit->minimumSizeHint().height());
    commentsEdit->setObjectName("commentsEdit");
    commentsLabel->setBuddy(commentsEdit);
    connect(commentsEdit, &QTextEdit::textChanged, this, &DeckEditorDeckDockWidget::updateComments);
    bannerCardLabel = new QLabel();
    bannerCardLabel->setObjectName("bannerCardLabel");
    bannerCardLabel->setText(tr("Banner Card"));
    bannerCardLabel->setHidden(!SettingsCache::instance().getDeckEditorBannerCardComboBoxVisible());
    bannerCardComboBox = new QComboBox(this);
    connect(deckModel, &DeckListModel::dataChanged, this, [this]() {
        // Delay the update to avoid race conditions
        QTimer::singleShot(100, this, &DeckEditorDeckDockWidget::updateBannerCardComboBox);
    });
    connect(bannerCardComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &DeckEditorDeckDockWidget::setBannerCard);
    bannerCardComboBox->setHidden(!SettingsCache::instance().getDeckEditorBannerCardComboBoxVisible());

    deckTagsDisplayWidget = new DeckPreviewDeckTagsDisplayWidget(this, deckModel->getDeckList());
    deckTagsDisplayWidget->setHidden(!SettingsCache::instance().getDeckEditorTagsWidgetVisible());

    aIncrement = new QAction(QString(), this);
    aIncrement->setIcon(QPixmap("theme:icons/increment"));
    connect(aIncrement, &QAction::triggered, this, &DeckEditorDeckDockWidget::actIncrement);
    auto *tbIncrement = new QToolButton(this);
    tbIncrement->setDefaultAction(aIncrement);

    aDecrement = new QAction(QString(), this);
    aDecrement->setIcon(QPixmap("theme:icons/decrement"));
    connect(aDecrement, &QAction::triggered, this, &DeckEditorDeckDockWidget::actDecrementSelection);
    auto *tbDecrement = new QToolButton(this);
    tbDecrement->setDefaultAction(aDecrement);

    aRemoveCard = new QAction(QString(), this);
    aRemoveCard->setIcon(QPixmap("theme:icons/remove_row"));
    connect(aRemoveCard, &QAction::triggered, this, &DeckEditorDeckDockWidget::actRemoveCard);
    auto *tbRemoveCard = new QToolButton(this);
    tbRemoveCard->setDefaultAction(aRemoveCard);

    aSwapCard = new QAction(QString(), this);
    aSwapCard->setIcon(QPixmap("theme:icons/swap"));
    connect(aSwapCard, &QAction::triggered, this, &DeckEditorDeckDockWidget::actSwapCard);
    auto *tbSwapCard = new QToolButton(this);
    tbSwapCard->setDefaultAction(aSwapCard);

    auto *upperLayout = new QGridLayout;
    upperLayout->setObjectName("upperLayout");
    upperLayout->setContentsMargins(11, 11, 11, 0);

    upperLayout->addWidget(nameLabel, 0, 0);
    upperLayout->addWidget(nameEdit, 0, 1);
    upperLayout->addWidget(quickSettingsWidget, 0, 2);

    upperLayout->addWidget(commentsLabel, 1, 0);
    upperLayout->addWidget(commentsEdit, 1, 1);

    upperLayout->addWidget(bannerCardLabel, 2, 0);
    upperLayout->addWidget(bannerCardComboBox, 2, 1);

    upperLayout->addWidget(deckTagsDisplayWidget, 3, 1);

    hashLabel1 = new QLabel();
    hashLabel1->setObjectName("hashLabel1");
    auto *hashSizePolicy = new QSizePolicy();
    hashSizePolicy->setHorizontalPolicy(QSizePolicy::Fixed);
    hashLabel1->setSizePolicy(*hashSizePolicy);
    hashLabel = new LineEditUnfocusable;
    hashLabel->setObjectName("hashLabel");
    hashLabel->setReadOnly(true);
    hashLabel->setFrame(false);

    auto *lowerLayout = new QGridLayout;
    lowerLayout->setObjectName("lowerLayout");
    lowerLayout->addWidget(hashLabel1, 0, 0);
    lowerLayout->addWidget(hashLabel, 0, 1);
    lowerLayout->addWidget(tbIncrement, 0, 2);
    lowerLayout->addWidget(tbDecrement, 0, 3);
    lowerLayout->addWidget(tbRemoveCard, 0, 4);
    lowerLayout->addWidget(tbSwapCard, 0, 5);
    lowerLayout->addWidget(deckView, 1, 0, 1, 6);

    // Create widgets for both layouts to make splitter work correctly
    auto *topWidget = new QWidget;
    topWidget->setLayout(upperLayout);
    auto *bottomWidget = new QWidget;
    bottomWidget->setLayout(lowerLayout);

    auto *split = new QSplitter;
    split->setObjectName("deckSplitter");
    split->setOrientation(Qt::Vertical);
    split->setChildrenCollapsible(true);
    split->addWidget(topWidget);
    split->addWidget(bottomWidget);
    split->setStretchFactor(0, 1);
    split->setStretchFactor(1, 4);

    auto *rightFrame = new QVBoxLayout;
    rightFrame->setObjectName("rightFrame");
    rightFrame->addWidget(split);

    auto *deckDockContents = new QWidget();
    deckDockContents->setObjectName("deckDockContents");
    deckDockContents->setLayout(rightFrame);
    setWidget(deckDockContents);

    refreshShortcuts();
    retranslateUi();
}

CardInfoPtr DeckEditorDeckDockWidget::getCurrentCard()
{
    QModelIndex current = deckView->selectionModel()->currentIndex();
    if (!current.isValid())
        return {};
    const QString cardName = current.sibling(current.row(), 1).data().toString();
    const QString cardProviderID = current.sibling(current.row(), 4).data().toString();
    const QModelIndex gparent = current.parent().parent();

    if (!gparent.isValid()) {
        return {};
    }

    const QString zoneName = gparent.sibling(gparent.row(), 1).data(Qt::EditRole).toString();

    if (!current.model()->hasChildren(current.sibling(current.row(), 0))) {
        QString cardName = current.sibling(current.row(), 1).data().toString();
        QString providerId = current.sibling(current.row(), 4).data().toString();
        if (CardInfoPtr selectedCard =
                CardDatabaseManager::getInstance()->getCardByNameAndProviderId(cardName, providerId)) {
            return selectedCard;
        }
    }
    return {};
}

void DeckEditorDeckDockWidget::updateCard(const QModelIndex /*&current*/, const QModelIndex & /*previous*/)
{
    if (CardInfoPtr card = getCurrentCard()) {
        emit cardChanged(card);
    }
}

void DeckEditorDeckDockWidget::updateName(const QString &name)
{
    deckModel->getDeckList()->setName(name);
    emit nameChanged();
    emit deckModified();
}

void DeckEditorDeckDockWidget::updateComments()
{
    deckModel->getDeckList()->setComments(commentsEdit->toPlainText());
    emit commentsChanged();
    emit deckModified();
}

void DeckEditorDeckDockWidget::updateHash()
{
    hashLabel->setText(deckModel->getDeckList()->getDeckHash());
    emit hashChanged();
    emit deckModified();
}

void DeckEditorDeckDockWidget::updateBannerCardComboBox()
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
        if (deckModel->getDeckList()->getBannerCard().second !=
            bannerCardComboBox->itemData(bannerCardComboBox->currentIndex()).toMap()["uuid"].toString()) {
            setBannerCard(restoredIndex);
        }
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

void DeckEditorDeckDockWidget::setBannerCard(int /* changedIndex */)
{
    QVariantMap itemData = bannerCardComboBox->itemData(bannerCardComboBox->currentIndex()).toMap();
    deckModel->getDeckList()->setBannerCard(
        QPair<QString, QString>(itemData["name"].toString(), itemData["uuid"].toString()));
    emit deckModified();
}

void DeckEditorDeckDockWidget::updateShowBannerCardComboBox(const bool visible)
{
    bannerCardLabel->setHidden(!visible);
    bannerCardComboBox->setHidden(!visible);
}

void DeckEditorDeckDockWidget::updateShowTagsWidget(const bool visible)
{
    deckTagsDisplayWidget->setHidden(!visible);
}

/**
 * Sets the currently active deck for this tab
 * @param _deck The deck. Takes ownership of the object
 */
void DeckEditorDeckDockWidget::setDeck(DeckLoader *_deck)
{
    deckModel->setDeckList(_deck);

    nameEdit->setText(deckModel->getDeckList()->getName());
    commentsEdit->setText(deckModel->getDeckList()->getComments());
    bannerCardComboBox->setCurrentText(deckModel->getDeckList()->getBannerCard().first);
    updateBannerCardComboBox();
    updateHash();
    deckModel->sort(deckView->header()->sortIndicatorSection(), deckView->header()->sortIndicatorOrder());
    deckView->expandAll();
    deckView->expandAll();

    deckTagsDisplayWidget->connectDeckList(deckModel->getDeckList());

    emit deckChanged();
}

DeckLoader *DeckEditorDeckDockWidget::getDeckList()
{
    return deckModel->getDeckList();
}

/**
 * Resets the tab to the state for a blank new tab.
 */
void DeckEditorDeckDockWidget::cleanDeck()
{
    deckModel->cleanList();
    nameEdit->setText(QString());
    emit nameChanged();
    commentsEdit->setText(QString());
    emit commentsChanged();
    hashLabel->setText(QString());
    emit hashChanged();
    emit deckModified();
    emit deckChanged();
    updateBannerCardComboBox();
    deckTagsDisplayWidget->connectDeckList(deckModel->getDeckList());
}

void DeckEditorDeckDockWidget::recursiveExpand(const QModelIndex &index)
{
    if (index.parent().isValid())
        recursiveExpand(index.parent());
    deckView->expand(index);
}

/**
 * Gets the index of all the currently selected card nodes in the decklist table.
 * The list is in reverse order of the visual selection, so that rows can be deleted while iterating over them.
 *
 * @return A model index list containing all selected card nodes
 */
QModelIndexList DeckEditorDeckDockWidget::getSelectedCardNodes() const
{
    auto selectedRows = deckView->selectionModel()->selectedRows();

    const auto notLeafNode = [this](const auto &index) { return deckModel->hasChildren(index); };
    selectedRows.erase(std::remove_if(selectedRows.begin(), selectedRows.end(), notLeafNode), selectedRows.end());

    std::reverse(selectedRows.begin(), selectedRows.end());
    return selectedRows;
}

void DeckEditorDeckDockWidget::actIncrement()
{
    auto selectedRows = getSelectedCardNodes();

    for (const auto &index : selectedRows) {
        offsetCountAtIndex(index, 1);
    }
}

void DeckEditorDeckDockWidget::actSwapCard()
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
        emit deckModified();
    }

    update();
}

/**
 * Swaps the card at the index between the maindeck and sideboard
 *
 * @param currentIndex The index to swap.
 * @return True if the swap was successful
 */
bool DeckEditorDeckDockWidget::swapCard(const QModelIndex &currentIndex)
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

void DeckEditorDeckDockWidget::actDecrementCard(CardInfoPtr info, QString zoneName)
{
    if (!info)
        return;
    if (info->getIsToken())
        zoneName = DECK_ZONE_TOKENS;

    QString providerId = CardDatabaseManager::getInstance()->getSetInfoForCard(info).getProperty("uuid");
    QString collectorNumber = CardDatabaseManager::getInstance()->getSetInfoForCard(info).getProperty("num");

    QModelIndex idx = deckModel->findCard(info->getName(), zoneName, providerId, collectorNumber);
    if (!idx.isValid()) {
        return;
    }

    deckView->clearSelection();
    deckView->setCurrentIndex(idx);
    offsetCountAtIndex(idx, -1);
}

void DeckEditorDeckDockWidget::actDecrementSelection()
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

void DeckEditorDeckDockWidget::actRemoveCard()
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
        emit deckModified();
    }
}

void DeckEditorDeckDockWidget::offsetCountAtIndex(const QModelIndex &idx, int offset)
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

    emit deckModified();
}

void DeckEditorDeckDockWidget::decklistCustomMenu(QPoint point)
{
    QMenu menu;
    const CardInfoPtr info = getCurrentCard();

    QAction *selectPrinting = menu.addAction(tr("Select Printing"));

    connect(selectPrinting, &QAction::triggered, deckEditor, &AbstractTabDeckEditor::showPrintingSelector);

    menu.exec(deckView->mapToGlobal(point));
}

void DeckEditorDeckDockWidget::refreshShortcuts()
{
    ShortcutsSettings &shortcuts = SettingsCache::instance().shortcuts();
    aRemoveCard->setShortcuts(shortcuts.getShortcut("TabDeckEditor/aRemoveCard"));
    aIncrement->setShortcuts(shortcuts.getShortcut("TabDeckEditor/aIncrement"));
    aDecrement->setShortcuts(shortcuts.getShortcut("TabDeckEditor/aDecrement"));
}

void DeckEditorDeckDockWidget::retranslateUi()
{
    setWindowTitle(tr("Deck"));

    nameLabel->setText(tr("Deck &name:"));
    quickSettingsWidget->setToolTip(tr("Banner Card/Tags Visibility Settings"));
    showBannerCardCheckBox->setText(tr("Show banner card selection menu"));
    showTagsWidgetCheckBox->setText(tr("Show tags selection menu"));
    commentsLabel->setText(tr("&Comments:"));
    hashLabel1->setText(tr("Hash:"));

    aIncrement->setText(tr("&Increment number"));
    aDecrement->setText(tr("&Decrement number"));
    aRemoveCard->setText(tr("&Remove row"));
    aSwapCard->setText(tr("Swap card to/from sideboard"));
}