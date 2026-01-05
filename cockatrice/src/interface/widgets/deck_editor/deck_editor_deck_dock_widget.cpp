#include "deck_editor_deck_dock_widget.h"

#include "../../../client/settings/cache_settings.h"
#include "deck_list_style_proxy.h"
#include "deck_state_manager.h"

#include <QComboBox>
#include <QDockWidget>
#include <QHeaderView>
#include <QLabel>
#include <QSplitter>
#include <QTextEdit>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/utility/trice_limits.h>

static int findRestoreIndex(const CardRef &wanted, const QComboBox *combo)
{
    // Try providerId + name (strongest match)
    if (!wanted.providerId.isEmpty()) {
        for (int i = 0; i < combo->count(); ++i) {
            auto pair = combo->itemData(i).value<QPair<QString, QString>>();
            if (pair.second == wanted.providerId && pair.first == wanted.name) {
                return i;
            }
        }
    }

    // Try name only
    for (int i = 0; i < combo->count(); ++i) {
        auto pair = combo->itemData(i).value<QPair<QString, QString>>();
        if (pair.first == wanted.name) {
            return i;
        }
    }

    // Not found
    return -1;
}

DeckEditorDeckDockWidget::DeckEditorDeckDockWidget(AbstractTabDeckEditor *parent)
    : QDockWidget(parent), deckEditor(parent), deckStateManager(parent->deckStateManager)
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
    connect(getModel(), &DeckListModel::deckHashChanged, this, &DeckEditorDeckDockWidget::updateHash);

    proxy = new DeckListStyleProxy(this);
    proxy->setSourceModel(getModel());

    historyManagerWidget = new DeckListHistoryManagerWidget(deckStateManager, proxy, this);
    connect(historyManagerWidget, &DeckListHistoryManagerWidget::requestDisplayWidgetSync, this,
            &DeckEditorDeckDockWidget::syncDisplayWidgetsToModel);

    connect(deckStateManager, &DeckStateManager::focusIndexChanged, this, &DeckEditorDeckDockWidget::setSelectedIndex);
    connect(deckStateManager, &DeckStateManager::deckReplaced, this,
            &DeckEditorDeckDockWidget::syncDisplayWidgetsToModel);
    connect(deckStateManager, &DeckStateManager::deckReplaced, this,
            &DeckEditorDeckDockWidget::applyActiveGroupCriteria);

    deckView = new QTreeView();
    deckView->setObjectName("deckView");
    deckView->setModel(proxy);
    deckView->setUniformRowHeights(true);
    deckView->setSortingEnabled(true);
    deckView->sortByColumn(1, Qt::AscendingOrder);
    deckView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    deckView->installEventFilter(&deckViewKeySignals);
    deckView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    connect(deckView->selectionModel(), &QItemSelectionModel::currentRowChanged, this,
            &DeckEditorDeckDockWidget::updateCard);
    connect(deckView, &QTreeView::doubleClicked, this, &DeckEditorDeckDockWidget::actSwapSelection);
    deckView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(deckView, &QTreeView::customContextMenuRequested, this, &DeckEditorDeckDockWidget::decklistCustomMenu);
    connect(&deckViewKeySignals, &KeySignals::onShiftS, this, &DeckEditorDeckDockWidget::actSwapSelection);
    connect(&deckViewKeySignals, &KeySignals::onEnter, this, &DeckEditorDeckDockWidget::actIncrementSelection);
    connect(&deckViewKeySignals, &KeySignals::onCtrlAltEqual, this, &DeckEditorDeckDockWidget::actIncrementSelection);
    connect(&deckViewKeySignals, &KeySignals::onCtrlAltMinus, this, &DeckEditorDeckDockWidget::actDecrementSelection);
    connect(&deckViewKeySignals, &KeySignals::onShiftRight, this, &DeckEditorDeckDockWidget::actIncrementSelection);
    connect(&deckViewKeySignals, &KeySignals::onShiftLeft, this, &DeckEditorDeckDockWidget::actDecrementSelection);
    connect(&deckViewKeySignals, &KeySignals::onDelete, this, &DeckEditorDeckDockWidget::actRemoveCard);

    nameLabel = new QLabel();
    nameLabel->setObjectName("nameLabel");
    nameEdit = new LineEditUnfocusable;
    nameEdit->setMaxLength(MAX_NAME_LENGTH);
    nameEdit->setObjectName("nameEdit");
    nameLabel->setBuddy(nameEdit);

    nameDebounceTimer = new QTimer(this);
    nameDebounceTimer->setSingleShot(true);
    nameDebounceTimer->setInterval(300); // debounce duration in ms
    connect(nameDebounceTimer, &QTimer::timeout, this, &DeckEditorDeckDockWidget::writeName);

    connect(nameEdit, &LineEditUnfocusable::textChanged, this, [this]() {
        nameDebounceTimer->start(); // restart debounce timer
    });

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

    formatLabel = new QLabel(this);

    formatComboBox = new QComboBox(this);
    formatComboBox->addItem(tr("Loading Database..."));
    formatComboBox->setEnabled(false); // Disable until loaded

    commentsLabel = new QLabel();
    commentsLabel->setObjectName("commentsLabel");
    commentsEdit = new QTextEdit;
    commentsEdit->setAcceptRichText(false);
    commentsEdit->setMinimumHeight(nameEdit->minimumSizeHint().height());
    commentsEdit->setObjectName("commentsEdit");
    commentsLabel->setBuddy(commentsEdit);

    commentsDebounceTimer = new QTimer(this);
    commentsDebounceTimer->setSingleShot(true);
    commentsDebounceTimer->setInterval(400); // longer debounce for multi-line
    connect(commentsDebounceTimer, &QTimer::timeout, this, &DeckEditorDeckDockWidget::writeComments);

    connect(commentsEdit, &QTextEdit::textChanged, this, [this]() {
        commentsDebounceTimer->start(); // restart debounce timer
    });

    bannerCardLabel = new QLabel();
    bannerCardLabel->setObjectName("bannerCardLabel");
    bannerCardLabel->setText(tr("Banner Card"));
    bannerCardLabel->setHidden(!SettingsCache::instance().getDeckEditorBannerCardComboBoxVisible());
    bannerCardComboBox = new QComboBox(this);
    connect(getModel(), &DeckListModel::dataChanged, this, [this]() {
        // Delay the update to avoid race conditions
        QTimer::singleShot(100, this, &DeckEditorDeckDockWidget::updateBannerCardComboBox);
    });
    connect(getModel(), &DeckListModel::cardAddedAt, this, &DeckEditorDeckDockWidget::recursiveExpand);
    connect(getModel(), &DeckListModel::modelReset, this, &DeckEditorDeckDockWidget::expandAll);

    connect(bannerCardComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &DeckEditorDeckDockWidget::writeBannerCard);
    bannerCardComboBox->setHidden(!SettingsCache::instance().getDeckEditorBannerCardComboBoxVisible());

    deckTagsDisplayWidget = new DeckPreviewDeckTagsDisplayWidget(this, {});
    deckTagsDisplayWidget->setHidden(!SettingsCache::instance().getDeckEditorTagsWidgetVisible());
    connect(deckTagsDisplayWidget, &DeckPreviewDeckTagsDisplayWidget::tagsChanged, deckStateManager,
            &DeckStateManager::setTags);

    activeGroupCriteriaLabel = new QLabel(this);

    activeGroupCriteriaComboBox = new QComboBox(this);
    activeGroupCriteriaComboBox->addItem(tr("Main Type"), DeckListModelGroupCriteria::MAIN_TYPE);
    activeGroupCriteriaComboBox->addItem(tr("Mana Cost"), DeckListModelGroupCriteria::MANA_COST);
    activeGroupCriteriaComboBox->addItem(tr("Colors"), DeckListModelGroupCriteria::COLOR);
    connect(activeGroupCriteriaComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this,
            &DeckEditorDeckDockWidget::applyActiveGroupCriteria);

    aIncrement = new QAction(QString(), this);
    aIncrement->setIcon(QPixmap("theme:icons/increment"));
    connect(aIncrement, &QAction::triggered, this, &DeckEditorDeckDockWidget::actIncrementSelection);
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
    connect(aSwapCard, &QAction::triggered, this, &DeckEditorDeckDockWidget::actSwapSelection);
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

    upperLayout->addWidget(formatLabel, 2, 0);
    upperLayout->addWidget(formatComboBox, 2, 1);

    upperLayout->addWidget(bannerCardLabel, 3, 0);
    upperLayout->addWidget(bannerCardComboBox, 3, 1);

    upperLayout->addWidget(deckTagsDisplayWidget, 4, 1);

    upperLayout->addWidget(activeGroupCriteriaLabel, 5, 0);
    upperLayout->addWidget(activeGroupCriteriaComboBox, 5, 1);

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
    lowerLayout->addWidget(historyManagerWidget, 0, 6);
    lowerLayout->addWidget(deckView, 1, 0, 1, 7);

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

    connect(CardDatabaseManager::getInstance(), &CardDatabase::cardDatabaseLoadingFinished, this,
            &DeckEditorDeckDockWidget::initializeFormats);

    if (CardDatabaseManager::getInstance()->getLoadStatus() == LoadStatus::Ok) {
        initializeFormats();
    }
}

