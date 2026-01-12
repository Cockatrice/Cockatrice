#include "deck_editor_database_display_widget.h"

#include "../../../client/settings/cache_settings.h"
#include "../../../filters/syntax_help.h"
#include "../../../interface/widgets/tabs/abstract_tab_deck_editor.h"
#include "../../../interface/widgets/tabs/tab_supervisor.h"
#include "../../pixel_map_generator.h"

#include <QClipboard>
#include <QHeaderView>
#include <QMenu>
#include <QToolButton>
#include <QTreeView>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/card/relation/card_relation.h>

static bool canBeCommander(const CardInfo &cardInfo)
{
    return (cardInfo.getCardType().contains("Legendary", Qt::CaseInsensitive) &&
            cardInfo.getCardType().contains("Creature", Qt::CaseInsensitive)) ||
           cardInfo.getText().contains("can be your commander", Qt::CaseInsensitive);
}

DeckEditorDatabaseDisplayWidget::DeckEditorDatabaseDisplayWidget(QWidget *parent, AbstractTabDeckEditor *deckEditor)
    : QWidget(parent), deckEditor(deckEditor)
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
    searchEdit->installEventFilter(&searchKeySignals);

    setFocusProxy(searchEdit);
    setFocusPolicy(Qt::ClickFocus);

    searchKeySignals.setObjectName("searchKeySignals");
    connect(searchEdit, &SearchLineEdit::textChanged, this, &DeckEditorDatabaseDisplayWidget::updateSearch);
    connect(&searchKeySignals, &KeySignals::onEnter, this, &DeckEditorDatabaseDisplayWidget::actAddCardToMainDeck);
    connect(&searchKeySignals, &KeySignals::onCtrlAltEqual, this,
            &DeckEditorDatabaseDisplayWidget::actAddCardToMainDeck);
    connect(&searchKeySignals, &KeySignals::onCtrlAltRBracket, this,
            &DeckEditorDatabaseDisplayWidget::actAddCardToSideboard);
    connect(&searchKeySignals, &KeySignals::onCtrlAltMinus, this,
            &DeckEditorDatabaseDisplayWidget::actDecrementCardFromMainDeck);
    connect(&searchKeySignals, &KeySignals::onCtrlAltLBracket, this,
            &DeckEditorDatabaseDisplayWidget::actDecrementCardFromSideboard);
    connect(&searchKeySignals, &KeySignals::onCtrlAltEnter, this,
            &DeckEditorDatabaseDisplayWidget::actAddCardToSideboard);
    connect(&searchKeySignals, &KeySignals::onCtrlEnter, this, &DeckEditorDatabaseDisplayWidget::actAddCardToSideboard);
    connect(&searchKeySignals, &KeySignals::onCtrlC, this, &DeckEditorDatabaseDisplayWidget::copyDatabaseCellContents);
    connect(help, &QAction::triggered, this, [this] { createSearchSyntaxHelpWindow(searchEdit); });

    databaseModel = new CardDatabaseModel(CardDatabaseManager::getInstance(), true, this);
    databaseModel->setObjectName("databaseModel");
    databaseDisplayModel = new CardDatabaseDisplayModel(this);
    databaseDisplayModel->setObjectName("databaseDisplayModel");
    databaseDisplayModel->setSourceModel(databaseModel);
    databaseDisplayModel->setFilterKeyColumn(0);

    databaseView = new QTreeView(this);
    databaseView->setObjectName("databaseView");
    databaseView->setFocusProxy(searchEdit);
    databaseView->setUniformRowHeights(true);
    databaseView->setRootIsDecorated(false);
    databaseView->setAlternatingRowColors(true);
    databaseView->setSortingEnabled(true);
    databaseView->sortByColumn(0, Qt::AscendingOrder);
    databaseView->setModel(databaseDisplayModel);
    databaseView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(databaseView, &QTreeView::customContextMenuRequested, this,
            &DeckEditorDatabaseDisplayWidget::databaseCustomMenu);
    connect(databaseView->selectionModel(), &QItemSelectionModel::currentRowChanged, this,
            &DeckEditorDatabaseDisplayWidget::updateCard);
    connect(databaseView, &QTreeView::doubleClicked, this, &DeckEditorDatabaseDisplayWidget::actAddCardToMainDeck);

    QByteArray dbHeaderState = SettingsCache::instance().layouts().getDeckEditorDbHeaderState();
    if (dbHeaderState.isNull()) {
        // first run
        databaseView->setColumnWidth(0, 200);
    } else {
        databaseView->header()->restoreState(dbHeaderState);
    }
    connect(databaseView->header(), &QHeaderView::geometriesChanged, this,
            &DeckEditorDatabaseDisplayWidget::saveDbHeaderState);

    searchEdit->setTreeView(databaseView);

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

