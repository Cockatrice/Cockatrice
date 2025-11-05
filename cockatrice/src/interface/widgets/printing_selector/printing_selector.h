/**
 * @file printing_selector.h
 * @ingroup PrintingWidgets
 * @brief TODO: Document this.
 */

#ifndef PRINTING_SELECTOR_H
#define PRINTING_SELECTOR_H

#include "../cards/card_size_widget.h"
#include "../general/layout_containers/flow_widget.h"
#include "../quick_settings/settings_button_widget.h"

#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QTreeView>
#include <QVBoxLayout>
#include <QWidget>
#include <libcockatrice/card/card_info.h>
#include <libcockatrice/models/deck_list/deck_list_model.h>

#define BATCH_SIZE 10

class PrintingSelectorCardSearchWidget;
class PrintingSelectorCardSelectionWidget;
class PrintingSelectorCardSortingWidget;
class PrintingSelectorViewOptionsWidget;
class AbstractTabDeckEditor;
class PrintingSelector : public QWidget
{
    Q_OBJECT

public:
    PrintingSelector(QWidget *parent, AbstractTabDeckEditor *deckEditor);

    void setCard(const CardInfoPtr &newCard, const QString &_currentZone);
    void getAllSetsForCurrentCard();
    DeckListModel *getDeckModel() const
    {
        return deckModel;
    };

public slots:
    void retranslateUi();
    void updateDisplay();
    void selectPreviousCard();
    void selectNextCard();
    void toggleVisibilityNavigationButtons(bool _state);

private slots:
    void printingsInDeckChanged();

private:
    QVBoxLayout *layout;
    SettingsButtonWidget *displayOptionsWidget;
    QWidget *sortAndOptionsContainer;
    QHBoxLayout *sortAndOptionsLayout;
    QCheckBox *navigationCheckBox;
    PrintingSelectorCardSortingWidget *sortToolBar;
    PrintingSelectorCardSearchWidget *searchBar;
    FlowWidget *flowWidget;
    CardSizeWidget *cardSizeWidget;
    PrintingSelectorCardSelectionWidget *cardSelectionBar;
    AbstractTabDeckEditor *deckEditor;
    DeckListModel *deckModel;
    QTreeView *deckView;
    CardInfoPtr selectedCard;
    QString currentZone;
    QTimer *widgetLoadingBufferTimer;
    int currentIndex = 0;
    void selectCard(int changeBy);
};

#endif // PRINTING_SELECTOR_H
