#include "tab_deck_editor_visual_tab_widget.h"

#include "../../ui/widgets/visual_database_display/visual_database_display_widget.h"
#include "../abstract_tab_deck_editor.h"

TabDeckEditorVisualTabWidget::TabDeckEditorVisualTabWidget(QWidget *parent,
                                                           AbstractTabDeckEditor *_deckEditor,
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
    connect(visualDeckView, &VisualDeckEditorWidget::activeCardChanged, this,
            &TabDeckEditorVisualTabWidget::onCardChanged);
    connect(visualDeckView, &VisualDeckEditorWidget::mainboardCardClicked, this,
            &TabDeckEditorVisualTabWidget::onMainboardCardClickedDeckEditor);
    connect(visualDeckView, &VisualDeckEditorWidget::sideboardCardClicked, this,
            &TabDeckEditorVisualTabWidget::onSideboardCardClickedDeckEditor);

    visualDatabaseDisplay =
        new VisualDatabaseDisplayWidget(this, deckEditor, cardDatabaseModel, cardDatabaseDisplayModel);
    visualDatabaseDisplay->setObjectName("visualDatabaseView");
    connect(visualDatabaseDisplay, &VisualDatabaseDisplayWidget::cardHoveredDatabaseDisplay, this,
            &TabDeckEditorVisualTabWidget::onCardChangedDatabaseDisplay);
    connect(visualDatabaseDisplay, &VisualDatabaseDisplayWidget::cardClickedDatabaseDisplay, this,
            &TabDeckEditorVisualTabWidget::onCardClickedDatabaseDisplay);

    deckAnalytics = new DeckAnalyticsWidget(this, deckModel);
    deckAnalytics->setObjectName("deckAnalytics");

    sampleHandWidget = new VisualDeckEditorSampleHandWidget(this, deckModel);

    this->addNewTab(visualDeckView, tr("Visual Deck View"));
    this->addNewTab(visualDatabaseDisplay, tr("Visual Database Display"));
    this->addNewTab(deckAnalytics, tr("Deck Analytics"));
    this->addNewTab(sampleHandWidget, tr("Sample Hand"));
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
