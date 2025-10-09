/**
 * @file deck_editor_menu.h
 * @ingroup DeckEditors
 * @brief TODO: Document this.
 */

#ifndef DECK_EDITOR_MENU_H
#define DECK_EDITOR_MENU_H

#include "../interface/widgets/tabs/abstract_tab_deck_editor.h"

#include <QMenu>

class AbstractTabDeckEditor;
class DeckEditorMenu : public QMenu
{
    Q_OBJECT
public:
    explicit DeckEditorMenu(AbstractTabDeckEditor *parent);

    AbstractTabDeckEditor *deckEditor;

    QAction *aNewDeck, *aLoadDeck, *aClearRecents, *aSaveDeck, *aSaveDeckAs, *aLoadDeckFromClipboard,
        *aEditDeckInClipboard, *aEditDeckInClipboardRaw, *aSaveDeckToClipboard, *aSaveDeckToClipboardNoSetInfo,
        *aSaveDeckToClipboardRaw, *aSaveDeckToClipboardRawNoSetInfo, *aPrintDeck, *aLoadDeckFromWebsite,
        *aExportDeckDecklist, *aExportDeckDecklistXyz, *aAnalyzeDeckDeckstats, *aAnalyzeDeckTappedout, *aClose;
    QMenu *loadRecentDeckMenu, *analyzeDeckMenu, *editDeckInClipboardMenu, *saveDeckToClipboardMenu;

    void setSaveStatus(bool newStatus);

public slots:
    void updateRecentlyOpened();
    void actClearRecents();
    void retranslateUi();
    void refreshShortcuts();
};

#endif
