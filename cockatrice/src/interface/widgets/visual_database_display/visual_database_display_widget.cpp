#include "visual_database_display_widget.h"

#include "../../../client/settings/cache_settings.h"
#include "../../../filters/filter_tree_model.h"
#include "../../../filters/syntax_help.h"
#include "../../key_signals.h"
#include "../../pixel_map_generator.h"
#include "../cards/card_size_widget.h"
#include "../deck_editor/card_database_view.h"
#include "../general/layout_containers/flow_widget.h"
#include "../utility/custom_line_edit.h"
#include "card_database_display_model.h"
#include "card_database_model.h"
#include "card_database_visual_display_model.h"
#include "card_database_visual_view.h"
#include "deck_list_model.h"
#include "libcockatrice/card/card_localization.h"
#include "libcockatrice/card/database/card_database_loader.h"
#include "libcockatrice/card/database/card_database_querier.h"
#include "libcockatrice/card/printing/exact_card.h"
#include "libcockatrice/filters/filter_card.h"
#include "libcockatrice/filters/filter_tree.h"
#include "visual_database_display_color_filter_widget.h"
#include "visual_database_display_filter_toolbar_widget.h"

#include <QAction>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSizePolicy>
#include <QSlider>
#include <QStringLiteral>
#include <QStyledItemDelegate>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>
#include <QVariant>
#include <QtGlobal>
#include <libcockatrice/card/database/card_database.h>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/deck_list/tree/inner_deck_list_node.h>
#include <libcockatrice/settings/cards_display_settings.h>
#include <libcockatrice/utility/qt_utils.h>

class QMouseEvent;

