#ifndef DECK_EDITOR_MENU_H
#define DECK_EDITOR_MENU_H

#include "../../tabs/tab_generic_deck_editor.h"

#include <QMenu>

class TabGenericDeckEditor;
class DeckEditorMenu : public QMenu
{
    Q_OBJECT
public:
    explicit DeckEditorMenu(QWidget *parent, TabGenericDeckEditor *deckEditor);

    TabGenericDeckEditor *deckEditor;

    QAction *aNewDeck, *aLoadDeck, *aClearRecents, *aSaveDeck, *aSaveDeckAs, *aLoadDeckFromClipboard,
        *aSaveDeckToClipboard, *aSaveDeckToClipboardNoSetNameAndNumber, *aSaveDeckToClipboardRaw,
        *aSaveDeckToClipboardRawNoSetNameAndNumber, *aPrintDeck, *aExportDeckDecklist, *aAnalyzeDeckDeckstats,
        *aAnalyzeDeckTappedout, *aClose;
    QMenu *loadRecentDeckMenu, *analyzeDeckMenu, *saveDeckToClipboardMenu;

    void setSaveStatus(bool newStatus);

public slots:
    void updateRecentlyOpened();
    void actClearRecents();
    void retranslateUi();
    void refreshShortcuts();
};

#endif
