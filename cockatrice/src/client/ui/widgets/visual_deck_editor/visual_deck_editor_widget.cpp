#include "visual_deck_editor_widget.h"

#include "../../../../card/card_completer_proxy_model.h"
#include "../../../../card/card_database.h"
#include "../../../../card/card_database_manager.h"
#include "../../../../card/card_database_model.h"
#include "../../../../card/card_search_model.h"
#include "../../../../deck/deck_list_model.h"
#include "../../../../deck/deck_loader.h"
#include "../../../../main.h"
#include "../../../../utility/card_info_comparator.h"
#include "../../layouts/overlap_layout.h"
#include "../cards/card_info_picture_with_text_overlay_widget.h"
#include "../cards/deck_card_zone_display_widget.h"
#include "../general/layout_containers/flow_widget.h"
#include "../general/layout_containers/overlap_control_widget.h"

#include <QCheckBox>
#include <QCompleter>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QResizeEvent>
#include <qscrollarea.h>

VisualDeckEditorWidget::VisualDeckEditorWidget(QWidget *parent, DeckListModel *_deckListModel)
    : QWidget(parent), deckListModel(_deckListModel)
{
    // The Main Widget and Main Layout, which contain a single Widget: The Scroll Area
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mainLayout = new QVBoxLayout(this);
    setLayout(mainLayout);
    mainLayout->setContentsMargins(9, 0, 9, 5);
    mainLayout->setSpacing(0);

    searchContainer = new QWidget(this);
    searchLayout = new QHBoxLayout(searchContainer);
    searchContainer->setLayout(searchLayout);

    searchBar = new QLineEdit(this);
    connect(searchBar, &QLineEdit::returnPressed, this, [=, this]() {
        if (!searchBar->hasFocus())
            return;

        ExactCard card = CardDatabaseManager::getInstance()->getCard({searchBar->text()});
        if (card) {
            emit cardAdditionRequested(card);
        }
    });

    setFocusProxy(searchBar);
    setFocusPolicy(Qt::ClickFocus);

    cardDatabaseModel = new CardDatabaseModel(CardDatabaseManager::getInstance(), false, this);
    cardDatabaseDisplayModel = new CardDatabaseDisplayModel(this);
    cardDatabaseDisplayModel->setSourceModel(cardDatabaseModel);
    CardSearchModel *searchModel = new CardSearchModel(cardDatabaseDisplayModel, this);

    proxyModel = new CardCompleterProxyModel(this);
    proxyModel->setSourceModel(searchModel);
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxyModel->setFilterRole(Qt::DisplayRole);

    completer = new QCompleter(proxyModel, this);
    completer->setCompletionRole(Qt::DisplayRole);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setFilterMode(Qt::MatchContains);
    completer->setMaxVisibleItems(15);
    searchBar->setCompleter(completer);

    // Update suggestions dynamically
    connect(searchBar, &QLineEdit::textEdited, searchModel, &CardSearchModel::updateSearchResults);
    connect(searchBar, &QLineEdit::textEdited, this, [=, this](const QString &text) {
        // Ensure substring matching
        QString pattern = ".*" + QRegularExpression::escape(text) + ".*";
        proxyModel->setFilterRegularExpression(QRegularExpression(pattern, QRegularExpression::CaseInsensitiveOption));

        if (!text.isEmpty()) {
            completer->complete(); // Force the dropdown to appear
        }
    });

    connect(completer, static_cast<void (QCompleter::*)(const QString &)>(&QCompleter::activated), this,
            [=, this](const QString &completion) {
                // Prevent the text from changing automatically when navigating with arrow keys
                if (searchBar->text() != completion) {
                    searchBar->setText(completion);                           // Set the completion explicitly
                    searchBar->setCursorPosition(searchBar->text().length()); // Move cursor to the end
                }
            });

    // Ensure that the text stays consistent during selection
    connect(searchBar, &QLineEdit::textEdited, this, [=, this](const QString &text) {
        if (searchBar->hasFocus() && !searchBar->completer()->popup()->isVisible()) {
            // Allow text to change when typing, but not when navigating the completer
            QString pattern = ".*" + QRegularExpression::escape(text) + ".*";
            proxyModel->setFilterRegularExpression(
                QRegularExpression(pattern, QRegularExpression::CaseInsensitiveOption));
        }
    });

    // Search button functionality
    searchPushButton = new QPushButton(this);
    connect(searchPushButton, &QPushButton::clicked, this, [=, this]() {
        ExactCard card = CardDatabaseManager::getInstance()->getCard({searchBar->text()});
        if (card) {
            emit cardAdditionRequested(card);
        }
    });

    searchLayout->addWidget(searchBar);
    searchLayout->addWidget(searchPushButton);

    mainLayout->addWidget(searchContainer);

    groupAndSortContainer = new QWidget(this);
    groupAndSortLayout = new QHBoxLayout(groupAndSortContainer);
    groupAndSortLayout->setAlignment(Qt::AlignLeft);
    groupAndSortContainer->setLayout(groupAndSortLayout);

    groupByComboBox = new QComboBox();
    QStringList groupProperties = {"maintype", "colors", "cmc", "name"};
    groupByComboBox->addItems(groupProperties);
    groupByComboBox->setMinimumWidth(300);
    connect(groupByComboBox, QOverload<const QString &>::of(&QComboBox::currentTextChanged), this,
            &VisualDeckEditorWidget::actChangeActiveGroupCriteria);
    actChangeActiveGroupCriteria();

    sortCriteriaButton = new SettingsButtonWidget(this);

    sortLabel = new QLabel(sortCriteriaButton);
    sortLabel->setWordWrap(true);

    QStringList sortProperties = {"colors", "cmc", "name", "maintype"};
    sortByListWidget = new QListWidget();
    sortByListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    sortByListWidget->setDragDropMode(QAbstractItemView::InternalMove);
    sortByListWidget->setDefaultDropAction(Qt::MoveAction);

    for (const QString &property : sortProperties) {
        QListWidgetItem *item = new QListWidgetItem(property, sortByListWidget);
        item->setFlags(item->flags() | Qt::ItemIsDragEnabled | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    }

    connect(sortByListWidget->model(), &QAbstractItemModel::rowsMoved, this,
            &VisualDeckEditorWidget::actChangeActiveSortCriteria);
    actChangeActiveSortCriteria();

    sortByListWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    sortCriteriaButton->addSettingsWidget(sortLabel);
    sortCriteriaButton->addSettingsWidget(sortByListWidget);

    displayTypeButton = new QPushButton(this);
    connect(displayTypeButton, &QPushButton::clicked, this, &VisualDeckEditorWidget::updateDisplayType);

    groupAndSortLayout->addWidget(groupByComboBox);
    groupAndSortLayout->addWidget(sortCriteriaButton);
    groupAndSortLayout->addWidget(displayTypeButton);

    scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setMinimumSize(0, 0);

    // Set scrollbar policies
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    zoneContainer = new QWidget(scrollArea);
    zoneContainerLayout = new QVBoxLayout(zoneContainer);
    zoneContainer->setLayout(zoneContainerLayout);
    scrollArea->addScrollBarWidget(zoneContainer, Qt::AlignHCenter);
    scrollArea->setWidget(zoneContainer);

    cardSizeWidget = new CardSizeWidget(this);

    mainLayout->addWidget(groupAndSortContainer);
    mainLayout->addWidget(scrollArea);
    mainLayout->addWidget(cardSizeWidget);

    connect(deckListModel, &DeckListModel::modelReset, this, &VisualDeckEditorWidget::decklistModelReset);
    connect(deckListModel, &DeckListModel::dataChanged, this, &VisualDeckEditorWidget::decklistDataChanged);
    connect(deckListModel, &QAbstractItemModel::rowsInserted, this, &VisualDeckEditorWidget::onCardAddition);
    connect(deckListModel, &QAbstractItemModel::rowsRemoved, this, &VisualDeckEditorWidget::onCardRemoval);
    constructZoneWidgetsFromDeckListModel();

    retranslateUi();
}

void VisualDeckEditorWidget::retranslateUi()
{
    sortLabel->setText(tr("Click and drag to change the sort order within the groups"));
    searchPushButton->setText(tr("Quick search and add card"));
    searchPushButton->setToolTip(tr("Search for closest match in the database (with auto-suggestions) and add "
                                    "preferred printing to the deck on pressing enter"));
    sortCriteriaButton->setToolTip(tr("Configure how cards are sorted within their groups"));
    displayTypeButton->setText(tr("Overlap Layout"));
    displayTypeButton->setToolTip(
        tr("Change how cards are displayed within zones (i.e. overlapped or fully visible.)"));
}

void VisualDeckEditorWidget::clearAllDisplayWidgets()
{
    for (auto idx : indexToWidgetMap.keys()) {
        auto displayWidget = indexToWidgetMap.value(idx);
        zoneContainerLayout->removeWidget(displayWidget);
        indexToWidgetMap.remove(idx);
        delete displayWidget;
    }
}

void VisualDeckEditorWidget::cleanupInvalidZones(DeckCardZoneDisplayWidget *displayWidget)
{
    zoneContainerLayout->removeWidget(displayWidget);
    for (auto idx : indexToWidgetMap.keys()) {
        if (!idx.isValid()) {
            indexToWidgetMap.remove(idx);
        }
    }
    delete displayWidget;
}

void VisualDeckEditorWidget::onCardAddition(const QModelIndex &parent, int first, int last)
{
    if (parent == deckListModel->getRoot()) {
        for (int i = first; i <= last; i++) {
            QPersistentModelIndex index = QPersistentModelIndex(deckListModel->index(i, 0, deckListModel->getRoot()));

            if (indexToWidgetMap.contains(index)) {
                continue;
            }

            DeckCardZoneDisplayWidget *zoneDisplayWidget = new DeckCardZoneDisplayWidget(
                zoneContainer, deckListModel, index,
                deckListModel->data(index.sibling(index.row(), 1), Qt::EditRole).toString(), activeGroupCriteria,
                activeSortCriteria, currentDisplayType, 20, 10, cardSizeWidget);
            connect(zoneDisplayWidget, &DeckCardZoneDisplayWidget::cardHovered, this, &VisualDeckEditorWidget::onHover);
            connect(zoneDisplayWidget, &DeckCardZoneDisplayWidget::cardClicked, this,
                    &VisualDeckEditorWidget::onCardClick);
            connect(zoneDisplayWidget, &DeckCardZoneDisplayWidget::requestCleanup, this,
                    &VisualDeckEditorWidget::cleanupInvalidZones);
            connect(this, &VisualDeckEditorWidget::activeSortCriteriaChanged, zoneDisplayWidget,
                    &DeckCardZoneDisplayWidget::onActiveSortCriteriaChanged);
            connect(this, &VisualDeckEditorWidget::activeGroupCriteriaChanged, zoneDisplayWidget,
                    &DeckCardZoneDisplayWidget::onActiveGroupCriteriaChanged);
            connect(this, &VisualDeckEditorWidget::displayTypeChanged, zoneDisplayWidget,
                    &DeckCardZoneDisplayWidget::refreshDisplayType);
            zoneDisplayWidget->refreshDisplayType(currentDisplayType);
            zoneContainerLayout->addWidget(zoneDisplayWidget);

            indexToWidgetMap.insert(index, zoneDisplayWidget);
        }
    }
}

void VisualDeckEditorWidget::onCardRemoval(const QModelIndex &parent, int first, int last)
{
    Q_UNUSED(parent);
    Q_UNUSED(first);
    Q_UNUSED(last);
    for (const QPersistentModelIndex &idx : indexToWidgetMap.keys()) {
        if (!idx.isValid()) {
            zoneContainerLayout->removeWidget(indexToWidgetMap.value(idx));
            indexToWidgetMap.value(idx)->deleteLater();
            indexToWidgetMap.remove(idx);
        }
    }
}

void VisualDeckEditorWidget::constructZoneWidgetsFromDeckListModel()
{
    QSortFilterProxyModel proxy;
    proxy.setSourceModel(deckListModel);
    proxy.setSortRole(Qt::EditRole);
    proxy.sort(1, Qt::AscendingOrder);

    for (int i = 0; i < proxy.rowCount(); ++i) {
        QModelIndex proxyIndex = proxy.index(i, 0);
        QModelIndex sourceIndex = proxy.mapToSource(proxyIndex);

        // Make a persistent index from the *source* model
        QPersistentModelIndex persistent(sourceIndex);

        if (indexToWidgetMap.contains(persistent)) {
            continue;
        }

        DeckCardZoneDisplayWidget *zoneDisplayWidget = new DeckCardZoneDisplayWidget(
            zoneContainer, deckListModel, persistent,
            deckListModel->data(persistent.sibling(persistent.row(), 1), Qt::EditRole).toString(), activeGroupCriteria,
            activeSortCriteria, currentDisplayType, 20, 10, cardSizeWidget);
        connect(zoneDisplayWidget, &DeckCardZoneDisplayWidget::cardHovered, this, &VisualDeckEditorWidget::onHover);
        connect(zoneDisplayWidget, &DeckCardZoneDisplayWidget::cardClicked, this, &VisualDeckEditorWidget::onCardClick);
        connect(zoneDisplayWidget, &DeckCardZoneDisplayWidget::requestCleanup, this,
                &VisualDeckEditorWidget::cleanupInvalidZones);
        connect(this, &VisualDeckEditorWidget::activeSortCriteriaChanged, zoneDisplayWidget,
                &DeckCardZoneDisplayWidget::onActiveSortCriteriaChanged);
        connect(this, &VisualDeckEditorWidget::activeGroupCriteriaChanged, zoneDisplayWidget,
                &DeckCardZoneDisplayWidget::onActiveGroupCriteriaChanged);
        connect(this, &VisualDeckEditorWidget::displayTypeChanged, zoneDisplayWidget,
                &DeckCardZoneDisplayWidget::refreshDisplayType);
        zoneContainerLayout->addWidget(zoneDisplayWidget);

        indexToWidgetMap.insert(persistent, zoneDisplayWidget);
    }
}

void VisualDeckEditorWidget::updateZoneWidgets()
{
}

void VisualDeckEditorWidget::updateDisplayType()
{
    // Toggle the display type
    currentDisplayType = (currentDisplayType == DisplayType::Overlap) ? DisplayType::Flat : DisplayType::Overlap;

    // Update UI and emit signal
    switch (currentDisplayType) {
        case DisplayType::Flat:
            displayTypeButton->setText(tr("Flat Layout"));
            break;
        case DisplayType::Overlap:
            displayTypeButton->setText(tr("Overlap Layout"));
            break;
    }
    emit displayTypeChanged(currentDisplayType);
}

void VisualDeckEditorWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    zoneContainer->setMaximumWidth(scrollArea->viewport()->width());
}

