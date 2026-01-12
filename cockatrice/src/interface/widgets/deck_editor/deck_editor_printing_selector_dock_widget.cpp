#include "deck_editor_printing_selector_dock_widget.h"

#include "../../../interface/widgets/tabs/abstract_tab_deck_editor.h"

#include <QVBoxLayout>

DeckEditorPrintingSelectorDockWidget::DeckEditorPrintingSelectorDockWidget(AbstractTabDeckEditor *parent)
    : QDockWidget(parent), deckEditor(parent)
{
    setObjectName("printingSelectorDock");

    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
    setFloating(false);

    createPrintingSelectorDock();

    retranslateUi();
}

void DeckEditorPrintingSelectorDockWidget::createPrintingSelectorDock()
{
    printingSelector = new PrintingSelector(this, deckEditor);
    printingSelector->setObjectName("printingSelector");
    auto *printingSelectorFrame = new QVBoxLayout;
    printingSelectorFrame->setObjectName("printingSelectorFrame");
    printingSelectorFrame->addWidget(printingSelector);

    auto *printingSelectorDockContents = new QWidget();
    printingSelectorDockContents->setObjectName("printingSelectorDockContents");
    printingSelectorDockContents->setLayout(printingSelectorFrame);
    setWidget(printingSelectorDockContents);

    installEventFilter(deckEditor);
    connect(this, &QDockWidget::topLevelChanged, deckEditor, &AbstractTabDeckEditor::dockTopLevelChanged);
    connect(printingSelector, &PrintingSelector::prevCardRequested, deckEditor->getDeckDockWidget(),
            &DeckEditorDeckDockWidget::selectPrevCard);
    connect(printingSelector, &PrintingSelector::nextCardRequested, deckEditor->getDeckDockWidget(),
            &DeckEditorDeckDockWidget::selectNextCard);
}

void DeckEditorPrintingSelectorDockWidget::retranslateUi()
{
    setWindowTitle(tr("Printing Selector"));
}
