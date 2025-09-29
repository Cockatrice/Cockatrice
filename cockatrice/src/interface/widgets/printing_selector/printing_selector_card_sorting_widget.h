/**
 * @file printing_selector_card_sorting_widget.h
 * @ingroup PrintingWidgets
 * @brief TODO: Document this.
 */

#ifndef PRINTING_SELECTOR_CARD_SORTING_WIDGET_H
#define PRINTING_SELECTOR_CARD_SORTING_WIDGET_H

#include "printing_selector.h"

#include <QComboBox>
#include <QPushButton>
#include <QWidget>

class PrintingSelectorCardSortingWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PrintingSelectorCardSortingWidget(PrintingSelector *parent);
    QList<PrintingInfo> sortSets(const SetToPrintingsMap &setMap);
    QList<PrintingInfo> filterSets(const QList<PrintingInfo> &printings, const QString &searchText);
    QList<PrintingInfo> prependPinnedPrintings(const QList<PrintingInfo> &printings, const QString &cardName);
    QList<PrintingInfo> prependPrintingsInDeck(const QList<PrintingInfo> &printings,
                                               const CardInfoPtr &selectedCard,
                                               DeckListModel *deckModel);

public slots:
    void updateSortOrder();
    void updateSortSetting();

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
