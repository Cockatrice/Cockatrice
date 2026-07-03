#include "deck_editor_database_display_widget.h"

#include "../../../client/settings/cache_settings.h"
#include "../../../filters/syntax_help.h"
#include "../../../interface/widgets/tabs/abstract_tab_deck_editor.h"
#include "../../../interface/widgets/tabs/tab_supervisor.h"
#include "../../pixel_map_generator.h"
#include "card_database_view.h"

#include <QClipboard>
#include <QHeaderView>
#include <QToolButton>
#include <QTreeView>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/card/relation/card_relation.h>

DeckEditorDatabaseDisplayWidget::DeckEditorDatabaseDisplayWidget(QWidget *parent, CardDatabaseModel *databaseModel)
    : QWidget(parent)
{
    setObjectName("databaseDisplayWidget");

    centralFrame = new QVBoxLayout(this);
    centralFrame->setObjectName("centralFrame");
    setLayout(centralFrame);

    searchEdit = new SearchLineEdit();
    searchEdit->setObjectName("searchEdit");
    searchEdit->setPlaceholderText(tr("Search by card name (or search expressions)"));
    searchEdit->setClearButtonEnabled(true);
    searchEdit->addAction(loadColorAdjustedPixmap("theme:icons/search"), QLineEdit::LeadingPosition);
    auto help = searchEdit->addAction(QPixmap("theme:icons/info"), QLineEdit::TrailingPosition);

    setFocusProxy(searchEdit);
    setFocusPolicy(Qt::ClickFocus);

    connect(help, &QAction::triggered, this, [this] { createSearchSyntaxHelpWindow(searchEdit); });

    databaseDisplayModel = new CardDatabaseDisplayModel(this);
    databaseDisplayModel->setObjectName("databaseDisplayModel");
    databaseDisplayModel->setSourceModel(databaseModel);
    databaseDisplayModel->setFilterKeyColumn(0);

    databaseView = new CardDatabaseView(this, databaseDisplayModel);
    databaseView->setObjectName("databaseView");
    databaseView->setFocusProxy(searchEdit);

    searchEdit->setTreeView(databaseView);
    searchEdit->installEventFilter(databaseView->getKeySignals());

    connect(searchEdit, &SearchLineEdit::textChanged, databaseDisplayModel, &CardDatabaseDisplayModel::setStringFilter);
    connect(databaseView, &CardDatabaseView::cardAdded, this, &DeckEditorDatabaseDisplayWidget::addCard);
    connect(databaseView, &CardDatabaseView::cardDecremented, this, &DeckEditorDatabaseDisplayWidget::decrementCard);
    connect(databaseView, &CardDatabaseView::cardChanged, this, &DeckEditorDatabaseDisplayWidget::updateCard);

    connect(databaseView, &CardDatabaseView::edhrecClicked, this, &DeckEditorDatabaseDisplayWidget::edhrecRequested);
    connect(databaseView, &CardDatabaseView::selectPrintingClicked, this,
            &DeckEditorDatabaseDisplayWidget::printingSelectorRequested);
    connect(databaseView, &CardDatabaseView::relatedCardClicked, this,
            &DeckEditorDatabaseDisplayWidget::onRelatedCardClicked);

    aAddCard = new QAction(QString(), this);
    aAddCard->setIcon(QPixmap("theme:icons/arrow_right_green"));
    connect(aAddCard, &QAction::triggered, this, &DeckEditorDatabaseDisplayWidget::actAddCardToMainDeck);
    auto *tbAddCard = new QToolButton(this);
    tbAddCard->setDefaultAction(aAddCard);

    aAddCardToSideboard = new QAction(QString(), this);
    aAddCardToSideboard->setIcon(QPixmap("theme:icons/arrow_right_blue"));
    connect(aAddCardToSideboard, &QAction::triggered, this, &DeckEditorDatabaseDisplayWidget::actAddCardToSideboard);
    auto *tbAddCardToSideboard = new QToolButton(this);
    tbAddCardToSideboard->setDefaultAction(aAddCardToSideboard);

    searchLayout = new QHBoxLayout;
    searchLayout->setObjectName("searchLayout");
    searchLayout->addWidget(searchEdit);
    searchLayout->addWidget(tbAddCard);
    searchLayout->addWidget(tbAddCardToSideboard);

    centralFrame->addLayout(searchLayout);
    centralFrame->addWidget(databaseView);

    retranslateUi();
}

void DeckEditorDatabaseDisplayWidget::clearAllDatabaseFilters()
{
    databaseDisplayModel->clearFilterAll();
    searchEdit->setText("");
}

void DeckEditorDatabaseDisplayWidget::actAddCardToMainDeck()
{
    addCard(databaseView->currentCardName(), DECK_ZONE_MAIN);
}

void DeckEditorDatabaseDisplayWidget::actAddCardToSideboard()
{
    addCard(databaseView->currentCardName(), DECK_ZONE_SIDE);
}

void DeckEditorDatabaseDisplayWidget::addCard(const QString &cardName, const QString &zoneName)
{
    highlightAllSearchEdit();
    ExactCard exactCard = CardDatabaseManager::query()->getPreferredCard(cardName);
    emit cardAdded(exactCard, zoneName);
}

void DeckEditorDatabaseDisplayWidget::decrementCard(const QString &cardName, const QString &zoneName)
{
    ExactCard exactCard = CardDatabaseManager::query()->getPreferredCard(cardName);
    emit cardDecremented(exactCard, zoneName);
}

void DeckEditorDatabaseDisplayWidget::updateCard(const QString &cardName)
{
    ExactCard exactCard = CardDatabaseManager::query()->getPreferredCard(cardName);
    emit cardChanged(exactCard);
}

void DeckEditorDatabaseDisplayWidget::setFilterTree(FilterTree *filterTree)
{
    databaseDisplayModel->setFilterTree(filterTree);
}

void DeckEditorDatabaseDisplayWidget::retranslateUi()
{
    aAddCard->setText(tr("Add card to &maindeck"));
    aAddCardToSideboard->setText(tr("Add card to &sideboard"));
}

void DeckEditorDatabaseDisplayWidget::highlightAllSearchEdit()
{
    searchEdit->setSelection(0, searchEdit->text().length());
}

void DeckEditorDatabaseDisplayWidget::onRelatedCardClicked(const QString &relatedCard)
{
    ExactCard exactCard = CardDatabaseManager::query()->guessCard({relatedCard});
    emit cardInfoRequested(exactCard);
}