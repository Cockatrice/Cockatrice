#include "card_database_view.h"

#include "../../../client/settings/cache_settings.h"
#include "card_database_display_model.h"
#include "card_database_model.h"

#include <QApplication>
#include <QClipboard>
#include <QHeaderView>
#include <QMenu>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/card/relation/card_relation.h>
#include <libcockatrice/deck_list/tree/inner_deck_list_node.h>

static bool canBeCommander(const CardInfo &cardInfo)
{
    return (cardInfo.getCardType().contains("Legendary", Qt::CaseInsensitive) &&
            cardInfo.getCardType().contains("Creature", Qt::CaseInsensitive)) ||
           cardInfo.getText().contains("can be your commander", Qt::CaseInsensitive);
}

CardDatabaseView::CardDatabaseView(QWidget *parent, CardDatabaseDisplayModel *model)
    : QTreeView(parent), databaseDisplayModel(model)
{
    // set up object
    setUniformRowHeights(true);
    setRootIsDecorated(false);
    setAlternatingRowColors(true);
    setSortingEnabled(true);
    sortByColumn(0, Qt::AscendingOrder);
    QTreeView::setModel(databaseDisplayModel);
    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(databaseDisplayModel, &CardDatabaseDisplayModel::modelDirty, this,
            &CardDatabaseView::resetSelectionIfEmpty);

    connect(this, &QTreeView::customContextMenuRequested, this, &CardDatabaseView::openCustomMenu);
    connect(selectionModel(), &QItemSelectionModel::currentRowChanged, this, &CardDatabaseView::updateCard);
    connect(this, &QTreeView::doubleClicked, this, &CardDatabaseView::actDoubleClick);

    // layout settings
    QByteArray dbHeaderState = SettingsCache::instance().layouts().getDeckEditorDbHeaderState();
    if (dbHeaderState.isNull()) {
        // first run
        setColumnWidth(0, 200);
    } else {
        header()->restoreState(dbHeaderState);
    }
    connect(header(), &QHeaderView::geometriesChanged, this, &CardDatabaseView::saveDbHeaderState);

    // create key filters
    searchKeySignals.setObjectName("searchKeySignals");
    connect(&searchKeySignals, &KeySignals::onEnter, this, [this] { addCard(DECK_ZONE_MAIN); });
    connect(&searchKeySignals, &KeySignals::onCtrlAltEqual, this, [this] { addCard(DECK_ZONE_MAIN); });
    connect(&searchKeySignals, &KeySignals::onCtrlAltRBracket, this, [this] { addCard(DECK_ZONE_SIDE); });
    connect(&searchKeySignals, &KeySignals::onCtrlAltMinus, this, [this] { decrementCard(DECK_ZONE_MAIN); });
    connect(&searchKeySignals, &KeySignals::onCtrlAltLBracket, this, [this] { decrementCard(DECK_ZONE_SIDE); });
    connect(&searchKeySignals, &KeySignals::onCtrlAltEnter, this, [this] { addCard(DECK_ZONE_SIDE); });
    connect(&searchKeySignals, &KeySignals::onCtrlEnter, this, [this] { addCard(DECK_ZONE_SIDE); });
    connect(&searchKeySignals, &KeySignals::onCtrlC, this, &CardDatabaseView::copyDatabaseCellContents);
}

QString CardDatabaseView::currentCardName() const
{
    const QModelIndex currentIndex = selectionModel()->currentIndex();
    if (!currentIndex.isValid()) {
        return {};
    }

    return currentIndex.siblingAtColumn(CardDatabaseModel::NameColumn).data().toString();
}

void CardDatabaseView::actDoubleClick()
{
    if (QApplication::keyboardModifiers() & Qt::ControlModifier) {
        addCard(DECK_ZONE_SIDE);
    } else {
        addCard(DECK_ZONE_MAIN);
    }
}

void CardDatabaseView::addCard(const QString &zoneName)
{
    emit cardAdded(currentCardName(), zoneName);
}

void CardDatabaseView::decrementCard(const QString &zoneName)
{
    emit cardDecremented(currentCardName(), zoneName);
}

void CardDatabaseView::updateCard(const QModelIndex &current, const QModelIndex & /*previous*/)
{
    if (!current.isValid()) {
        return;
    }

    const QString cardName = current.siblingAtColumn(CardDatabaseModel::NameColumn).data().toString();

    if (!current.model()->hasChildren(current.siblingAtColumn(CardDatabaseModel::NameColumn))) {
        emit cardChanged(cardName);
    }
}

void CardDatabaseView::resetSelectionIfEmpty()
{
    QModelIndexList sel = selectionModel()->selectedRows();
    if (sel.isEmpty() && databaseDisplayModel->rowCount() > 0) {
        selectionModel()->setCurrentIndex(databaseDisplayModel->index(0, 0),
                                          QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
    }
}

void CardDatabaseView::copyDatabaseCellContents() const
{
    auto _data = selectionModel()->currentIndex().data();
    QApplication::clipboard()->setText(_data.toString());
}

void CardDatabaseView::saveDbHeaderState()
{
    SettingsCache::instance().layouts().setDeckEditorDbHeaderState(header()->saveState());
}

void CardDatabaseView::openCustomMenu(QPoint point)
{
    CardInfoPtr card = CardDatabaseManager::query()->getCardInfo(currentCardName());

    if (!card) {
        return;
    }

    QMenu menu;
    // add to deck and sideboard options
    QAction *addToDeck = menu.addAction(tr("Add to Deck"));
    QAction *addToSideboard = menu.addAction(tr("Add to Sideboard"));
    QAction *selectPrinting = menu.addAction(tr("Select Printing"));

    connect(addToDeck, &QAction::triggered, this, [this, card] { emit cardAdded(card->getName(), DECK_ZONE_MAIN); });
    connect(addToSideboard, &QAction::triggered, this,
            [this, card] { emit cardAdded(card->getName(), DECK_ZONE_SIDE); });
    connect(selectPrinting, &QAction::triggered, this, &CardDatabaseView::selectPrintingClicked);

    if (canBeCommander(*card)) {
        QAction *edhRecCommander = menu.addAction(tr("Show on EDHRec (Commander)"));
        connect(edhRecCommander, &QAction::triggered, this, [this, card] { emit edhrecClicked(card, true); });
    }
    QAction *edhRecCard = menu.addAction(tr("Show on EDHRec (Card)"));
    connect(edhRecCard, &QAction::triggered, this, [this, card] { emit edhrecClicked(card, false); });

    // filling out the related cards submenu
    auto *relatedMenu = new QMenu(tr("Show Related cards"));
    menu.addMenu(relatedMenu);
    auto relatedCards = card->getAllRelatedCards();
    if (relatedCards.isEmpty()) {
        relatedMenu->setDisabled(true);
    } else {
        for (const CardRelation *rel : relatedCards) {
            const QString &relatedCardName = rel->getName();
            QAction *relatedCard = relatedMenu->addAction(relatedCardName);
            connect(relatedCard, &QAction::triggered, this,
                    [this, relatedCardName] { emit relatedCardClicked(relatedCardName); });
        }
    }

    menu.exec(mapToGlobal(point));
}