void DeckEditorDeckDockWidget::initializeFormats()
{
    QStringList allFormats = CardDatabaseManager::query()->getAllFormatsWithCount().keys();

    formatComboBox->clear(); // Remove "Loading Database..."
    formatComboBox->setEnabled(true);

    // Populate with formats
    formatComboBox->addItem("", "");
    for (auto formatName : allFormats) {
        formatComboBox->addItem(formatName, formatName); // store the raw key in itemData
    }

    QString format = deckStateManager->getMetadata().gameFormat;
    if (!format.isEmpty()) {
        formatComboBox->setCurrentIndex(formatComboBox->findData(format));
    } else {
        // Ensure no selection is visible initially
        formatComboBox->setCurrentIndex(-1);
    }

    connect(formatComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        if (index >= 0) {
            QString formatKey = formatComboBox->itemData(index).toString();
            deckStateManager->setFormat(formatKey);
        } else {
            deckStateManager->setFormat(""); // clear format if deselected
        }
    });
}

ExactCard DeckEditorDeckDockWidget::getCurrentCard()
{
    QModelIndex current = deckView->selectionModel()->currentIndex();
    if (!current.isValid())
        return {};
    const QString cardName = current.siblingAtColumn(DeckListModelColumns::CARD_NAME).data().toString();
    const QString cardProviderID = current.siblingAtColumn(DeckListModelColumns::CARD_PROVIDER_ID).data().toString();
    const QModelIndex gparent = current.parent().parent();

    if (!gparent.isValid()) {
        return {};
    }

    const QString zoneName = gparent.siblingAtColumn(DeckListModelColumns::CARD_NAME).data(Qt::EditRole).toString();

    if (!current.model()->hasChildren(current.siblingAtColumn(DeckListModelColumns::CARD_AMOUNT))) {
        if (ExactCard selectedCard = CardDatabaseManager::query()->getCard({cardName, cardProviderID})) {
            return selectedCard;
        }
    }
    return {};
}

