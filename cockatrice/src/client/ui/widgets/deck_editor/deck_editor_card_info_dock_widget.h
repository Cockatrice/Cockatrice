#ifndef DECK_EDITOR_CARD_INFO_DOCK_WIDGET_H
#define DECK_EDITOR_CARD_INFO_DOCK_WIDGET_H

#include "../../../tabs/tab_generic_deck_editor.h"
#include "../cards/card_info_frame_widget.h"

#include <QDockWidget>

class TabGenericDeckEditor;
class DeckEditorCardInfoDockWidget : public QDockWidget
{
    Q_OBJECT
public:
    explicit DeckEditorCardInfoDockWidget(QWidget *parent, TabGenericDeckEditor *deckEditor);
    void createCardInfoDock();
    void retranslateUi();

    TabGenericDeckEditor *deckEditor;
    CardInfoFrameWidget *cardInfo;
};

#endif // DECK_EDITOR_CARD_INFO_DOCK_WIDGET_H
