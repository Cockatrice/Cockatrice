#ifndef DECK_EDITOR_CARD_INFO_DOCK_WIDGET_H
#define DECK_EDITOR_CARD_INFO_DOCK_WIDGET_H

#include "../../../tabs/abstract_tab_deck_editor.h"
#include "../cards/card_info_frame_widget.h"

#include <QDockWidget>

class AbstractTabDeckEditor;
class DeckEditorCardInfoDockWidget : public QDockWidget
{
    Q_OBJECT
public:
    explicit DeckEditorCardInfoDockWidget(AbstractTabDeckEditor *parent);
    void createCardInfoDock();
    void retranslateUi();

    AbstractTabDeckEditor *deckEditor;
    CardInfoFrameWidget *cardInfo;

public slots:
    void updateCard(const ExactCard &_card);
};

#endif // DECK_EDITOR_CARD_INFO_DOCK_WIDGET_H