void DeckEditorDeckDockWidget::updateCard(const QModelIndex /*&current*/, const QModelIndex & /*previous*/)
{
    if (ExactCard card = getCurrentCard()) {
        emit selectedCardChanged(card);
    }
}

/**
 * @brief Writes the contents of the name textBox to the DeckStateManager
 */
void DeckEditorDeckDockWidget::writeName()
{
    QString name = nameEdit->text();
    deckStateManager->setName(name);
}

/**
 * @brief Writes the contents of the comments textBox to the DeckStateManager
 */
void DeckEditorDeckDockWidget::writeComments()
{
    QString comments = commentsEdit->toPlainText();
    deckStateManager->setComments(comments);
}

void DeckEditorDeckDockWidget::updateHash()
{
    hashLabel->setText(deckStateManager->getDeckHash());
}

void DeckEditorDeckDockWidget::updateBannerCardComboBox()
{
    // Store current banner card identity
    CardRef wanted = deckStateManager->getMetadata().bannerCard;

    // Block signals temporarily
    bool wasBlocked = bannerCardComboBox->blockSignals(true);

    // Clear the existing items in the combo box
    bannerCardComboBox->clear();

    // Collect unique (name, providerId) pairs
    QSet<QPair<QString, QString>> bannerCardSet;
    QList<CardRef> cardsInDeck = getModel()->getCardRefs();

    for (auto cardRef : cardsInDeck) {
        if (!CardDatabaseManager::query()->getCard(cardRef)) {
            continue;
        }

        // Insert one entry per distinct card, ignore copies
        bannerCardSet.insert({cardRef.name, cardRef.providerId});
    }

    // Convert to sorted list
    QList<QPair<QString, QString>> pairList = bannerCardSet.values();

    // Sort QList by the first() element of the QPair
    std::sort(pairList.begin(), pairList.end(),
              [](const auto &a, const auto &b) { return a.first.toLower() < b.first.toLower(); });

    // Add to combo box
    for (const auto &pair : pairList) {
        bannerCardComboBox->addItem(pair.first, QVariant::fromValue(pair));
    }

    // Try to find an index with a matching card
    int restoreIndex = findRestoreIndex(wanted, bannerCardComboBox);

    // Handle results
    if (restoreIndex != -1) {
        bannerCardComboBox->setCurrentIndex(restoreIndex);
    } else {
        // Add a placeholder "-" and set it as the current selection
        bannerCardComboBox->insertItem(0, "-");
        bannerCardComboBox->setCurrentIndex(0);
    }

    // Restore signal state
    bannerCardComboBox->blockSignals(wasBlocked);
}

