#ifndef PRINTING_SELECTOR_CARD_DISPLAY_WIDGET_H
#define PRINTING_SELECTOR_CARD_DISPLAY_WIDGET_H

#include "../../../../deck/deck_list_model.h"
#include "../../../../game/cards/card_info.h"
#include "../../../tabs/abstract_tab_deck_editor.h"
#include "printing_selector_card_overlay_widget.h"
#include "set_name_and_collectors_number_display_widget.h"

#include <QPainter>
#include <QWidget>

class PrintingSelectorCardDisplayWidget : public QWidget
{
    Q_OBJECT

public:
    PrintingSelectorCardDisplayWidget(QWidget *parent,
                                      AbstractTabDeckEditor *_deckEditor,
                                      DeckListModel *_deckModel,
                                      QTreeView *_deckView,
                                      QSlider *_cardSizeSlider,
                                      CardInfoPtr _rootCard,
                                      const PrintingInfo &_printingInfo,
                                      QString &_currentZone);

public slots:
    void clampSetNameToPicture();

signals:
    void cardPreferenceChanged();

private:
    QVBoxLayout *layout;
    SetNameAndCollectorsNumberDisplayWidget *setNameAndCollectorsNumberDisplayWidget;
    AbstractTabDeckEditor *deckEditor;
    DeckListModel *deckModel;
    QTreeView *deckView;
    QSlider *cardSizeSlider;
    CardInfoPtr rootCard;
    CardInfoPtr setCard;
    PrintingInfo printingInfo;
    QString currentZone;
    PrintingSelectorCardOverlayWidget *overlayWidget;
};

#endif // PRINTING_SELECTOR_CARD_DISPLAY_WIDGET_H
