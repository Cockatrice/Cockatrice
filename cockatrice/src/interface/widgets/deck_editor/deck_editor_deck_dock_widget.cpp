#include "deck_editor_deck_dock_widget.h"

#include "../../../client/settings/cache_settings.h"
#include "../../deck_loader/deck_loader.h"
#include "deck_list_style_proxy.h"

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

    deckLoader = new DeckLoader(this);

    proxy = new DeckListStyleProxy(this);
    proxy->setSourceModel(deckModel);

    historyManagerWidget = new DeckListHistoryManagerWidget(deckModel, proxy, deckEditor->getHistoryManager(), this);
    connect(historyManagerWidget, &DeckListHistoryManagerWidget::requestDisplayWidgetSync, this,
            &DeckEditorDeckDockWidget::syncDisplayWidgetsToModel);

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
    connect(nameDebounceTimer, &QTimer::timeout, this, [this]() { updateName(nameEdit->text()); });

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
    connect(commentsDebounceTimer, &QTimer::timeout, this, [this]() { updateComments(); });

    connect(commentsEdit, &QTextEdit::textChanged, this, [this]() {
        commentsDebounceTimer->start(); // restart debounce timer
    });

    bannerCardLabel = new QLabel();
    bannerCardLabel->setObjectName("bannerCardLabel");
    bannerCardLabel->setText(tr("Banner Card"));
    bannerCardLabel->setHidden(!SettingsCache::instance().getDeckEditorBannerCardComboBoxVisible());
    bannerCardComboBox = new QComboBox(this);
    connect(deckModel, &DeckListModel::dataChanged, this, [this]() {
        // Delay the update to avoid race conditions
        QTimer::singleShot(100, this, &DeckEditorDeckDockWidget::updateBannerCardComboBox);
    });
    connect(deckModel, &DeckListModel::cardAddedAt, this, &DeckEditorDeckDockWidget::recursiveExpand);
    connect(deckModel, &DeckListModel::deckReplaced, this, &DeckEditorDeckDockWidget::expandAll);

    connect(bannerCardComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &DeckEditorDeckDockWidget::setBannerCard);
    bannerCardComboBox->setHidden(!SettingsCache::instance().getDeckEditorBannerCardComboBoxVisible());

    deckTagsDisplayWidget = new DeckPreviewDeckTagsDisplayWidget(this, deckModel->getDeckList()->getTags());
    deckTagsDisplayWidget->setHidden(!SettingsCache::instance().getDeckEditorTagsWidgetVisible());
    connect(deckTagsDisplayWidget, &DeckPreviewDeckTagsDisplayWidget::tagsChanged, this,
            &DeckEditorDeckDockWidget::setTags);

    activeGroupCriteriaLabel = new QLabel(this);

    activeGroupCriteriaComboBox = new QComboBox(this);
    activeGroupCriteriaComboBox->addItem(tr("Main Type"), DeckListModelGroupCriteria::MAIN_TYPE);
    activeGroupCriteriaComboBox->addItem(tr("Mana Cost"), DeckListModelGroupCriteria::MANA_COST);
    activeGroupCriteriaComboBox->addItem(tr("Colors"), DeckListModelGroupCriteria::COLOR);
    connect(activeGroupCriteriaComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [this]() {
        deckModel->setActiveGroupCriteria(static_cast<DeckListModelGroupCriteria::Type>(
            activeGroupCriteriaComboBox->currentData(Qt::UserRole).toInt()));
        deckModel->sort(deckView->header()->sortIndicatorSection(), deckView->header()->sortIndicatorOrder());
    });

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
    QMap<QString, int> allFormats = CardDatabaseManager::query()->getAllFormatsWithCount();

    formatComboBox->clear(); // Remove "Loading Database..."
    formatComboBox->setEnabled(true);

    // Populate with formats
    formatComboBox->addItem("", "");
    for (auto it = allFormats.constBegin(); it != allFormats.constEnd(); ++it) {
        QString displayText = QString("%1").arg(it.key());
        formatComboBox->addItem(displayText, it.key()); // store the raw key in itemData
    }

    if (!deckModel->getDeckList()->getGameFormat().isEmpty()) {
        deckModel->setActiveFormat(deckModel->getDeckList()->getGameFormat());
        formatComboBox->setCurrentIndex(formatComboBox->findData(deckModel->getDeckList()->getGameFormat()));
    } else {
        // Ensure no selection is visible initially
        formatComboBox->setCurrentIndex(-1);
    }

    connect(formatComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        if (index >= 0) {
            QString formatKey = formatComboBox->itemData(index).toString();
            deckModel->setActiveFormat(formatKey);
        } else {
            deckModel->setActiveFormat(QString()); // clear format if deselected
        }
        emit deckModified();
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
        emit cardChanged(card);
    }
}