VisualDatabaseDisplayWidget::VisualDatabaseDisplayWidget(QWidget *parent,
                                                         CardDatabaseModel *database_model,
                                                         DeckListModel *deckListModel)
    : QWidget(parent)
{
    // Create display model
    databaseDisplayModel = new CardDatabaseDisplayModel(this);
    databaseDisplayModel->setObjectName("databaseDisplayModel");
    databaseDisplayModel->setSourceModel(database_model);
    databaseDisplayModel->setFilterKeyColumn(0);

    const auto applyCardSearchLanguage = [this]() {
        const CardsDisplaySettings &cardsDisplay = SettingsCache::instance().cardsDisplay();
        databaseDisplayModel->setSearchLanguage(CardSearchLanguage{
            cardsDisplay.getCardLang(), static_cast<SearchLanguageMode>(cardsDisplay.getCardSearchLanguage())});
    };
    applyCardSearchLanguage();
    connect(&SettingsCache::instance().cardsDisplay(), &CardsDisplaySettings::cardLangChanged, this,
            applyCardSearchLanguage);
    connect(&SettingsCache::instance().cardsDisplay(), &CardsDisplaySettings::cardSearchLanguageChanged, this,
            applyCardSearchLanguage);

    // Set up main layout and widgets
    setMinimumSize(0, 0);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mainLayout = QtUtils::makeLayout<QVBoxLayout>(this, 0, 1);
    setLayout(mainLayout);

    searchContainer = new FlowWidget(this, Qt::Horizontal, Qt::ScrollBarAlwaysOff, Qt::ScrollBarAlwaysOff);

    searchEdit = new SearchLineEdit();
    searchEdit->setObjectName("searchEdit");
    searchEdit->setPlaceholderText(tr("Search by card name (or search expressions)"));
    searchEdit->setClearButtonEnabled(true);
    searchEdit->addAction(loadColorAdjustedPixmap("theme:icons/search"), QLineEdit::LeadingPosition);
    auto help = searchEdit->addAction(themePixmap(QStringLiteral("icons/info")), QLineEdit::TrailingPosition);
    connect(help, &QAction::triggered, this, [this] { createSearchSyntaxHelpWindow(searchEdit); });

    setFocusProxy(searchEdit);
    setFocusPolicy(Qt::ClickFocus);

    displayModeButton = new QPushButton(this);
    displayModeButton->setCheckable(true); // Toggle button

    connect(displayModeButton, &QPushButton::toggled, this, &VisualDatabaseDisplayWidget::onDisplayModeChanged);

    displayModeButton->setChecked(false); // Start in Visual mode

    filterModel = new FilterTreeModel();
    filterModel->setObjectName("filterModel");

    connect(searchEdit, &SearchLineEdit::textChanged, databaseDisplayModel, &CardDatabaseDisplayModel::setStringFilter);

    databaseView = new CardDatabaseView(this, databaseDisplayModel);
    databaseView->setObjectName("databaseView");
    databaseView->setFocusProxy(searchEdit);
    databaseView->setItemDelegate(nullptr);
    databaseView->setVisible(false);

    // Without a deck model there is nothing to add cards to, so the zone menu stays hidden.
    if (deckListModel) {
        databaseView->setZoneMenuProvider(
            [deckListModel]() -> QList<QPair<QString, QStringList>> {
                QList<QPair<QString, QStringList>> result;
                for (const QString &boardName : InnerDecklistNode::boardZoneNames()) {
                    result.append({boardName, deckListModel->getCustomZoneNames(boardName)});
                }
                return result;
            },
            [this] { return newZoneCreator ? newZoneCreator() : QString(); });
    }

    searchEdit->setTreeView(databaseView);
    searchEdit->installEventFilter(databaseView->getKeySignals());

    connect(databaseView, &CardDatabaseView::cardChanged, this, &VisualDatabaseDisplayWidget::onSelectedCardChanged);
    connect(databaseView, &CardDatabaseView::cardAdded, this, &VisualDatabaseDisplayWidget::actAddCard);
    connect(databaseView, &CardDatabaseView::cardDecremented, this, &VisualDatabaseDisplayWidget::actDecrementCard);
    connect(databaseView, &CardDatabaseView::edhrecClicked, this, &VisualDatabaseDisplayWidget::edhrecRequested);
    connect(databaseView, &CardDatabaseView::selectPrintingClicked, this,
            &VisualDatabaseDisplayWidget::printingSelectorRequested);
    connect(databaseView, &CardDatabaseView::relatedCardClicked, this,
            &VisualDatabaseDisplayWidget::onRelatedCardClicked);

    // The visual grid is a real view on a flattened model: one row per displayed
    // card picture, painted on demand.
    visualDisplayModel = new CardDatabaseVisualDisplayModel(databaseDisplayModel, CardDatabaseManager::query(), this);

    cardView = new CardDatabaseVisualView(visualDisplayModel, this);
    cardView->setObjectName("visualCardView");
    connect(cardView, &CardDatabaseVisualView::cardClicked, this,
            &VisualDatabaseDisplayWidget::cardClickedDatabaseDisplay);
    connect(cardView, &CardDatabaseVisualView::cardHovered, this,
            &VisualDatabaseDisplayWidget::cardHoveredDatabaseDisplay);

    const int cardSize = SettingsCache::instance().cardsDisplay().getVisualDatabaseDisplayCardSize();
    cardSizeWidget = new CardSizeWidget(this, nullptr, cardSize);
    connect(cardSizeWidget, &CardSizeWidget::cardSizeSettingUpdated, &SettingsCache::instance().cardsDisplay(),
            &CardsDisplaySettings::setVisualDatabaseDisplayCardSize);
    connect(cardSizeWidget->getSlider(), &QSlider::valueChanged, cardView, &CardDatabaseVisualView::setCardSizeFactor);
    cardView->setCardSizeFactor(cardSize);
    cardSizeWidget->enableCtrlScrollResize(cardView);

    colorFilterWidget = new VisualDatabaseDisplayColorFilterWidget(this, filterModel);

    filterContainer = new VisualDatabaseDisplayFilterToolbarWidget(this, deckListModel);

    clearFilterWidget = new QToolButton();
    clearFilterWidget->setFixedSize(32, 32);
    clearFilterWidget->setIcon(themePixmap(QStringLiteral("icons/delete")));
    connect(clearFilterWidget, &QToolButton::clicked, this, [this] {
        filterModel->blockSignals(true);
        filterModel->filterTree()->blockSignals(true);
        filterModel->clear();
        filterModel->blockSignals(false);
        filterModel->filterTree()->blockSignals(false);
        emit filterModel->filterTree()->changed();
        emit filterModel->layoutChanged();
    });

    databaseLoadIndicator = new QLabel(this);
    databaseLoadIndicator->setAlignment(Qt::AlignCenter);

    mainLayout->addWidget(databaseLoadIndicator);

    if (CardDatabaseManager::getInstance()->getLoadStatus() != LoadStatus::Ok) {
        connect(CardDatabaseManager::getInstance(), &CardDatabase::cardDatabaseLoadingFinished, this,
                &VisualDatabaseDisplayWidget::initialize);
        filterContainer->setVisible(false);
    } else {
        initialize();
        databaseLoadIndicator->setVisible(false);
    }

    retranslateUi();
}