void DeckEditorDatabaseDisplayWidget::updateSearch(const QString &search)
{
    databaseDisplayModel->setStringFilter(search);
    QModelIndexList sel = databaseView->selectionModel()->selectedRows();
    if (sel.isEmpty() && databaseDisplayModel->rowCount())
        databaseView->selectionModel()->setCurrentIndex(databaseDisplayModel->index(0, 0),
                                                        QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
}

void DeckEditorDatabaseDisplayWidget::clearAllDatabaseFilters()
{
    databaseDisplayModel->clearFilterAll();
    searchEdit->setText("");
}

void DeckEditorDatabaseDisplayWidget::updateCard(const QModelIndex &current, const QModelIndex & /*previous*/)
{
    if (!current.isValid()) {
        return;
    }

    const QString cardName = current.siblingAtColumn(CardDatabaseModel::NameColumn).data().toString();

    if (!current.model()->hasChildren(current.siblingAtColumn(CardDatabaseModel::NameColumn))) {
        emit cardChanged(CardDatabaseManager::query()->getPreferredCard(cardName));
    }
}

void DeckEditorDatabaseDisplayWidget::actAddCardToMainDeck()
{
    emit addCardToMainDeck(currentCard());
}

void DeckEditorDatabaseDisplayWidget::actAddCardToSideboard()
{
    emit addCardToSideboard(currentCard());
}

void DeckEditorDatabaseDisplayWidget::actDecrementCardFromMainDeck()
{
    emit decrementCardFromMainDeck(currentCard());
}

void DeckEditorDatabaseDisplayWidget::actDecrementCardFromSideboard()
{
    emit decrementCardFromSideboard(currentCard());
}

ExactCard DeckEditorDatabaseDisplayWidget::currentCard() const
{
    const QModelIndex currentIndex = databaseView->selectionModel()->currentIndex();
    if (!currentIndex.isValid()) {
        return {};
    }

    const QString cardName = currentIndex.siblingAtColumn(CardDatabaseModel::NameColumn).data().toString();

    return CardDatabaseManager::query()->getPreferredCard(cardName);
}

void DeckEditorDatabaseDisplayWidget::databaseCustomMenu(QPoint point)
{
    QMenu menu;
    ExactCard card = currentCard();

    if (card) {
        // add to deck and sideboard options
        QAction *addToDeck, *addToSideboard, *selectPrinting, *edhRecCommander, *edhRecCard;
        addToDeck = menu.addAction(tr("Add to Deck"));
        addToSideboard = menu.addAction(tr("Add to Sideboard"));
        if (!SettingsCache::instance().getOverrideAllCardArtWithPersonalPreference()) {
            selectPrinting = menu.addAction(tr("Select Printing"));
            connect(selectPrinting, &QAction::triggered, this, [this, card] { deckEditor->showPrintingSelector(); });
        }
        if (canBeCommander(card.getInfo())) {
            edhRecCommander = menu.addAction(tr("Show on EDHRec (Commander)"));
            connect(edhRecCommander, &QAction::triggered, this,
                    [this, card] { deckEditor->getTabSupervisor()->addEdhrecTab(card.getCardPtr(), true); });
        }
        edhRecCard = menu.addAction(tr("Show on EDHRec (Card)"));

        connect(addToDeck, &QAction::triggered, this, &DeckEditorDatabaseDisplayWidget::actAddCardToMainDeck);
        connect(addToSideboard, &QAction::triggered, this, &DeckEditorDatabaseDisplayWidget::actAddCardToSideboard);
        connect(edhRecCard, &QAction::triggered, this,
                [this, card] { deckEditor->getTabSupervisor()->addEdhrecTab(card.getCardPtr()); });

        // filling out the related cards submenu
        auto *relatedMenu = new QMenu(tr("Show Related cards"));
        menu.addMenu(relatedMenu);
        auto relatedCards = card.getInfo().getAllRelatedCards();
        if (relatedCards.isEmpty()) {
            relatedMenu->setDisabled(true);
        } else {
            for (const CardRelation *rel : relatedCards) {
                const QString &relatedCardName = rel->getName();
                QAction *relatedCard = relatedMenu->addAction(relatedCardName);
                connect(
                    relatedCard, &QAction::triggered, deckEditor->cardInfoDockWidget->cardInfo,
                    [this, relatedCardName] { deckEditor->cardInfoDockWidget->cardInfo->setCard(relatedCardName); });
            }
        }
        menu.exec(databaseView->mapToGlobal(point));
    }
}

void DeckEditorDatabaseDisplayWidget::copyDatabaseCellContents()
{
    auto _data = databaseView->selectionModel()->currentIndex().data();
    QApplication::clipboard()->setText(_data.toString());
}

void DeckEditorDatabaseDisplayWidget::saveDbHeaderState()
{
    SettingsCache::instance().layouts().setDeckEditorDbHeaderState(databaseView->header()->saveState());
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