void VisualDeckEditorWidget::actChangeActiveGroupCriteria()
{
    activeGroupCriteria = groupByComboBox->currentText();
    emit activeGroupCriteriaChanged(activeGroupCriteria);
}

void VisualDeckEditorWidget::actChangeActiveSortCriteria()
{
    QStringList selectedCriteria;
    for (int i = 0; i < sortByListWidget->count(); ++i) {
        QListWidgetItem *item = sortByListWidget->item(i);
        selectedCriteria.append(item->text()); // Collect user-defined sort order
    }

    activeSortCriteria = selectedCriteria;

    emit activeSortCriteriaChanged(selectedCriteria);
}

void VisualDeckEditorWidget::decklistModelReset()
{
    clearAllDisplayWidgets();
    constructZoneWidgetsFromDeckListModel();
}

void VisualDeckEditorWidget::decklistDataChanged(QModelIndex topLeft, QModelIndex bottomRight)
{
    // Might use these at some point.
    Q_UNUSED(topLeft);
    Q_UNUSED(bottomRight);
    // Necessary to delay this in this manner else the updateDisplay will nuke widgets while their onClick event
    // hasn't returned yet. Interval of 0 means QT will schedule this after the current event loop has finished.
    updateZoneWidgets();
}

void VisualDeckEditorWidget::onHover(const ExactCard &hoveredCard)
{
    emit activeCardChanged(hoveredCard);
}

void VisualDeckEditorWidget::onCardClick(QMouseEvent *event,
                                         CardInfoPictureWithTextOverlayWidget *instance,
                                         QString zoneName)
{
    emit cardClicked(event, instance, zoneName);
}
