#include "deck_editor_database_display_widget.h"

#include "../../../../game/cards/card_database_manager.h"
#include "../../../../settings/cache_settings.h"
#include "../../../tabs/abstract_tab_deck_editor.h"
#include "../../../tabs/tab_supervisor.h"

#include <QClipboard>
#include <QFile>
#include <QHeaderView>
#include <QMenu>
#include <QTextBrowser>
#include <QToolButton>
#include <QTreeView>

static bool canBeCommander(const CardInfoPtr &cardInfo)
{
    return ((cardInfo->getCardType().contains("Legendary", Qt::CaseInsensitive) &&
             cardInfo->getCardType().contains("Creature", Qt::CaseInsensitive))) ||
           cardInfo->getText().contains("can be your commander", Qt::CaseInsensitive);
}

DeckEditorDatabaseDisplayWidget::DeckEditorDatabaseDisplayWidget(QWidget *parent, AbstractTabDeckEditor *_deckEditor)
    : QWidget(parent), deckEditor(_deckEditor)
{
    setObjectName("centralWidget");

    centralFrame = new QVBoxLayout(this);
    centralFrame->setObjectName("centralFrame");
    setLayout(centralFrame);

    searchEdit = new SearchLineEdit();
    searchEdit->setObjectName("searchEdit");
    searchEdit->setPlaceholderText(tr("Search by card name (or search expressions)"));
    searchEdit->setClearButtonEnabled(true);
    // searchEdit->addAction(loadColorAdjustedPixmap("theme:icons/search"), QLineEdit::LeadingPosition);
    auto help = searchEdit->addAction(QPixmap("theme:icons/info"), QLineEdit::TrailingPosition);
    searchEdit->installEventFilter(&searchKeySignals);

    setFocusProxy(searchEdit);
    setFocusPolicy(Qt::ClickFocus);

    searchKeySignals.setObjectName("searchKeySignals");
    connect(searchEdit, SIGNAL(textChanged(const QString &)), this, SLOT(updateSearch(const QString &)));
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
    connect(help, &QAction::triggered, this, &DeckEditorDatabaseDisplayWidget::showSearchSyntaxHelp);

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
    connect(databaseView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(databaseCustomMenu(QPoint)));
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
    connect(databaseView->header(), SIGNAL(geometriesChanged()), this, SLOT(saveDbHeaderState()));

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

void DeckEditorDatabaseDisplayWidget::updateCard(const QModelIndex &current, const QModelIndex & /*previous*/)
{
    const QString cardName = current.sibling(current.row(), 0).data().toString();
    const QString cardProviderID = CardDatabaseManager::getInstance()->getPreferredPrintingProviderIdForCard(cardName);

    if (!current.isValid()) {
        return;
    }

    if (!current.model()->hasChildren(current.sibling(current.row(), 0))) {
        CardInfoPtr card = CardDatabaseManager::getInstance()->getCardByNameAndProviderId(cardName, cardProviderID);
        emit cardChanged(card);
    }
}

void DeckEditorDatabaseDisplayWidget::actAddCardToMainDeck()
{
    emit addCardToMainDeck(currentCardInfo());
}

void DeckEditorDatabaseDisplayWidget::actAddCardToSideboard()
{
    emit addCardToSideboard(currentCardInfo());
}

void DeckEditorDatabaseDisplayWidget::actDecrementCardFromMainDeck()
{
    emit decrementCardFromMainDeck(currentCardInfo());
}

void DeckEditorDatabaseDisplayWidget::actDecrementCardFromSideboard()
{
    emit decrementCardFromSideboard(currentCardInfo());
}

CardInfoPtr DeckEditorDatabaseDisplayWidget::currentCardInfo() const
{
    const QModelIndex currentIndex = databaseView->selectionModel()->currentIndex();
    if (!currentIndex.isValid()) {
        return {};
    }

    const QString cardName = currentIndex.sibling(currentIndex.row(), 0).data().toString();

    return CardDatabaseManager::getInstance()->getCard(cardName);
}

void DeckEditorDatabaseDisplayWidget::databaseCustomMenu(QPoint point)
{
    QMenu menu;
    const CardInfoPtr info = currentCardInfo();

    if (info) {
        // add to deck and sideboard options
        QAction *addToDeck, *addToSideboard, *selectPrinting, *edhRecCommander, *edhRecCard;
        addToDeck = menu.addAction(tr("Add to Deck"));
        addToSideboard = menu.addAction(tr("Add to Sideboard"));
        selectPrinting = menu.addAction(tr("Select Printing"));
        if (canBeCommander(info)) {
            edhRecCommander = menu.addAction(tr("Show on EDHREC (Commander)"));
            connect(edhRecCommander, &QAction::triggered, this,
                    [this, info] { deckEditor->getTabSupervisor()->addEdhrecTab(info, true); });
        }
        edhRecCard = menu.addAction(tr("Show on EDHREC (Card)"));

        connect(addToDeck, &QAction::triggered, this, &DeckEditorDatabaseDisplayWidget::actAddCardToMainDeck);
        connect(addToSideboard, &QAction::triggered, this, &DeckEditorDatabaseDisplayWidget::actAddCardToSideboard);
        connect(selectPrinting, &QAction::triggered, this, [this, info] { deckEditor->showPrintingSelector(); });
        connect(edhRecCard, &QAction::triggered, this,
                [this, info] { deckEditor->getTabSupervisor()->addEdhrecTab(info); });

        // filling out the related cards submenu
        auto *relatedMenu = new QMenu(tr("Show Related cards"));
        menu.addMenu(relatedMenu);
        auto relatedCards = info->getAllRelatedCards();
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

void DeckEditorDatabaseDisplayWidget::showSearchSyntaxHelp()
{

    QFile file("theme:help/search.md");

    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        return;
    }

    QTextStream in(&file);
    QString text = in.readAll();
    file.close();

    // Poor Markdown Converter
    auto opts = QRegularExpression::MultilineOption;
    text = text.replace(QRegularExpression("^(###)(.*)", opts), "<h3>\\2</h3>")
               .replace(QRegularExpression("^(##)(.*)", opts), "<h2>\\2</h2>")
               .replace(QRegularExpression("^(#)(.*)", opts), "<h1>\\2</h1>")
               .replace(QRegularExpression("^------*", opts), "<hr />")
               .replace(QRegularExpression(R"(\[([^[]+)\]\(([^\)]+)\))", opts), R"(<a href='\2'>\1</a>)");

    auto browser = new QTextBrowser;
    browser->setParent(this, Qt::Window | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint |
                                 Qt::WindowCloseButtonHint | Qt::WindowFullscreenButtonHint);
    browser->setWindowTitle("Search Help");
    browser->setReadOnly(true);
    browser->setMinimumSize({500, 600});

    QString sheet = QString("a { text-decoration: underline; color: rgb(71,158,252) };");
    browser->document()->setDefaultStyleSheet(sheet);

    browser->setHtml(text);
    connect(browser, &QTextBrowser::anchorClicked, [this](const QUrl &link) { searchEdit->setText(link.fragment()); });
    browser->show();
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