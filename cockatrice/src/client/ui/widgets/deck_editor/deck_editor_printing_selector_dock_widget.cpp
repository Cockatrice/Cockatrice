#include "deck_editor_printing_selector_dock_widget.h"

#include "../../../tabs/abstract_tab_deck_editor.h"

#include <QVBoxLayout>

DeckEditorPrintingSelectorDockWidget::DeckEditorPrintingSelectorDockWidget(QWidget *parent,
                                                                           AbstractTabDeckEditor *_deckEditor)
    : QDockWidget(parent), deckEditor(_deckEditor)
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
}

void DeckEditorPrintingSelectorDockWidget::retranslateUi()
{
    setWindowTitle(tr("Printing Selector"));
}