void VisualDatabaseDisplayWidget::initialize()
{
    databaseLoadIndicator->setVisible(false);

    searchContainer->addWidget(colorFilterWidget);
    searchContainer->addWidget(clearFilterWidget);
    searchContainer->addWidget(searchEdit);
    searchContainer->addWidget(displayModeButton);

    mainLayout->addWidget(searchContainer);

    mainLayout->addWidget(filterContainer);

    mainLayout->addWidget(databaseView);

    mainLayout->addWidget(cardView);

    mainLayout->addWidget(cardSizeWidget);

    connect(filterModel, &FilterTreeModel::layoutChanged, this, &VisualDatabaseDisplayWidget::onSearchModelChanged);

    initializeFilters();
}

void VisualDatabaseDisplayWidget::initializeFilters()
{
    if (filtersInitialized || !isVisible() || CardDatabaseManager::getInstance()->getLoadStatus() != LoadStatus::Ok) {
        return;
    }

    filtersInitialized = true;

    // The filter toolbar builds its widgets by iterating the entire card database
    // (per-set, per-main-type, per-sub-type and per-format buttons). Building it
    // inside showEvent would block the tab switch, so keep it hidden and defer the
    // build to the next event loop turn, letting the tab paint first. The toolbar
    // then appears one event loop turn later, shifting the grid down by the toolbar
    // height -- the intended tradeoff of an responsive tab switch.
    filterContainer->setVisible(false);

    QTimer::singleShot(0, this, [this] {
        filterContainer->initialize();
        filterContainer->setVisible(true);

        databaseDisplayModel->setFilterTree(filterModel->filterTree());

        retranslateUi();
    });
}

void VisualDatabaseDisplayWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    initializeFilters();
}

void VisualDatabaseDisplayWidget::setNewZoneCreator(const std::function<QString()> &creator)
{
    newZoneCreator = creator;
}

void VisualDatabaseDisplayWidget::retranslateUi()
{
    databaseLoadIndicator->setText(tr("Loading database ..."));
    clearFilterWidget->setToolTip(tr("Clear all filters"));
    searchEdit->setPlaceholderText(tr("Search by card name (or search expressions)"));
    updateDisplayModeButton();
    colorFilterWidget->retranslateUi();
    filterContainer->retranslateUi();
}

void VisualDatabaseDisplayWidget::highlightAllSearchEdit()
{
    searchEdit->setSelection(0, searchEdit->text().length());
}

void VisualDatabaseDisplayWidget::updateDisplayModeButton()
{
    displayModeButton->setText(displayModeButton->isChecked() ? tr("Table") : tr("Visual"));
}

void VisualDatabaseDisplayWidget::onDisplayModeChanged(bool checked)
{
    updateDisplayModeButton();
    if (checked) {
        // Table mode
        cardView->setVisible(false);
        cardSizeWidget->setVisible(false);
        databaseView->setItemDelegate(new QStyledItemDelegate(databaseView));
        databaseView->setVisible(true);
    } else {
        // Visual mode
        cardView->setVisible(true);
        cardSizeWidget->setVisible(true);
        databaseView->setVisible(false);
    }
}

bool VisualDatabaseDisplayWidget::isVisualDisplayMode() const
{
    return !displayModeButton->isChecked();
}

QStringList VisualDatabaseDisplayWidget::collectSetFilterTerms() const
{
    return filterModel->getFilterTermsOfType(CardFilter::AttrSet);
}

void VisualDatabaseDisplayWidget::onSearchModelChanged()
{
    visualDisplayModel->setSetFilterTerms(collectSetFilterTerms());
}

void VisualDatabaseDisplayWidget::onSelectedCardChanged(const QString &cardName)
{
    emit cardHoveredDatabaseDisplay(CardDatabaseManager::query()->getPreferredCard(cardName));
}

void VisualDatabaseDisplayWidget::actAddCard(const QString &cardName, const QString &zoneName)
{
    highlightAllSearchEdit();
    ExactCard exactCard = CardDatabaseManager::query()->getPreferredCard(cardName);
    emit cardAdded(exactCard, zoneName);
}

void VisualDatabaseDisplayWidget::actDecrementCard(const QString &cardName, const QString &zoneName)
{
    ExactCard exactCard = CardDatabaseManager::query()->getPreferredCard(cardName);
    emit cardDecremented(exactCard, zoneName);
}

void VisualDatabaseDisplayWidget::onRelatedCardClicked(const QString &relatedCard)
{
    ExactCard exactCard = CardDatabaseManager::query()->guessCard({relatedCard});
    emit cardInfoRequested(exactCard);
}
