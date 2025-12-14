#include "visual_deck_editor_widget.h"

#include "../../../client/settings/cache_settings.h"
#include "../../../main.h"
#include "../../deck_loader/deck_loader.h"
#include "../../layouts/overlap_layout.h"
#include "../cards/card_info_picture_with_text_overlay_widget.h"
#include "../cards/deck_card_zone_display_widget.h"
#include "../general/layout_containers/flow_widget.h"
#include "../tabs/visual_deck_editor/tab_deck_editor_visual.h"
#include "../tabs/visual_deck_editor/tab_deck_editor_visual_tab_widget.h"
#include "visual_deck_display_options_widget.h"

#include <QCheckBox>
#include <QCompleter>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QResizeEvent>
#include <libcockatrice/card/card_info_comparator.h>
#include <libcockatrice/card/database/card_database.h>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/models/database/card/card_completer_proxy_model.h>
#include <libcockatrice/models/database/card/card_search_model.h>
#include <libcockatrice/models/database/card_database_model.h>
#include <libcockatrice/models/deck_list/deck_list_model.h>
#include <qscrollarea.h>

VisualDeckEditorWidget::VisualDeckEditorWidget(QWidget *parent,
                                               DeckListModel *_deckListModel,
                                               QItemSelectionModel *_selectionModel)
    : QWidget(parent), deckListModel(_deckListModel), selectionModel(_selectionModel)
{
    // The Main Widget and Main Layout, which contain a single Widget: The Scroll Area
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mainLayout = new QVBoxLayout(this);
    setLayout(mainLayout);
    mainLayout->setContentsMargins(9, 0, 9, 5);
    mainLayout->setSpacing(0);

    searchBar = new QLineEdit(this);
    connect(searchBar, &QLineEdit::returnPressed, this, [=, this]() {
        if (!searchBar->hasFocus())
            return;

        ExactCard card = CardDatabaseManager::query()->getCard({searchBar->text()});
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
        ExactCard card = CardDatabaseManager::query()->getCard({searchBar->text()});
        if (card) {
            emit cardAdditionRequested(card);
        }
    });

    displayOptionsAndSearch = new QWidget(this);
    displayOptionsAndSearchLayout = new QHBoxLayout(displayOptionsAndSearch);
    displayOptionsAndSearchLayout->setAlignment(Qt::AlignLeft);
    displayOptionsAndSearch->setLayout(displayOptionsAndSearchLayout);

    displayOptionsWidget = new VisualDeckDisplayOptionsWidget(this);
    connect(displayOptionsWidget, &VisualDeckDisplayOptionsWidget::displayTypeChanged, this,
            &VisualDeckEditorWidget::displayTypeChanged);
    connect(displayOptionsWidget, &VisualDeckDisplayOptionsWidget::groupCriteriaChanged, this,
            &VisualDeckEditorWidget::activeGroupCriteriaChanged);
    connect(displayOptionsWidget, &VisualDeckDisplayOptionsWidget::sortCriteriaChanged, this,
            &VisualDeckEditorWidget::activeSortCriteriaChanged);

    displayOptionsAndSearchLayout->addWidget(displayOptionsWidget);
    displayOptionsAndSearchLayout->addWidget(searchBar);
    displayOptionsAndSearchLayout->addWidget(searchPushButton);

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

    cardSizeWidget = new CardSizeWidget(this, nullptr, SettingsCache::instance().getVisualDeckEditorCardSize());
    connect(cardSizeWidget, &CardSizeWidget::cardSizeSettingUpdated, &SettingsCache::instance(),
            &SettingsCache::setVisualDeckEditorCardSize);

    mainLayout->addWidget(displayOptionsAndSearch);
    mainLayout->addWidget(scrollArea);
    mainLayout->addWidget(cardSizeWidget);

    connect(deckListModel, &DeckListModel::modelReset, this, &VisualDeckEditorWidget::decklistModelReset);
    connect(deckListModel, &DeckListModel::dataChanged, this, &VisualDeckEditorWidget::decklistDataChanged);
    connect(deckListModel, &QAbstractItemModel::rowsInserted, this, &VisualDeckEditorWidget::onCardAddition);
    connect(deckListModel, &QAbstractItemModel::rowsRemoved, this, &VisualDeckEditorWidget::onCardRemoval);
    constructZoneWidgetsFromDeckListModel();

    if (selectionModel) {
        connect(selectionModel, &QItemSelectionModel::selectionChanged, this,
                &VisualDeckEditorWidget::onSelectionChanged);
    }

    retranslateUi();
}