void DeckEditorDeckDockWidget::updateName(const QString &name)
{
    emit requestDeckHistorySave(
        QString(tr("Rename deck to \"%1\" from \"%2\"")).arg(name).arg(deckLoader->getDeck().deckList.getName()));
    deckModel->getDeckList()->setName(name);
    deckEditor->setModified(name.isEmpty());
    emit nameChanged();
    emit deckModified();
}

void DeckEditorDeckDockWidget::updateComments()
{
    emit requestDeckHistorySave(tr("Updated comments (was %1 chars, now %2 chars)")
                                    .arg(deckLoader->getDeck().deckList.getComments().size())
                                    .arg(commentsEdit->toPlainText().size()));

    deckModel->getDeckList()->setComments(commentsEdit->toPlainText());
    deckEditor->setModified(commentsEdit->toPlainText().isEmpty());
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
    // Store current banner card identity
    CardRef wanted = deckModel->getDeckList()->getBannerCard();

    // Block signals temporarily
    bool wasBlocked = bannerCardComboBox->blockSignals(true);

    // Clear the existing items in the combo box
    bannerCardComboBox->clear();

    // Collect unique (name, providerId) pairs
    QSet<QPair<QString, QString>> bannerCardSet;
    QList<CardRef> cardsInDeck = deckModel->getCardRefs();

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
        syncDeckListBannerCardWithComboBox();
    } else {
        // Add a placeholder "-" and set it as the current selection
        bannerCardComboBox->insertItem(0, "-");
        bannerCardComboBox->setCurrentIndex(0);
    }

    // Restore signal state
    bannerCardComboBox->blockSignals(wasBlocked);
}

void DeckEditorDeckDockWidget::setBannerCard(int /* changedIndex */)
{
    emit requestDeckHistorySave(tr("Banner card changed"));
    syncDeckListBannerCardWithComboBox();
    deckEditor->setModified(true);
    emit deckModified();
}

void DeckEditorDeckDockWidget::setTags(const QStringList &tags)
{
    deckModel->getDeckList()->setTags(tags);
    deckEditor->setModified(true);
    emit deckModified();
}

void DeckEditorDeckDockWidget::syncDeckListBannerCardWithComboBox()
{
    auto [name, id] = bannerCardComboBox->currentData().value<QPair<QString, QString>>();
    deckModel->getDeckList()->setBannerCard({name, id});
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
    if (deckModel->getDeckList()->getBannerCard().name == "") {
        if (bannerCardComboBox->findText("-") != -1) {
            bannerCardComboBox->setCurrentIndex(bannerCardComboBox->findText("-"));
        } else {
            bannerCardComboBox->insertItem(0, "-");
            bannerCardComboBox->setCurrentIndex(0);
        }
    } else {
        bannerCardComboBox->setCurrentText(deckModel->getDeckList()->getBannerCard().name);
    }
}

/**
 * Sets the currently active deck for this tab
 * @param _deck The deck.
 */
