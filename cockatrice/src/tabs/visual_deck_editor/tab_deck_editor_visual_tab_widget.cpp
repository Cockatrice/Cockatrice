#include "tab_deck_editor_visual_tab_widget.h"

#include "../../interface/widgets/visual_database_display/visual_database_display_widget.h"
#include "../abstract_tab_deck_editor.h"

TabDeckEditorVisualTabWidget::TabDeckEditorVisualTabWidget(QWidget *parent,
                                                           AbstractTabDeckEditor *_deckEditor,
                                                           DeckListModel *_deckModel,
                                                           CardDatabaseModel *_cardDatabaseModel,
                                                           CardDatabaseDisplayModel *_cardDatabaseDisplayModel)
    : QTabWidget(parent), deckEditor(_deckEditor), deckModel(_deckModel), cardDatabaseModel(_cardDatabaseModel),
      cardDatabaseDisplayModel(_cardDatabaseDisplayModel)
{
    this->setTabsClosable(true); // Enable tab closing
    connect(this, &QTabWidget::tabCloseRequested, this, &TabDeckEditorVisualTabWidget::handleTabClose);

    // Set up the layout and add tab widget
    layout = new QVBoxLayout(this);
    setLayout(layout);

    visualDeckView = new VisualDeckEditorWidget(this, deckModel);
    visualDeckView->setObjectName("visualDeckView");
    connect(visualDeckView, &VisualDeckEditorWidget::activeCardChanged, this,
            &TabDeckEditorVisualTabWidget::onCardChanged);
    connect(visualDeckView, &VisualDeckEditorWidget::cardClicked, this,
            &TabDeckEditorVisualTabWidget::onCardClickedDeckEditor);
    connect(visualDeckView, &VisualDeckEditorWidget::cardAdditionRequested, deckEditor,
            &AbstractTabDeckEditor::actAddCard);

    visualDatabaseDisplay =
        new VisualDatabaseDisplayWidget(this, deckEditor, _cardDatabaseModel, _cardDatabaseDisplayModel);
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

void TabDeckEditorVisualTabWidget::onCardChanged(const ExactCard &activeCard)
{
    emit cardChanged(activeCard);
}

void TabDeckEditorVisualTabWidget::onCardChangedDatabaseDisplay(const ExactCard &activeCard)
{
    emit cardChangedDatabaseDisplay(activeCard);
}

void TabDeckEditorVisualTabWidget::onCardClickedDeckEditor(QMouseEvent *event,
                                                           CardInfoPictureWithTextOverlayWidget *instance,
                                                           QString zoneName)
{
    emit cardClicked(event, instance, zoneName);
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
