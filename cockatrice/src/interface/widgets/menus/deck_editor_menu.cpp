#include "../../../interface/widgets/menus/deck_editor_menu.h"

#include "../../../client/settings/cache_settings.h"
#include "../../../client/settings/shortcuts_settings.h"
#include "../tabs/abstract_tab_deck_editor.h"

DeckEditorMenu::DeckEditorMenu(AbstractTabDeckEditor *parent) : QMenu(parent), deckEditor(parent)
{
    aNewDeck = new QAction(QString(), this);
    connect(aNewDeck, &QAction::triggered, deckEditor, &AbstractTabDeckEditor::actNewDeck);

    aLoadDeck = new QAction(QString(), this);
    connect(aLoadDeck, &QAction::triggered, deckEditor, &AbstractTabDeckEditor::actLoadDeck);

    loadRecentDeckMenu = new QMenu(this);
    connect(&SettingsCache::instance().recents(), &RecentsSettings::recentlyOpenedDeckPathsChanged, this,
            &DeckEditorMenu::updateRecentlyOpened);

    aClearRecents = new QAction(QString(), this);
    connect(aClearRecents, &QAction::triggered, this, &DeckEditorMenu::actClearRecents);

    updateRecentlyOpened();

    aSaveDeck = new QAction(QString(), this);
    connect(aSaveDeck, &QAction::triggered, deckEditor, &AbstractTabDeckEditor::actSaveDeck);

    aSaveDeckAs = new QAction(QString(), this);
    connect(aSaveDeckAs, &QAction::triggered, deckEditor, &AbstractTabDeckEditor::actSaveDeckAs);

    aLoadDeckFromClipboard = new QAction(QString(), this);
    connect(aLoadDeckFromClipboard, &QAction::triggered, deckEditor, &AbstractTabDeckEditor::actLoadDeckFromClipboard);

    aEditDeckInClipboard = new QAction(QString(), this);
    connect(aEditDeckInClipboard, &QAction::triggered, deckEditor, &AbstractTabDeckEditor::actEditDeckInClipboard);

    aEditDeckInClipboardRaw = new QAction(QString(), this);
    connect(aEditDeckInClipboardRaw, &QAction::triggered, deckEditor,
            &AbstractTabDeckEditor::actEditDeckInClipboardRaw);

    aSaveDeckToClipboard = new QAction(QString(), this);
    connect(aSaveDeckToClipboard, &QAction::triggered, deckEditor, &AbstractTabDeckEditor::actSaveDeckToClipboard);

    aSaveDeckToClipboardNoSetInfo = new QAction(QString(), this);
    connect(aSaveDeckToClipboardNoSetInfo, &QAction::triggered, deckEditor,
            &AbstractTabDeckEditor::actSaveDeckToClipboardNoSetInfo);

    aSaveDeckToClipboardRaw = new QAction(QString(), this);
    connect(aSaveDeckToClipboardRaw, &QAction::triggered, deckEditor,
            &AbstractTabDeckEditor::actSaveDeckToClipboardRaw);

    aSaveDeckToClipboardRawNoSetInfo = new QAction(QString(), this);
    connect(aSaveDeckToClipboardRawNoSetInfo, &QAction::triggered, deckEditor,
            &AbstractTabDeckEditor::actSaveDeckToClipboardRawNoSetInfo);

    aPrintDeck = new QAction(QString(), this);
    connect(aPrintDeck, &QAction::triggered, deckEditor, &AbstractTabDeckEditor::actPrintDeck);

    aLoadDeckFromWebsite = new QAction(QString(), this);
    connect(aLoadDeckFromWebsite, &QAction::triggered, deckEditor, &AbstractTabDeckEditor::actLoadDeckFromWebsite);

    aExportDeckDecklist = new QAction(QString(), this);
    connect(aExportDeckDecklist, &QAction::triggered, deckEditor, &AbstractTabDeckEditor::actExportDeckDecklist);

    aExportDeckDecklistXyz = new QAction(QString(), this);
    connect(aExportDeckDecklistXyz, &QAction::triggered, deckEditor, &AbstractTabDeckEditor::actExportDeckDecklistXyz);

    aAnalyzeDeckDeckstats = new QAction(QString(), this);
    connect(aAnalyzeDeckDeckstats, &QAction::triggered, deckEditor, &AbstractTabDeckEditor::actAnalyzeDeckDeckstats);

    aAnalyzeDeckTappedout = new QAction(QString(), this);
    connect(aAnalyzeDeckTappedout, &QAction::triggered, deckEditor, &AbstractTabDeckEditor::actAnalyzeDeckTappedout);

    analyzeDeckMenu = new QMenu(this);
    analyzeDeckMenu->addAction(aExportDeckDecklist);
    analyzeDeckMenu->addAction(aExportDeckDecklistXyz);
    analyzeDeckMenu->addSeparator();
    analyzeDeckMenu->addAction(aAnalyzeDeckDeckstats);
    analyzeDeckMenu->addAction(aAnalyzeDeckTappedout);

    aClose = new QAction(QString(), this);
    connect(aClose, &QAction::triggered, deckEditor, &AbstractTabDeckEditor::closeRequest);

    editDeckInClipboardMenu = new QMenu(this);
    editDeckInClipboardMenu->addAction(aEditDeckInClipboard);
    editDeckInClipboardMenu->addAction(aEditDeckInClipboardRaw);

    saveDeckToClipboardMenu = new QMenu(this);
    saveDeckToClipboardMenu->addAction(aSaveDeckToClipboard);
    saveDeckToClipboardMenu->addAction(aSaveDeckToClipboardNoSetInfo);
    saveDeckToClipboardMenu->addAction(aSaveDeckToClipboardRaw);
    saveDeckToClipboardMenu->addAction(aSaveDeckToClipboardRawNoSetInfo);

    addAction(aNewDeck);
    addAction(aLoadDeck);
    addMenu(loadRecentDeckMenu);
    addAction(aSaveDeck);
    addAction(aSaveDeckAs);
    addSeparator();
    addAction(aLoadDeckFromClipboard);
    addMenu(editDeckInClipboardMenu);
    addMenu(saveDeckToClipboardMenu);
    addSeparator();
    addAction(aPrintDeck);
    addAction(aLoadDeckFromWebsite);
    addMenu(analyzeDeckMenu);
    addSeparator();
    addAction(deckEditor->filterDockWidget->aClearFilterOne);
    addAction(deckEditor->filterDockWidget->aClearFilterAll);
    addSeparator();
    addAction(aClose);

    retranslateUi();
    connect(&SettingsCache::instance().shortcuts(), &ShortcutsSettings::shortCutChanged, this,
            &DeckEditorMenu::refreshShortcuts);
    refreshShortcuts();
}

