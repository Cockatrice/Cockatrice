#include "deck_editor_printing_selector_dock_widget.h"

#include "../../../tabs/tab_generic_deck_editor.h"

#include <QVBoxLayout>

DeckEditorPrintingSelectorDockWidget::DeckEditorPrintingSelectorDockWidget(QWidget *parent,
                                                                           TabGenericDeckEditor *_deckEditor)
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

    installEventFilter(this);
    connect(this, &QDockWidget::topLevelChanged, deckEditor, &TabGenericDeckEditor::dockTopLevelChanged);
}

void DeckEditorPrintingSelectorDockWidget::retranslateUi()
{
    setWindowTitle(tr("Printing Selector"));
}
