#include "deck_editor_menu.h"

#include "../../../settings/cache_settings.h"
#include "../../../settings/shortcuts_settings.h"

DeckEditorMenu::DeckEditorMenu(AbstractTabDeckEditor *parent) : QMenu(parent), deckEditor(parent)
{
    aNewDeck = new QAction(QString(), this);
    connect(aNewDeck, SIGNAL(triggered()), deckEditor, SLOT(actNewDeck()));

    aLoadDeck = new QAction(QString(), this);
    connect(aLoadDeck, SIGNAL(triggered()), deckEditor, SLOT(actLoadDeck()));

    loadRecentDeckMenu = new QMenu(this);
    connect(&SettingsCache::instance().recents(), &RecentsSettings::recentlyOpenedDeckPathsChanged, this,
            &DeckEditorMenu::updateRecentlyOpened);

    aClearRecents = new QAction(QString(), this);
    connect(aClearRecents, &QAction::triggered, this, &DeckEditorMenu::actClearRecents);

    updateRecentlyOpened();

    aSaveDeck = new QAction(QString(), this);
    connect(aSaveDeck, SIGNAL(triggered()), deckEditor, SLOT(actSaveDeck()));

    aSaveDeckAs = new QAction(QString(), this);
    connect(aSaveDeckAs, SIGNAL(triggered()), deckEditor, SLOT(actSaveDeckAs()));

    aLoadDeckFromClipboard = new QAction(QString(), this);
    connect(aLoadDeckFromClipboard, SIGNAL(triggered()), deckEditor, SLOT(actLoadDeckFromClipboard()));

    aEditDeckInClipboard = new QAction(QString(), this);
    connect(aEditDeckInClipboard, SIGNAL(triggered()), deckEditor, SLOT(actEditDeckInClipboard()));

    aEditDeckInClipboardRaw = new QAction(QString(), this);
    connect(aEditDeckInClipboardRaw, SIGNAL(triggered()), deckEditor, SLOT(actEditDeckInClipboardRaw()));

    aSaveDeckToClipboard = new QAction(QString(), this);
    connect(aSaveDeckToClipboard, SIGNAL(triggered()), deckEditor, SLOT(actSaveDeckToClipboard()));

    aSaveDeckToClipboardNoSetInfo = new QAction(QString(), this);
    connect(aSaveDeckToClipboardNoSetInfo, SIGNAL(triggered()), deckEditor, SLOT(actSaveDeckToClipboardNoSetInfo()));

    aSaveDeckToClipboardRaw = new QAction(QString(), this);
    connect(aSaveDeckToClipboardRaw, SIGNAL(triggered()), deckEditor, SLOT(actSaveDeckToClipboardRaw()));

    aSaveDeckToClipboardRawNoSetInfo = new QAction(QString(), this);
    connect(aSaveDeckToClipboardRawNoSetInfo, SIGNAL(triggered()), deckEditor,
            SLOT(actSaveDeckToClipboardRawNoSetInfo()));

    aPrintDeck = new QAction(QString(), this);
    connect(aPrintDeck, SIGNAL(triggered()), deckEditor, SLOT(actPrintDeck()));

    aExportDeckDecklist = new QAction(QString(), this);
    connect(aExportDeckDecklist, SIGNAL(triggered()), deckEditor, SLOT(actExportDeckDecklist()));

    aAnalyzeDeckDeckstats = new QAction(QString(), this);
    connect(aAnalyzeDeckDeckstats, SIGNAL(triggered()), deckEditor, SLOT(actAnalyzeDeckDeckstats()));

    aAnalyzeDeckTappedout = new QAction(QString(), this);
    connect(aAnalyzeDeckTappedout, SIGNAL(triggered()), deckEditor, SLOT(actAnalyzeDeckTappedout()));

    analyzeDeckMenu = new QMenu(this);
    analyzeDeckMenu->addAction(aExportDeckDecklist);
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
    addMenu(analyzeDeckMenu);
    addSeparator();
    addAction(deckEditor->filterDockWidget->aClearFilterOne);
    addAction(deckEditor->filterDockWidget->aClearFilterAll);
    addSeparator();
    addAction(aClose);

    retranslateUi();
    connect(&SettingsCache::instance().shortcuts(), SIGNAL(shortCutChanged()), this, SLOT(refreshShortcuts()));
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

    analyzeDeckMenu->setTitle(tr("&Send deck to online service"));
    aExportDeckDecklist->setText(tr("Create decklist (decklist.org)"));
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
    aExportDeckDecklist->setShortcuts(shortcuts.getShortcut("TabDeckEditor/aExportDeckDecklist"));
    aSaveDeckAs->setShortcuts(shortcuts.getShortcut("TabDeckEditor/aSaveDeckAs"));
    aLoadDeckFromClipboard->setShortcuts(shortcuts.getShortcut("TabDeckEditor/aLoadDeckFromClipboard"));
    aEditDeckInClipboard->setShortcuts(shortcuts.getShortcut("TabDeckEditor/aEditDeckInClipboard"));
    aEditDeckInClipboardRaw->setShortcuts(shortcuts.getShortcut("TabDeckEditor/aEditDeckInClipboardRaw"));
    aPrintDeck->setShortcuts(shortcuts.getShortcut("TabDeckEditor/aPrintDeck"));
    aAnalyzeDeckDeckstats->setShortcuts(shortcuts.getShortcut("TabDeckEditor/aAnalyzeDeck"));
    aClose->setShortcuts(shortcuts.getShortcut("TabDeckEditor/aClose"));

    aSaveDeckToClipboard->setShortcuts(shortcuts.getShortcut("TabDeckEditor/aSaveDeckToClipboard"));
    aSaveDeckToClipboardNoSetInfo->setShortcuts(shortcuts.getShortcut("TabDeckEditor/aSaveDeckToClipboardNoSetInfo"));
    aSaveDeckToClipboardRaw->setShortcuts(shortcuts.getShortcut("TabDeckEditor/aSaveDeckToClipboardRaw"));
    aSaveDeckToClipboardRawNoSetInfo->setShortcuts(
        shortcuts.getShortcut("TabDeckEditor/aSaveDeckToClipboardRawNoSetInfo"));

    aClose->setShortcuts(shortcuts.getShortcut("TabDeckEditor/aClose"));
}