/**
 * @brief Writes the selected bannerCard to the DeckStateManager
 */
void DeckEditorDeckDockWidget::writeBannerCard(int index)
{
    auto [name, id] = bannerCardComboBox->itemData(index).value<QPair<QString, QString>>();
    CardRef bannerCard = {name, id};
    deckStateManager->setBannerCard(bannerCard);
}

void DeckEditorDeckDockWidget::applyActiveGroupCriteria()
{
    getModel()->setActiveGroupCriteria(
        static_cast<DeckListModelGroupCriteria::Type>(activeGroupCriteriaComboBox->currentData(Qt::UserRole).toInt()));
    getModel()->sort(deckView->header()->sortIndicatorSection(), deckView->header()->sortIndicatorOrder());
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

void DeckEditorDeckDockWidget::syncBannerCardComboBoxSelectionWithDeck()
{
    if (deckStateManager->getMetadata().bannerCard.name == "") {
        if (bannerCardComboBox->findText("-") != -1) {
            bannerCardComboBox->setCurrentIndex(bannerCardComboBox->findText("-"));
        } else {
            bannerCardComboBox->insertItem(0, "-");
            bannerCardComboBox->setCurrentIndex(0);
        }
    } else {
        bannerCardComboBox->setCurrentText(deckStateManager->getMetadata().bannerCard.name);
    }
}

void DeckEditorDeckDockWidget::setSelectedIndex(const QModelIndex &newCardIndex)
{
    deckView->clearSelection();
    deckView->setCurrentIndex(newCardIndex);
    recursiveExpand(newCardIndex);
    deckView->setFocus(Qt::FocusReason::MouseFocusReason);
}

void DeckEditorDeckDockWidget::syncDisplayWidgetsToModel()
{
    nameEdit->blockSignals(true);
    nameEdit->setText(deckStateManager->getMetadata().name);
    nameEdit->blockSignals(false);

    commentsEdit->blockSignals(true);
    commentsEdit->setText(deckStateManager->getMetadata().comments);
    commentsEdit->blockSignals(false);

    bannerCardComboBox->blockSignals(true);
    syncBannerCardComboBoxSelectionWithDeck();
    updateBannerCardComboBox();
    bannerCardComboBox->blockSignals(false);
    updateHash();

    formatComboBox->blockSignals(true);
    formatComboBox->setCurrentIndex(formatComboBox->findData(deckStateManager->getMetadata().gameFormat));
    formatComboBox->blockSignals(false);

    deckTagsDisplayWidget->blockSignals(true);
    deckTagsDisplayWidget->setTags(deckStateManager->getMetadata().tags);
    deckTagsDisplayWidget->blockSignals(false);
}

/**
 * @brief Convenience method to get the underlying model instance from the DeckStateManager
 */
DeckListModel *DeckEditorDeckDockWidget::getModel() const
{
    return deckStateManager->getModel();
}

void DeckEditorDeckDockWidget::selectPrevCard()
{
    changeSelectedCard(-1);
}

void DeckEditorDeckDockWidget::selectNextCard()
{
    changeSelectedCard(1);
}

/**
 * @brief Selects a card based on the change direction.
 *
 * @param changeBy The direction to change, -1 for previous, 1 for next.
 */
void DeckEditorDeckDockWidget::changeSelectedCard(int changeBy)
{
    if (changeBy == 0) {
        return;
    }

    // Get the current index of the selected item
    auto deckViewCurrentIndex = deckView->currentIndex();

    // For some reason, if the deckModel is modified but the view is not manually reselected,
    // currentIndex will return an index for the underlying deckModel instead of the proxy.
    // That index will return an invalid index when indexBelow/indexAbove crosses a header node,
    // causing the selection to fail to move down.
    /// \todo Figure out why it's happening so we can do a proper fix instead of a hacky workaround
    if (deckViewCurrentIndex.model() == proxy->sourceModel()) {
        deckViewCurrentIndex = proxy->mapFromSource(deckViewCurrentIndex);
    }

    auto nextIndex = deckViewCurrentIndex.siblingAtRow(deckViewCurrentIndex.row() + changeBy);
    if (!nextIndex.isValid()) {
        nextIndex = deckViewCurrentIndex;

        // Increment to the next valid index, skipping header rows
        AbstractDecklistNode *node;
        do {
            if (changeBy > 0) {
                nextIndex = deckView->indexBelow(nextIndex);
            } else {
                nextIndex = deckView->indexAbove(nextIndex);
            }
            node = static_cast<AbstractDecklistNode *>(nextIndex.internalPointer());
        } while (node && node->isDeckHeader());
    }

    if (nextIndex.isValid()) {
        deckView->setCurrentIndex(nextIndex);
        deckView->setFocus(Qt::FocusReason::MouseFocusReason);
    }
}

/**
 * @brief Expands all parents of the given index.
 * @param sourceIndex The index to expand (model source index)
 */
void DeckEditorDeckDockWidget::recursiveExpand(const QModelIndex &sourceIndex)
{
    auto index = proxy->mapFromSource(sourceIndex);

    while (index.parent().isValid()) {
        index = index.parent();
        deckView->expand(index);
    }
}

/**
 * @brief Fully expands all levels of the deck view
 */
void DeckEditorDeckDockWidget::expandAll()
{
    deckView->expandRecursively(deckView->rootIndex());
}

/**
 * Gets the source index of all the currently selected card nodes in the decklist table.
 * The list is in reverse order of the visual selection, so that rows can be deleted while iterating over them.
 *
 * @return A list containing the source indices of all selected card nodes
 */
QModelIndexList DeckEditorDeckDockWidget::getSelectedCardNodeSourceIndices() const
{
    auto selectedRows = deckView->selectionModel()->selectedRows();

    const auto mapToSource = [this](const QModelIndex &index) { return proxy->mapToSource(index); };
    std::transform(selectedRows.begin(), selectedRows.end(), selectedRows.begin(), mapToSource);

    const auto notLeafNode = [this](const QModelIndex &sourceIndex) { return getModel()->hasChildren(sourceIndex); };
    selectedRows.erase(std::remove_if(selectedRows.begin(), selectedRows.end(), notLeafNode), selectedRows.end());

    std::reverse(selectedRows.begin(), selectedRows.end());
    return selectedRows;
}

void DeckEditorDeckDockWidget::actAddCard(const ExactCard &card, const QString &zoneName)
{
    if (!card) {
        return;
    }

    deckStateManager->addCard(card, zoneName);
}

void DeckEditorDeckDockWidget::actIncrementSelection()
{
    auto selectedRows = getSelectedCardNodeSourceIndices();

    for (const auto &sourceIndex : selectedRows) {
        offsetCountAtIndex(sourceIndex, true);
    }
}

void DeckEditorDeckDockWidget::actSwapCard(const ExactCard &card, const QString &zoneName)
{
    QString providerId = card.getPrinting().getUuid();
    QString collectorNumber = card.getPrinting().getProperty("num");

    QModelIndex foundCard = getModel()->findCard(card.getName(), zoneName, providerId, collectorNumber);
    if (!foundCard.isValid()) {
        foundCard = getModel()->findCard(card.getName(), zoneName);
    }

    deckStateManager->swapCardAtIndex(foundCard);
}

void DeckEditorDeckDockWidget::actSwapSelection()
{
    auto selectedRows = getSelectedCardNodeSourceIndices();

    // hack to maintain the old reselection behavior when currently selected row of a single-selection gets deleted
    // TODO: remove the hack and also handle reselection when all rows of a multi-selection gets deleted
    if (selectedRows.length() == 1) {
        deckView->setSelectionMode(QAbstractItemView::SingleSelection);
    }

    for (const auto &sourceIndex : selectedRows) {
        deckStateManager->swapCardAtIndex(sourceIndex);
    }

    deckView->setSelectionMode(QAbstractItemView::ExtendedSelection);

    update();
}

void DeckEditorDeckDockWidget::actDecrementCard(const ExactCard &card, QString zoneName)
{
    if (!card)
        return;
    if (card.getInfo().getIsToken())
        zoneName = DECK_ZONE_TOKENS;

    deckStateManager->decrementCard(card, zoneName);
}

void DeckEditorDeckDockWidget::actDecrementSelection()
{
    auto selectedRows = getSelectedCardNodeSourceIndices();

    // hack to maintain the old reselection behavior when currently selected row of a single-selection gets deleted
    // TODO: remove the hack and also handle reselection when all rows of a multi-selection gets deleted
    if (selectedRows.length() == 1) {
        deckView->setSelectionMode(QAbstractItemView::SingleSelection);
    }

    for (const auto &sourceIndex : selectedRows) {
        offsetCountAtIndex(sourceIndex, false);
    }

    deckView->setSelectionMode(QAbstractItemView::ExtendedSelection);
}

void DeckEditorDeckDockWidget::actRemoveCard()
{
    auto selectedRows = getSelectedCardNodeSourceIndices();

    // hack to maintain the old reselection behavior when currently selected row of a single-selection gets deleted
    // TODO: remove the hack and also handle reselection when all rows of a multi-selection gets deleted
    if (selectedRows.length() == 1) {
        deckView->setSelectionMode(QAbstractItemView::SingleSelection);
    }

    for (const auto &sourceIndex : selectedRows) {
        deckStateManager->removeCardAtIndex(sourceIndex);
    }

    deckView->setSelectionMode(QAbstractItemView::ExtendedSelection);
}

/**
 * @brief Increments or decrements the amount of the card node at the index by 1.
 * @param idx The source index
 * @param isIncrement If true, increments the count. If false, decrements the count
 */
void DeckEditorDeckDockWidget::offsetCountAtIndex(const QModelIndex &idx, bool isIncrement)
{
    if (!idx.isValid() || getModel()->hasChildren(idx)) {
        return;
    }

    if (isIncrement) {
        deckStateManager->incrementCountAtIndex(idx);
    } else {
        deckStateManager->decrementCountAtIndex(idx);
    }
}

void DeckEditorDeckDockWidget::decklistCustomMenu(QPoint point)
{
    if (!SettingsCache::instance().getOverrideAllCardArtWithPersonalPreference()) {
        QMenu menu;

        QAction *selectPrinting = menu.addAction(tr("Select Printing"));
        connect(selectPrinting, &QAction::triggered, deckEditor, &AbstractTabDeckEditor::showPrintingSelector);

        menu.exec(deckView->mapToGlobal(point));
    }
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
    activeGroupCriteriaLabel->setText(tr("Group by:"));
    formatLabel->setText(tr("Format:"));

    hashLabel1->setText(tr("Hash:"));

    aIncrement->setText(tr("&Increment number"));
    aDecrement->setText(tr("&Decrement number"));
    aRemoveCard->setText(tr("&Remove row"));
    aSwapCard->setText(tr("Swap card to/from sideboard"));
}