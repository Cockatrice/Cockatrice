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
#include <libcockatrice/utility/qt_utils.h>
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

    initializeDisplayOptionsAndSearchWidget();

    initializeScrollAreaAndZoneContainer();

    cardSizeWidget = new CardSizeWidget(this, nullptr, SettingsCache::instance().getVisualDeckEditorCardSize());
    connect(cardSizeWidget, &CardSizeWidget::cardSizeSettingUpdated, &SettingsCache::instance(),
            &SettingsCache::setVisualDeckEditorCardSize);

    mainLayout->addWidget(displayOptionsAndSearch);
    mainLayout->addWidget(scrollArea);
    mainLayout->addWidget(cardSizeWidget);

    connectDeckListModel();

    constructZoneWidgetsFromDeckListModel();

    if (selectionModel) {
        connect(selectionModel, &QItemSelectionModel::selectionChanged, this,
                &VisualDeckEditorWidget::onSelectionChanged);
    }

    updatePlaceholderVisibility();
    retranslateUi();
}

// =====================================================================================================================
//                                                  Constructor helpers
// =====================================================================================================================

void VisualDeckEditorWidget::initializeSearchBarAndCompleter()
{
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
}

void VisualDeckEditorWidget::initializeDisplayOptionsWidget()
{
    displayOptionsWidget = new VisualDeckDisplayOptionsWidget(this);
    connect(displayOptionsWidget, &VisualDeckDisplayOptionsWidget::displayTypeChanged, this,
            &VisualDeckEditorWidget::displayTypeChanged);
    connect(displayOptionsWidget, &VisualDeckDisplayOptionsWidget::groupCriteriaChanged, this,
            &VisualDeckEditorWidget::activeGroupCriteriaChanged);
    connect(displayOptionsWidget, &VisualDeckDisplayOptionsWidget::sortCriteriaChanged, this,
            &VisualDeckEditorWidget::activeSortCriteriaChanged);
}

void VisualDeckEditorWidget::initializeDisplayOptionsAndSearchWidget()
{
    initializeSearchBarAndCompleter();

    initializeDisplayOptionsWidget();

    displayOptionsAndSearch = new QWidget(this);
    displayOptionsAndSearchLayout = new QHBoxLayout(displayOptionsAndSearch);
    displayOptionsAndSearchLayout->setAlignment(Qt::AlignLeft);
    displayOptionsAndSearch->setLayout(displayOptionsAndSearchLayout);

    displayOptionsAndSearchLayout->addWidget(displayOptionsWidget);
    displayOptionsAndSearchLayout->addWidget(searchBar);
    displayOptionsAndSearchLayout->addWidget(searchPushButton);
}

void VisualDeckEditorWidget::initializeScrollAreaAndZoneContainer()
{
    scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setMinimumSize(0, 0);

    // Set scrollbar policies
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    zoneContainer = new QWidget(scrollArea);
    zoneContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    zoneContainer->setObjectName("zoneContainer");
    zoneContainerLayout = new QVBoxLayout(zoneContainer);
    zoneContainer->setLayout(zoneContainerLayout);

    // Create placeholder widget
    placeholderWidget = new VisualDeckEditorPlaceholderWidget(zoneContainer);
    zoneContainerLayout->addWidget(placeholderWidget);

    scrollArea->addScrollBarWidget(zoneContainer, Qt::AlignHCenter);
    scrollArea->setWidget(zoneContainer);
}

void VisualDeckEditorWidget::connectDeckListModel()
{
    connect(deckListModel, &DeckListModel::modelReset, this, &VisualDeckEditorWidget::decklistModelReset);
    connect(deckListModel, &DeckListModel::dataChanged, this, &VisualDeckEditorWidget::decklistDataChanged);
    connect(deckListModel, &QAbstractItemModel::rowsInserted, this, &VisualDeckEditorWidget::onCardAddition);
    connect(deckListModel, &QAbstractItemModel::rowsRemoved, this, &VisualDeckEditorWidget::onCardRemoval);
}

void VisualDeckEditorWidget::retranslateUi()
{
    searchBar->setPlaceholderText(tr("Type a card name here for suggestions from the database..."));
    searchPushButton->setText(tr("Quick search and add card"));
    searchPushButton->setToolTip(tr("Search for closest match in the database (with auto-suggestions) and add "
                                    "preferred printing to the deck on pressing enter"));

    if (placeholderWidget) {
        placeholderWidget->retranslateUi();
    }
}

void VisualDeckEditorWidget::updatePlaceholderVisibility()
{
    if (placeholderWidget) {
        placeholderWidget->setVisible(indexToWidgetMap.isEmpty());
    }
}

// =====================================================================================================================
//                                              Display Widget Management
// =====================================================================================================================