void DeckEditorDeckDockWidget::setDeck(const LoadedDeck &_deck)
{
    deckLoader->setDeck(_deck);
    deckModel->setDeckList(&deckLoader->getDeck().deckList);
    connect(deckLoader, &DeckLoader::deckLoaded, deckModel, &DeckListModel::rebuildTree);

    emit requestDeckHistoryClear();
    historyManagerWidget->setDeckListModel(deckModel);

    syncDisplayWidgetsToModel();

    emit deckChanged();
}

void DeckEditorDeckDockWidget::syncDisplayWidgetsToModel()
{
    nameEdit->blockSignals(true);
    nameEdit->setText(deckModel->getDeckList()->getName());
    nameEdit->blockSignals(false);

    commentsEdit->blockSignals(true);
    commentsEdit->setText(deckModel->getDeckList()->getComments());
    commentsEdit->blockSignals(false);

    bannerCardComboBox->blockSignals(true);
    syncBannerCardComboBoxSelectionWithDeck();
    updateBannerCardComboBox();
    bannerCardComboBox->blockSignals(false);
    updateHash();
    sortDeckModelToDeckView();

    deckTagsDisplayWidget->setTags(deckModel->getDeckList()->getTags());
}

void DeckEditorDeckDockWidget::sortDeckModelToDeckView()
{
    deckModel->sort(deckView->header()->sortIndicatorSection(), deckView->header()->sortIndicatorOrder());
    deckModel->setActiveFormat(deckModel->getDeckList()->getGameFormat());
    formatComboBox->setCurrentIndex(formatComboBox->findData(deckModel->getDeckList()->getGameFormat()));

    emit deckChanged();
}

DeckLoader *DeckEditorDeckDockWidget::getDeckLoader()
{
    return deckLoader;
}

const DeckList &DeckEditorDeckDockWidget::getDeckList() const
{
    return *deckModel->getDeckList();
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
    deckTagsDisplayWidget->setTags(deckModel->getDeckList()->getTags());
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
 * Gets the index of all the currently selected card nodes in the decklist table.
 * The list is in reverse order of the visual selection, so that rows can be deleted while iterating over them.
 *
 * @return A model index list containing all selected card nodes
 */
QModelIndexList DeckEditorDeckDockWidget::getSelectedCardNodes() const
{
    auto selectedRows = deckView->selectionModel()->selectedRows();

    const auto notLeafNode = [this](const QModelIndex &index) {
        return deckModel->hasChildren(proxy->mapToSource(index));
    };
    selectedRows.erase(std::remove_if(selectedRows.begin(), selectedRows.end(), notLeafNode), selectedRows.end());

    std::reverse(selectedRows.begin(), selectedRows.end());
    return selectedRows;
}

void DeckEditorDeckDockWidget::actAddCard(const ExactCard &card, const QString &_zoneName)
{
    if (!card) {
        return;
    }

    QString zoneName = card.getInfo().getIsToken() ? DECK_ZONE_TOKENS : _zoneName;

    emit requestDeckHistorySave(tr("Added (%1): %2 (%3) %4")
                                    .arg(zoneName, card.getName(), card.getPrinting().getSet()->getCorrectedShortName(),
                                         card.getPrinting().getProperty("num")));

    QModelIndex newCardIndex = deckModel->addCard(card, zoneName);

    if (!newCardIndex.isValid()) {
        return;
    }

    deckView->clearSelection();
    deckView->setCurrentIndex(newCardIndex);

    emit deckModified();
}

void DeckEditorDeckDockWidget::actIncrementSelection()
{
    auto selectedRows = getSelectedCardNodes();

    for (const auto &index : selectedRows) {
        offsetCountAtIndex(index, true);
    }
}

void DeckEditorDeckDockWidget::actSwapCard(const ExactCard &card, const QString &zoneName)
{
    QString providerId = card.getPrinting().getUuid();
    QString collectorNumber = card.getPrinting().getProperty("num");

    QModelIndex foundCard = deckModel->findCard(card.getName(), zoneName, providerId, collectorNumber);
    if (!foundCard.isValid()) {
        foundCard = deckModel->findCard(card.getName(), zoneName);
    }

    swapCard(foundCard);
}

void DeckEditorDeckDockWidget::actSwapSelection()
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
    const QString cardName = currentIndex.siblingAtColumn(DeckListModelColumns::CARD_NAME).data().toString();
    const QString cardProviderID =
        currentIndex.siblingAtColumn(DeckListModelColumns::CARD_PROVIDER_ID).data().toString();
    const QModelIndex gparent = currentIndex.parent().parent();

    if (!gparent.isValid())
        return false;

    const QString zoneName = gparent.siblingAtColumn(DeckListModelColumns::CARD_NAME).data(Qt::EditRole).toString();
    offsetCountAtIndex(currentIndex, false);
    const QString otherZoneName = zoneName == DECK_ZONE_MAIN ? DECK_ZONE_SIDE : DECK_ZONE_MAIN;

    if (ExactCard card = CardDatabaseManager::query()->getCard({cardName, cardProviderID})) {
        deckModel->addCard(card, otherZoneName);
    } else {
        // Third argument (true) says create the card no matter what, even if not in DB
        deckModel->addPreferredPrintingCard(cardName, otherZoneName, true);
    }

    return true;
}

