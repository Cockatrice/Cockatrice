#include "tab_deck_editor_visual_tab_widget.h"

#include "../../ui/widgets/visual_database_display/visual_database_display_widget.h"
#include "../tab_generic_deck_editor.h"

TabDeckEditorVisualTabWidget::TabDeckEditorVisualTabWidget(QWidget *parent,
                                                           TabGenericDeckEditor *_deckEditor,
                                                           DeckListModel *model,
                                                           CardDatabaseModel *card_database_model,
                                                           CardDatabaseDisplayModel *card_database_display_model)
    : QTabWidget(parent), deckEditor(_deckEditor), deckModel(model), cardDatabaseModel(card_database_model),
      cardDatabaseDisplayModel(card_database_display_model)
{
    this->setTabsClosable(true); // Enable tab closing
    connect(this, &QTabWidget::tabCloseRequested, this, &TabDeckEditorVisualTabWidget::handleTabClose);

    // Set up the layout and add tab widget
    layout = new QVBoxLayout(this);
    setLayout(layout);

    visualDeckView = new VisualDeckEditorWidget(this, deckModel);
    visualDeckView->setObjectName("visualDeckView");
    visualDeckView->updateDisplay();
    connect(visualDeckView, SIGNAL(activeCardChanged(CardInfoPtr)), this, SLOT(onCardChanged(CardInfoPtr)));
    connect(visualDeckView, SIGNAL(mainboardCardClicked(QMouseEvent *, CardInfoPictureWithTextOverlayWidget *)), this,
            SLOT(onMainboardCardClickedDeckEditor(QMouseEvent *, CardInfoPictureWithTextOverlayWidget *)));
    connect(visualDeckView, SIGNAL(sideboardCardClicked(QMouseEvent *, CardInfoPictureWithTextOverlayWidget *)), this,
            SLOT(onSideboardCardClickedDeckEditor(QMouseEvent *, CardInfoPictureWithTextOverlayWidget *)));

    visualDatabaseDisplay = new VisualDatabaseDisplayWidget(this, cardDatabaseModel, cardDatabaseDisplayModel);
    visualDatabaseDisplay->setObjectName("visualDatabaseView");
    connect(visualDatabaseDisplay, SIGNAL(cardHoveredDatabaseDisplay(CardInfoPtr)), this,
            SLOT(onCardChangedDatabaseDisplay(CardInfoPtr)));
    connect(visualDatabaseDisplay,
            SIGNAL(cardClickedDatabaseDisplay(QMouseEvent *, CardInfoPictureWithTextOverlayWidget *)), this,
            SLOT(onCardClickedDatabaseDisplay(QMouseEvent *, CardInfoPictureWithTextOverlayWidget *)));

    deckAnalytics = new DeckAnalyticsWidget(this, deckModel);
    deckAnalytics->setObjectName("deckAnalytics");

    sampleHandWidget = new VisualDeckEditorSampleHandWidget(this, deckModel);

    // printingSelector = new PrintingSelector(this, this->deckModel);

    this->addNewTab(visualDeckView, "Visual Deck View");
    this->addNewTab(visualDatabaseDisplay, "Visual Database Display");
    this->addNewTab(deckAnalytics, "Deck Analytics");
    this->addNewTab(sampleHandWidget, "Sample Hand");
}

void TabDeckEditorVisualTabWidget::onCardChanged(CardInfoPtr activeCard)
{
    emit cardChanged(activeCard);
}

void TabDeckEditorVisualTabWidget::onCardChangedDatabaseDisplay(CardInfoPtr activeCard)
{
    emit cardChangedDatabaseDisplay(activeCard);
}

void TabDeckEditorVisualTabWidget::onMainboardCardClickedDeckEditor(QMouseEvent *event,
                                                                    CardInfoPictureWithTextOverlayWidget *instance)
{
    emit mainboardCardClicked(event, instance);
}

void TabDeckEditorVisualTabWidget::onSideboardCardClickedDeckEditor(QMouseEvent *event,
                                                                    CardInfoPictureWithTextOverlayWidget *instance)
{
    emit sideboardCardClicked(event, instance);
}

void TabDeckEditorVisualTabWidget::onCardClickedDatabaseDisplay(QMouseEvent *event,
                                                                CardInfoPictureWithTextOverlayWidget *instance)
{
    emit cardClickedDatabaseDisplay(event, instance);
}

void TabDeckEditorVisualTabWidget::addNewTab(QWidget *widget, const QString &title)
{
    // Add new tab to the tab widget
    this->addTab(widget, title);
}

void TabDeckEditorVisualTabWidget::removeCurrentTab()
{
    // Remove the currently selected tab
    int currentIndex = this->currentIndex();
    if (currentIndex != -1) {
        this->removeTab(currentIndex);
    }
}

void TabDeckEditorVisualTabWidget::setTabTitle(int index, const QString &title)
{
    // Set the title of the tab at the given index
    if (index >= 0 && index < this->count()) {
        this->setTabText(index, title);
    }
}

QWidget *TabDeckEditorVisualTabWidget::getCurrentTab() const
{
    // Return the currently selected tab widget
    return this->currentWidget();
}

int TabDeckEditorVisualTabWidget::getTabCount() const
{
    // Return the number of tabs
    return this->count();
}

void TabDeckEditorVisualTabWidget::handleTabClose(int index)
{
    // Handle closing of the tab at the given index
    QWidget *tab = this->widget(index);
    this->removeTab(index);
    delete tab; // Delete the tab's widget to free memory
}