void VisualDeckEditorWidget::retranslateUi()
{
    searchBar->setPlaceholderText(tr("Type a card name here for suggestions from the database..."));
    searchPushButton->setText(tr("Quick search and add card"));
    searchPushButton->setToolTip(tr("Search for closest match in the database (with auto-suggestions) and add "
                                    "preferred printing to the deck on pressing enter"));
}

void VisualDeckEditorWidget::setSelectionModel(QItemSelectionModel *model)
{
    if (selectionModel == model) {
        return;
    }

    if (selectionModel) {
        // TODO: Possibly disconnect old ones?
    }

    selectionModel = model;

    if (selectionModel) {
        connect(selectionModel, &QItemSelectionModel::selectionChanged, this,
                &VisualDeckEditorWidget::onSelectionChanged);
    }
}

void VisualDeckEditorWidget::onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    for (auto &range : selected) {
        for (int row = range.top(); row <= range.bottom(); ++row) {
            QModelIndex idx = range.model()->index(row, 0, range.parent());
            auto it = indexToWidgetMap.find(QPersistentModelIndex(idx));
            if (it != indexToWidgetMap.end()) {
                // it.value()->setHighlighted(true);
            }
        }
    }

    for (auto &range : deselected) {
        for (int row = range.top(); row <= range.bottom(); ++row) {
            QModelIndex idx = range.model()->index(row, 0, range.parent());
            auto it = indexToWidgetMap.find(QPersistentModelIndex(idx));
            if (it != indexToWidgetMap.end()) {
                // it.value()->setHighlighted(false);
            }
        }
    }
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

            constructZoneWidgetForIndex(index);
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

void VisualDeckEditorWidget::constructZoneWidgetForIndex(QPersistentModelIndex persistent)
{
    DeckCardZoneDisplayWidget *zoneDisplayWidget = new DeckCardZoneDisplayWidget(
        zoneContainer, deckListModel, selectionModel, persistent,
        deckListModel->data(persistent.sibling(persistent.row(), 1), Qt::EditRole).toString(),
        displayOptionsWidget->getActiveGroupCriteria(), displayOptionsWidget->getActiveSortCriteria(),
        displayOptionsWidget->getDisplayType(), 20, 10, cardSizeWidget);
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
    zoneDisplayWidget->refreshDisplayType(displayOptionsWidget->getDisplayType());
    zoneContainerLayout->addWidget(zoneDisplayWidget);

    indexToWidgetMap.insert(persistent, zoneDisplayWidget);
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

        constructZoneWidgetForIndex(persistent);
    }
}

void VisualDeckEditorWidget::updateZoneWidgets()
{
}

void VisualDeckEditorWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    zoneContainer->setMaximumWidth(scrollArea->viewport()->width());
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
    // If user has any card selected, ignore hover
    if (selectionModel->hasSelection()) {
        return;
    }

    // If nothing is selected -> this is our "active/preview" card
    emit activeCardChanged(hoveredCard);

    // TODO: highlight hovered card visually:
    // highlightHoveredCard(hoveredCard);
}

void VisualDeckEditorWidget::onCardClick(QMouseEvent *event,
                                         CardInfoPictureWithTextOverlayWidget *instance,
                                         QString zoneName)
{
    emit cardClicked(event, instance, zoneName);
}