void DeckEditorDeckDockWidget::actDecrementCard(const ExactCard &card, QString zoneName)
{
    if (!card)
        return;
    if (card.getInfo().getIsToken())
        zoneName = DECK_ZONE_TOKENS;

    QString providerId = card.getPrinting().getUuid();
    QString collectorNumber = card.getPrinting().getProperty("num");

    QModelIndex idx = deckModel->findCard(card.getName(), zoneName, providerId, collectorNumber);
    if (!idx.isValid()) {
        return;
    }

    deckView->clearSelection();
    deckView->setCurrentIndex(proxy->mapToSource(idx));
    offsetCountAtIndex(idx, false);
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
        offsetCountAtIndex(index, false);
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
        QModelIndex sourceIndex = proxy->mapToSource(index);
        QString cardName = sourceIndex.siblingAtColumn(DeckListModelColumns::CARD_NAME).data().toString();

        emit requestDeckHistorySave(QString(tr("Removed \"%1\" (all copies)")).arg(cardName));

        deckModel->removeRow(sourceIndex.row(), sourceIndex.parent());
        isModified = true;
    }

    deckView->setSelectionMode(QAbstractItemView::ExtendedSelection);

    if (isModified) {
        emit deckModified();
    }
}

/**
 * @brief Increments or decrements the amount of the card node at the index by 1.
 * @param idx The proxy index
 * @param isIncrement If true, increments the count. If false, decrements the count
 */
void DeckEditorDeckDockWidget::offsetCountAtIndex(const QModelIndex &idx, bool isIncrement)
{
    if (!idx.isValid() || deckModel->hasChildren(idx)) {
        return;
    }

    QModelIndex sourceIndex = proxy->mapToSource(idx);

    QString cardName = sourceIndex.siblingAtColumn(DeckListModelColumns::CARD_NAME).data(Qt::EditRole).toString();
    QString providerId =
        sourceIndex.siblingAtColumn(DeckListModelColumns::CARD_PROVIDER_ID).data(Qt::DisplayRole).toString();

    const auto reason = QString(tr("%1 %2 × \"%3\" (%4)"))
                            .arg(isIncrement ? tr("Added") : tr("Removed"))
                            .arg(1)
                            .arg(cardName)
                            .arg(providerId);

    emit requestDeckHistorySave(reason);

    if (isIncrement) {
        deckModel->incrementAmountAtIndex(sourceIndex);
    } else {
        deckModel->decrementAmountAtIndex(sourceIndex);
    }

    emit deckModified();
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