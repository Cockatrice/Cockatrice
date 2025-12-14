#include "tab_deck_editor_visual_tab_widget.h"

#include "../../interface/widgets/visual_database_display/visual_database_display_widget.h"
#include "../abstract_tab_deck_editor.h"

/**
 * @brief Constructs the TabDeckEditorVisualTabWidget.
 * @param parent The parent QWidget.
 * @param _deckEditor Pointer to the associated deck editor.
 * @param _deckModel Pointer to the deck list model.
 * @param _cardDatabaseModel Pointer to the card database model.
 * @param _cardDatabaseDisplayModel Pointer to the card database display model.
 *
 * Initializes all sub-widgets (visual deck view, database display, deck analytics,
 * sample hand) and sets up the tab layout and signal connections.
 */
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

    layout = new QVBoxLayout(this);
    setLayout(layout);

    visualDeckView = new VisualDeckEditorWidget(this, deckModel, _deckEditor->deckDockWidget->getSelectionModel());
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

/**
 * @brief Emits the cardChanged signal when a card is activated in the visual deck view.
 * @param activeCard The card that was activated.
 */
void TabDeckEditorVisualTabWidget::onCardChanged(const ExactCard &activeCard)
{
    emit cardChanged(activeCard);
}

/**
 * @brief Emits the cardChangedDatabaseDisplay signal when a card is hovered in the database display.
 * @param activeCard The card that was hovered.
 */
void TabDeckEditorVisualTabWidget::onCardChangedDatabaseDisplay(const ExactCard &activeCard)
{
    emit cardChangedDatabaseDisplay(activeCard);
}

/**
 * @brief Emits the cardClicked signal when a card is clicked in the visual deck view.
 * @param event The mouse event.
 * @param instance The widget instance of the clicked card.
 * @param zoneName The zone of the deck where the card is located.
 */
void TabDeckEditorVisualTabWidget::onCardClickedDeckEditor(QMouseEvent *event,
                                                           CardInfoPictureWithTextOverlayWidget *instance,
                                                           QString zoneName)
{
    emit cardClicked(event, instance, zoneName);
}

/**
 * @brief Emits the cardClickedDatabaseDisplay signal when a card is clicked in the database display.
 * @param event The mouse event.
 * @param instance The widget instance of the clicked card.
 */
void TabDeckEditorVisualTabWidget::onCardClickedDatabaseDisplay(QMouseEvent *event,
                                                                CardInfoPictureWithTextOverlayWidget *instance)
{
    emit cardClickedDatabaseDisplay(event, instance);
}

/**
 * @brief Adds a new tab with the given widget and title.
 * @param widget The widget to add.
 * @param title The title of the tab.
 */
void TabDeckEditorVisualTabWidget::addNewTab(QWidget *widget, const QString &title)
{
    this->addTab(widget, title);
}

/**
 * @brief Removes the currently selected tab.
 */
void TabDeckEditorVisualTabWidget::removeCurrentTab()
{
    int currentIndex = this->currentIndex();
    if (currentIndex != -1) {
        this->removeTab(currentIndex);
    }
}

/**
 * @brief Sets the title of a tab at a given index.
 * @param index The index of the tab.
 * @param title The new title.
 */
void TabDeckEditorVisualTabWidget::setTabTitle(int index, const QString &title)
{
    if (index >= 0 && index < this->count()) {
        this->setTabText(index, title);
    }
}

/**
 * @brief Returns the currently selected tab widget.
 * @return Pointer to the current tab widget.
 */
QWidget *TabDeckEditorVisualTabWidget::getCurrentTab() const
{
    return this->currentWidget();
}

/**
 * @brief Returns the number of tabs in the tab widget.
 * @return Number of tabs.
 */
int TabDeckEditorVisualTabWidget::getTabCount() const
{
    return this->count();
}

/**
 * @brief Handles the closing of a tab.
 * @param index The index of the tab to close.
 *
 * Removes the tab and deletes the widget to free memory.
 */
void TabDeckEditorVisualTabWidget::handleTabClose(int index)
{
    QWidget *tab = this->widget(index);
    this->removeTab(index);
    delete tab;
}