void DeckEditorMenu::setSaveStatus(bool newStatus)
{
    aSaveDeck->setEnabled(newStatus);
    aSaveDeckAs->setEnabled(newStatus);
    aSaveDeckToClipboard->setEnabled(newStatus);
    aSaveDeckToClipboardNoSetInfo->setEnabled(newStatus);
    aSaveDeckToClipboardRaw->setEnabled(newStatus);
    aSaveDeckToClipboardRawNoSetInfo->setEnabled(newStatus);
    saveDeckToClipboardMenu->setEnabled(newStatus);
    aPrintDeck->setEnabled(newStatus);
    analyzeDeckMenu->setEnabled(newStatus);
}

void DeckEditorMenu::updateRecentlyOpened()
{
    loadRecentDeckMenu->clear();
    for (const auto &deckPath : SettingsCache::instance().recents().getRecentlyOpenedDeckPaths()) {
        QAction *aRecentlyOpenedDeck = new QAction(deckPath, this);
        loadRecentDeckMenu->addAction(aRecentlyOpenedDeck);
        connect(aRecentlyOpenedDeck, &QAction::triggered, deckEditor,
                [=, this] { deckEditor->actOpenRecent(aRecentlyOpenedDeck->text()); });
    }
    loadRecentDeckMenu->addSeparator();
    loadRecentDeckMenu->addAction(aClearRecents);
    aClearRecents->setEnabled(SettingsCache::instance().recents().getRecentlyOpenedDeckPaths().length() > 0);
}

void DeckEditorMenu::actClearRecents()
{
    SettingsCache::instance().recents().clearRecentlyOpenedDeckPaths();
}

