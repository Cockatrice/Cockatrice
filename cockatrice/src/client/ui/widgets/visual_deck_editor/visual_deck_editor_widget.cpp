#include "visual_deck_editor_widget.h"

#include "../../../../deck/deck_list_model.h"
#include "../../../../deck/deck_loader.h"
#include "../../../../game/cards/card_completer_proxy_model.h"
#include "../../../../game/cards/card_database.h"
#include "../../../../game/cards/card_database_manager.h"
#include "../../../../game/cards/card_database_model.h"
#include "../../../../game/cards/card_search_model.h"
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
    connect(deckListModel, &DeckListModel::dataChanged, this, &VisualDeckEditorWidget::decklistDataChanged);

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

        CardInfoPtr card = CardDatabaseManager::getInstance()->getCard(searchBar->text());
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
        CardInfoPtr card = CardDatabaseManager::getInstance()->getCard(searchBar->text());
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

    updateZoneWidgets();

    cardSizeWidget = new CardSizeWidget(this);

    mainLayout->addWidget(groupAndSortContainer);
    mainLayout->addWidget(scrollArea);
    mainLayout->addWidget(cardSizeWidget);

    retranslateUi();
}

void VisualDeckEditorWidget::retranslateUi()
{
    sortLabel->setText(tr("Click and drag to change the sort order within the groups"));
    searchPushButton->setText(tr("Quick search and add card"));
    searchPushButton->setToolTip(tr("Search for closest match in the database (with auto-suggestions) and add "
                                    "preferred printing to the deck on pressing enter"));
    sortCriteriaButton->setToolTip(tr("Configure how cards are sorted within their groups"));
    displayTypeButton->setText(tr("Flat Layout"));
    displayTypeButton->setToolTip(
        tr("Change how cards are displayed within zones (i.e. overlapped or fully visible.)"));
}

void VisualDeckEditorWidget::updateZoneWidgets()
{
    addZoneIfDoesNotExist();
    deleteZoneIfDoesNotExist();
}

void VisualDeckEditorWidget::updateDisplayType()
{
    // Toggle the display type
    currentDisplayType = (currentDisplayType == DisplayType::Overlap) ? DisplayType::Flat : DisplayType::Overlap;

    // Update UI and emit signal
    switch (currentDisplayType) {
        case DisplayType::Flat:
            emit displayTypeChanged("flat");
            displayTypeButton->setText(tr("Flat Layout"));
            break;
        case DisplayType::Overlap:
            emit displayTypeChanged("overlap");
            displayTypeButton->setText(tr("Overlap Layout"));
            break;
    }
}

void VisualDeckEditorWidget::addZoneIfDoesNotExist()
{
    QList<DeckCardZoneDisplayWidget *> cardZoneDisplayWidgets =
        zoneContainer->findChildren<DeckCardZoneDisplayWidget *>();
    for (const QString &zone : *deckListModel->getZones()) {
        bool found = false;
        for (DeckCardZoneDisplayWidget *displayWidget : cardZoneDisplayWidgets) {
            if (displayWidget->zoneName == zone) {
                found = true;
                displayWidget->displayCards();
                break;
            }
        }

        if (found) {
            continue;
        }
        DeckCardZoneDisplayWidget *zoneDisplayWidget = new DeckCardZoneDisplayWidget(
            zoneContainer, deckListModel, zone, activeGroupCriteria, activeSortCriteria, 20, 10, cardSizeWidget);
        connect(zoneDisplayWidget, &DeckCardZoneDisplayWidget::cardHovered, this, &VisualDeckEditorWidget::onHover);
        connect(zoneDisplayWidget, &DeckCardZoneDisplayWidget::cardClicked, this, &VisualDeckEditorWidget::onCardClick);
        connect(this, &VisualDeckEditorWidget::activeSortCriteriaChanged, zoneDisplayWidget,
                &DeckCardZoneDisplayWidget::onActiveSortCriteriaChanged);
        connect(this, &VisualDeckEditorWidget::activeGroupCriteriaChanged, zoneDisplayWidget,
                &DeckCardZoneDisplayWidget::onActiveGroupCriteriaChanged);
        connect(this, &VisualDeckEditorWidget::displayTypeChanged, zoneDisplayWidget,
                &DeckCardZoneDisplayWidget::refreshDisplayType);
        zoneContainerLayout->addWidget(zoneDisplayWidget);
    }
}

void VisualDeckEditorWidget::deleteZoneIfDoesNotExist()
{
    QList<DeckCardZoneDisplayWidget *> cardZoneDisplayWidgets =
        zoneContainer->findChildren<DeckCardZoneDisplayWidget *>();
    for (DeckCardZoneDisplayWidget *displayWidget : cardZoneDisplayWidgets) {
        bool found = false;
        for (const QString &zone : *deckListModel->getZones()) {
            if (displayWidget->zoneName == zone) {
                found = true;
                break;
            }
        }

        if (!found) {
            zoneContainerLayout->removeWidget(displayWidget);
        }
    }
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

void VisualDeckEditorWidget::decklistDataChanged(QModelIndex topLeft, QModelIndex bottomRight)
{
    // Might use these at some point.
    Q_UNUSED(topLeft);
    Q_UNUSED(bottomRight);
    // Necessary to delay this in this manner else the updateDisplay will nuke widgets while their onClick event
    // hasn't returned yet. Interval of 0 means QT will schedule this after the current event loop has finished.
    updateZoneWidgets();
}

void VisualDeckEditorWidget::onHover(CardInfoPtr hoveredCard)
{
    emit activeCardChanged(hoveredCard);
}

void VisualDeckEditorWidget::onCardClick(QMouseEvent *event,
                                         CardInfoPictureWithTextOverlayWidget *instance,
                                         QString zoneName)
{
    emit cardClicked(event, instance, zoneName);
}
