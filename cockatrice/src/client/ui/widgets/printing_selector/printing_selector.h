#ifndef PRINTING_SELECTOR_H
#define PRINTING_SELECTOR_H

#include "../../../../deck/deck_list_model.h"
#include "../../../../deck/deck_view.h"
#include "../../../../game/cards/card_database.h"
#include "../cards/card_size_widget.h"
#include "../general/layout_containers/flow_widget.h"

#include <QLabel>
#include <QTreeView>
#include <QVBoxLayout>
#include <QWidget>

#define BATCH_SIZE 10

class PrintingSelectorCardSearchWidget;
class PrintingSelectorCardSelectionWidget;
class PrintingSelectorCardSortingWidget;
class TabDeckEditor;
class PrintingSelector : public QWidget
{
    Q_OBJECT

public:
    PrintingSelector(QWidget *parent, TabDeckEditor *deckEditor, DeckListModel *deckModel, QTreeView *deckView);
    void setCard(const CardInfoPtr &newCard, const QString &_currentZone);
    CardInfoPerSet getSetForUUID(const QString &uuid);
    void getAllSetsForCurrentCard();

public slots:
    void updateDisplay();
    void selectPreviousCard();
    void selectNextCard();

private:
    QVBoxLayout *layout;
    PrintingSelectorCardSortingWidget *sortToolBar;
    PrintingSelectorCardSearchWidget *searchBar;
    FlowWidget *flowWidget;
    CardSizeWidget *cardSizeWidget;
    PrintingSelectorCardSelectionWidget *cardSelectionBar;
    TabDeckEditor *deckEditor;
    DeckListModel *deckModel;
    QTreeView *deckView;
    CardInfoPtr selectedCard;
    QString currentZone;
    QTimer *widgetLoadingBufferTimer;
    int currentIndex = 0;
    void selectCard(int changeBy);
};

#endif // PRINTING_SELECTOR_H
