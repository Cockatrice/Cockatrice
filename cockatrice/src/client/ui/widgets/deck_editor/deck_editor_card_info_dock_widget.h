#ifndef DECK_EDITOR_CARD_INFO_DOCK_WIDGET_H
#define DECK_EDITOR_CARD_INFO_DOCK_WIDGET_H

#include "../cards/card_info_frame_widget.h"

#include <QDockWidget>

class DeckEditorCardInfoDockWidget : public QDockWidget
{
    Q_OBJECT
public:
    explicit DeckEditorCardInfoDockWidget(QWidget *parent);
    void createCardInfoDock();
    void retranslateUi();
    CardInfoFrameWidget *cardInfo;
};

#endif // DECK_EDITOR_CARD_INFO_DOCK_WIDGET_H
