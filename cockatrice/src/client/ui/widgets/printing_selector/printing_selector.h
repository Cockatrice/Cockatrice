#ifndef PRINTING_SELECTOR_H
#define PRINTING_SELECTOR_H

#include "../../../../deck/deck_list_model.h"
#include "../../../../deck/deck_view.h"
#include "../../../../game/cards/card_database.h"
#include "../cards/card_size_widget.h"
#include "../general/layout_containers/flow_widget.h"

#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QTreeView>
#include <QVBoxLayout>
#include <QWidget>

#define BATCH_SIZE 10

class PrintingSelectorCardSelectionWidget;
class TabDeckEditor;
class PrintingSelector : public QWidget
{
    Q_OBJECT

public:
    PrintingSelector(QWidget *parent, TabDeckEditor *deckEditor, DeckListModel *deckModel, QTreeView *deckView);
    void setCard(const CardInfoPtr &newCard, const QString &_currentZone);
    CardInfoPerSet getSetForUUID(const QString &uuid);
    QList<CardInfoPerSet> prependPrintingsInDeck(const QList<CardInfoPerSet> &sets);
    QList<CardInfoPerSet> sortSets();
    QList<CardInfoPerSet> filterSets(const QList<CardInfoPerSet> &sets) const;
    void getAllSetsForCurrentCard();

public slots:
    void updateDisplay();
    void selectPreviousCard();
    void selectNextCard();
    void updateSortOrder();

private:
    QVBoxLayout *layout;
    QHBoxLayout *sortToolBar;
    static const QString SORT_OPTIONS_ALPHABETICAL;
    static const QString SORT_OPTIONS_PREFERENCE;
    static const QString SORT_OPTIONS_RELEASE_DATE;
    static const QString SORT_OPTIONS_CONTAINED_IN_DECK;
    static const QString SORT_OPTIONS_POTENTIAL_CARDS;
    static const QStringList SORT_OPTIONS;
    QComboBox *sortOptionsSelector;
    bool descendingSort;
    QPushButton *toggleSortOrder;
    QLineEdit *searchBar;
    QTimer *searchDebounceTimer;
    FlowWidget *flowWidget;
    CardSizeWidget *cardSizeWidget;
    PrintingSelectorCardSelectionWidget *cardSelectionBar;
    TabDeckEditor *deckEditor;
    DeckListModel *deckModel;
    QTreeView *deckView;
    CardInfoPtr selectedCard;
    QString currentZone;
    QTimer *timer;
    int currentIndex = 0;
    void selectCard(int changeBy);
};

#endif // PRINTING_SELECTOR_H
