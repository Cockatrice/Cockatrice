#include "deck_editor_printing_selector_dock_widget.h"

#include "../../../client/settings/cache_settings.h"
#include "../../../interface/widgets/tabs/abstract_tab_deck_editor.h"
#include "printing_disabled_info_widget.h"

#include <QVBoxLayout>

DeckEditorPrintingSelectorDockWidget::DeckEditorPrintingSelectorDockWidget(AbstractTabDeckEditor *parent)
    : QDockWidget(parent), deckEditor(parent)
{
    setObjectName("printingSelectorDock");

    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
    setFloating(false);

    createPrintingSelectorDock();
    printingDisabledInfoWidget = new PrintingDisabledInfoWidget(this);

    setVisibleWidget(SettingsCache::instance().getOverrideAllCardArtWithPersonalPreference());
    connect(&SettingsCache::instance(), &SettingsCache::overrideAllCardArtWithPersonalPreferenceChanged, this,
            &DeckEditorPrintingSelectorDockWidget::setVisibleWidget);

    retranslateUi();
}

void DeckEditorPrintingSelectorDockWidget::createPrintingSelectorDock()
{
    printingSelector = new PrintingSelector(this, deckEditor);
    printingSelector->setObjectName("printingSelector");
    auto *printingSelectorFrame = new QVBoxLayout;
    printingSelectorFrame->setObjectName("printingSelectorFrame");
    printingSelectorFrame->addWidget(printingSelector);

    printingSelectorDockContents = new QWidget();
    printingSelectorDockContents->setObjectName("printingSelectorDockContents");
    printingSelectorDockContents->setLayout(printingSelectorFrame);

    installEventFilter(deckEditor);
    connect(printingSelector, &PrintingSelector::prevCardRequested, deckEditor->getDeckDockWidget(),
            &DeckEditorDeckDockWidget::selectPrevCard);
    connect(printingSelector, &PrintingSelector::nextCardRequested, deckEditor->getDeckDockWidget(),
            &DeckEditorDeckDockWidget::selectNextCard);
}

void DeckEditorPrintingSelectorDockWidget::retranslateUi()
{
    setWindowTitle(tr("Printing Selector"));
}

void DeckEditorPrintingSelectorDockWidget::setVisibleWidget(bool overridePrintings)
{
    if (overridePrintings) {
        setWidget(printingDisabledInfoWidget);
    } else {
        setWidget(printingSelectorDockContents);
        printingSelector->updateDisplay();
    }
}
