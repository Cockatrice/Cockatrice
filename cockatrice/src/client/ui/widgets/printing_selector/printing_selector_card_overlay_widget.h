#ifndef PRINTING_SELECTOR_CARD_OVERLAY_WIDGET_H
#define PRINTING_SELECTOR_CARD_OVERLAY_WIDGET_H

#ifndef CARDOVERLAYWIDGET_H
#define CARDOVERLAYWIDGET_H

#include "../../../../client/ui/widgets/cards/card_info_picture_widget.h"
#include "../../../../deck/deck_list_model.h"
#include "../../../../deck/deck_view.h"
#include "../../../../game/cards/card_database.h"
#include "../../../tabs/tab_deck_editor.h"
#include "all_zones_card_amount_widget.h"
#include "card_amount_widget.h"
#include "set_name_and_collectors_number_display_widget.h"

class PrintingSelectorCardOverlayWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PrintingSelectorCardOverlayWidget(QWidget *parent,
                                               TabDeckEditor *deckEditor,
                                               DeckListModel *deckModel,
                                               QTreeView *deckView,
                                               QSlider *cardSizeSlider,
                                               CardInfoPtr rootCard,
                                               CardInfoPerSet setInfoForCard);

protected:
    void resizeEvent(QResizeEvent *event) override;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent *event) override;
#else
    void enterEvent(QEvent *event) override;
#endif
    void leaveEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void customMenu(QPoint point);

private:
    CardInfoPictureWidget *cardInfoPicture;
    AllZonesCardAmountWidget *allZonesCardAmountWidget;
    TabDeckEditor *deckEditor;
    DeckListModel *deckModel;
    QTreeView *deckView;
    QSlider *cardSizeSlider;
    CardInfoPtr rootCard;
    CardInfoPtr setCard;
    CardInfoPerSet setInfoForCard;
};

#endif // CARDOVERLAYWIDGET_H

#endif // PRINTING_SELECTOR_CARD_OVERLAY_WIDGET_H