void VisualDeckEditorWidget::constructZoneWidgetForIndex(QPersistentModelIndex persistent)
{
    QString zoneName =
        persistent.sibling(persistent.row(), DeckListModelColumns::CARD_NAME).data(Qt::EditRole).toString();
    DeckCardZoneDisplayWidget *zoneDisplayWidget = new DeckCardZoneDisplayWidget(
        zoneContainer, deckListModel, selectionModel, persistent, zoneName,
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
    proxy.sort(DeckListModelColumns::CARD_NAME, Qt::AscendingOrder);

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
    updatePlaceholderVisibility();
}

void VisualDeckEditorWidget::updateZoneWidgets()
{
}

void VisualDeckEditorWidget::clearAllDisplayWidgets()
{
    for (auto idx : indexToWidgetMap.keys()) {
        auto displayWidget = indexToWidgetMap.value(idx);
        zoneContainerLayout->removeWidget(displayWidget);
        indexToWidgetMap.remove(idx);
        delete displayWidget;
    }
    updatePlaceholderVisibility();
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
    updatePlaceholderVisibility();
}

// =====================================================================================================================
//                                                 DeckModel Signals Management
// =====================================================================================================================

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
    updatePlaceholderVisibility();
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
    updatePlaceholderVisibility();
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

// =====================================================================================================================
//                                                 User Interaction
// =====================================================================================================================

void VisualDeckEditorWidget::onCardClick(QMouseEvent *event,
                                         CardInfoPictureWithTextOverlayWidget *instance,
                                         QString zoneName)
{
    emit cardClicked(event, instance, zoneName);
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

TutorialSequence VisualDeckEditorWidget::addTutorialSteps()
{
    TutorialSequence sequence;
    sequence.name = "Adding Cards to Your Deck";

    TutorialStep introStep;
    introStep.targetWidget = displayOptionsAndSearch;
    introStep.text = "There are two ways of adding cards to your deck:\n\n"
                     "The first is by using the quick search bar in the deck view tab.\n"
                     "This is helpful if you already know which card you would like to add "
                     "and will provide name suggestions as you type.\n\n"
                     "We'll look at the second way, through the database tab, later.";
    sequence.addStep(introStep);

    TutorialStep searchStep;
    searchStep.targetWidget = searchBar;
    searchStep.text = "Let's try it out now!\nType the name of a card into the search bar.";
    searchStep.allowClickThrough = true;
    searchStep.requiresInteraction = true;
    searchStep.autoAdvanceOnValid = true;
    searchStep.validationTiming = ValidationTiming::OnChange; // Make sure this is set!
    searchStep.validator = [this]() {
        return CardDatabaseManager::query()->getCard({searchBar->text()}) != ExactCard();
    };
    searchStep.validationHint = "Please enter a valid card name.";
    searchStep.customInteractionHint = "✏️ Type a valid card name to continue";

    sequence.addStep(searchStep);

    TutorialStep addStep;
    addStep.targetWidget = searchPushButton;
    addStep.text = "Click this button to add the card to your deck.";
    addStep.allowClickThrough = true;
    addStep.requiresInteraction = true;
    addStep.autoAdvanceOnValid = true;
    addStep.validationTiming = ValidationTiming::OnSignal;
    addStep.signalSource = deckListModel;
    addStep.signalName = SIGNAL(cardAddedAt(const QModelIndex &));
    addStep.validator = [this]() { return deckListModel->getCardNodes().size() >= 1; };

    sequence.addStep(addStep);

    TutorialStep organizationStep;
    organizationStep.targetWidget = this;
    organizationStep.text = "Let's look at how cards are organized and displayed now.\n\nWe'll add some random cards "
                            "from the database to your deck, so you can see it in action properly.";
    organizationStep.onExit = [this]() {
        while (deckListModel->getDeckList()->getCardList().size() < 60) {
            deckListModel->addCard(CardDatabaseManager::query()->getRandomCard(), DECK_ZONE_MAIN);
        }
    };

    sequence.addStep(organizationStep);

    TutorialStep hoverStep;
    hoverStep.targetWidget = this;
    hoverStep.text = "Great! Take some time to explore these new cards in the current display mode.\n\nYou can select "
                     "a card by clicking on it with the left mouse button.\nYou can select multiple cards by holding "
                     "down CTRL or Shift.\nYou can clear the current selection by clicking on an area without a "
                     "card.\nDouble-clicking a card will move it between main and sideboard.\nRight-clicking a card "
                     "will remove it from the deck.\n\nYou can hover over a card to see a zoomed version of it.";
    hoverStep.allowClickThrough = true;

    sequence.addStep(hoverStep);

    sequence = displayOptionsWidget->generateTutorialSequence(sequence);

    TutorialStep conclusionStep;
    conclusionStep.targetWidget = this;
    conclusionStep.text =
        "Great!\n\nNow that you've learned about all the different ways of displaying the cards in "
        "your deck, it's time to move on to searching for new cards for your deck in style and ease.\n\nYou can stay "
        "on this screen to play around with the display options and advance when you are ready.";
    conclusionStep.allowClickThrough = true;

    sequence.addStep(conclusionStep);

    return sequence;
}