void DeckEditorMenu::retranslateUi()
{
    setTitle(tr("&Deck Editor"));
    aNewDeck->setText(tr("&New deck"));
    aLoadDeck->setText(tr("&Load deck..."));
    loadRecentDeckMenu->setTitle(tr("Load recent deck..."));
    aClearRecents->setText(tr("Clear"));
    aSaveDeck->setText(tr("&Save deck"));
    aSaveDeckAs->setText(tr("Save deck &as..."));

    aLoadDeckFromClipboard->setText(tr("Load deck from cl&ipboard..."));

    editDeckInClipboardMenu->setTitle(tr("Edit deck in clipboard"));
    aEditDeckInClipboard->setText(tr("Annotated"));
    aEditDeckInClipboardRaw->setText(tr("Not Annotated"));

    saveDeckToClipboardMenu->setTitle(tr("Save deck to clipboard"));
    aSaveDeckToClipboard->setText(tr("Annotated"));
    aSaveDeckToClipboardNoSetInfo->setText(tr("Annotated (No set info)"));
    aSaveDeckToClipboardRaw->setText(tr("Not Annotated"));
    aSaveDeckToClipboardRawNoSetInfo->setText(tr("Not Annotated (No set info)"));

    aPrintDeck->setText(tr("&Print deck..."));

    aLoadDeckFromWebsite->setText(tr("Load deck from online service..."));
    analyzeDeckMenu->setTitle(tr("&Send deck to online service"));
    aExportDeckDecklist->setText(tr("Create decklist (decklist.org)"));
    aExportDeckDecklistXyz->setText(tr("Create decklist (decklist.xyz)"));
    aAnalyzeDeckDeckstats->setText(tr("Analyze deck (deckstats.net)"));
    aAnalyzeDeckTappedout->setText(tr("Analyze deck (tappedout.net)"));

    aClose->setText(tr("&Close"));
}

void DeckEditorMenu::refreshShortcuts()
{
    ShortcutsSettings &shortcuts = SettingsCache::instance().shortcuts();
    aNewDeck->setShortcuts(shortcuts.getShortcut("TabDeckEditor/aNewDeck"));
    aLoadDeck->setShortcuts(shortcuts.getShortcut("TabDeckEditor/aLoadDeck"));
    aSaveDeck->setShortcuts(shortcuts.getShortcut("TabDeckEditor/aSaveDeck"));
    aSaveDeckAs->setShortcuts(shortcuts.getShortcut("TabDeckEditor/aSaveDeckAs"));
    aLoadDeckFromClipboard->setShortcuts(shortcuts.getShortcut("TabDeckEditor/aLoadDeckFromClipboard"));
    aEditDeckInClipboard->setShortcuts(shortcuts.getShortcut("TabDeckEditor/aEditDeckInClipboard"));
    aEditDeckInClipboardRaw->setShortcuts(shortcuts.getShortcut("TabDeckEditor/aEditDeckInClipboardRaw"));
    aPrintDeck->setShortcuts(shortcuts.getShortcut("TabDeckEditor/aPrintDeck"));

    aExportDeckDecklist->setShortcuts(shortcuts.getShortcut("TabDeckEditor/aExportDeckDecklist"));
    aExportDeckDecklistXyz->setShortcuts(shortcuts.getShortcut("TabDeckEditor/aExportDeckDecklistXyz"));
    aAnalyzeDeckDeckstats->setShortcuts(shortcuts.getShortcut("TabDeckEditor/aAnalyzeDeck"));
    aAnalyzeDeckTappedout->setShortcuts(shortcuts.getShortcut("TabDeckEditor/aAnalyzeDeckTappedout"));

    aClose->setShortcuts(shortcuts.getShortcut("TabDeckEditor/aClose"));

    aSaveDeckToClipboard->setShortcuts(shortcuts.getShortcut("TabDeckEditor/aSaveDeckToClipboard"));
    aSaveDeckToClipboardNoSetInfo->setShortcuts(shortcuts.getShortcut("TabDeckEditor/aSaveDeckToClipboardNoSetInfo"));
    aSaveDeckToClipboardRaw->setShortcuts(shortcuts.getShortcut("TabDeckEditor/aSaveDeckToClipboardRaw"));
    aSaveDeckToClipboardRawNoSetInfo->setShortcuts(
        shortcuts.getShortcut("TabDeckEditor/aSaveDeckToClipboardRawNoSetInfo"));

    aClose->setShortcuts(shortcuts.getShortcut("TabDeckEditor/aClose"));
}
