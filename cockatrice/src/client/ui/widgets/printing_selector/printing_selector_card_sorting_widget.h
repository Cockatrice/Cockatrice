#ifndef PRINTING_SELECTOR_CARD_SORTING_WIDGET_H
#define PRINTING_SELECTOR_CARD_SORTING_WIDGET_H

#include "printing_selector.h"

#include <QWidget>

class PrintingSelectorCardSortingWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PrintingSelectorCardSortingWidget(PrintingSelector *parent);
    QList<CardInfoPerSet> sortSets(CardInfoPerSetMap cardInfoPerSets);
    QList<CardInfoPerSet> filterSets(const QList<CardInfoPerSet> &sets, QString searchText) const;
    QList<CardInfoPerSet>
    prependPrintingsInDeck(const QList<CardInfoPerSet> &sets, CardInfoPtr selectedCard, DeckListModel *deckModel);

public slots:
    void updateSortOrder();

private:
    PrintingSelector *parent;
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
};

#endif // PRINTING_SELECTOR_CARD_SORTING_WIDGET_H
