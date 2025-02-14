#ifndef PRINTING_SELECTOR_CARD_OVERLAY_WIDGET_H
#define PRINTING_SELECTOR_CARD_OVERLAY_WIDGET_H

#include "../../../../client/ui/widgets/cards/card_info_picture_widget.h"
#include "../../../../deck/deck_list_model.h"
#include "../../../../game/cards/card_database.h"
#include "../../../tabs/tab_generic_deck_editor.h"
#include "all_zones_card_amount_widget.h"
#include "card_amount_widget.h"
#include "set_name_and_collectors_number_display_widget.h"

class PrintingSelectorCardOverlayWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PrintingSelectorCardOverlayWidget(QWidget *parent,
                                               TabGenericDeckEditor *_deckEditor,
                                               DeckListModel *_deckModel,
                                               QTreeView *_deckView,
                                               QSlider *_cardSizeSlider,
                                               CardInfoPtr _rootCard,
                                               const CardInfoPerSet &_setInfoForCard);

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

signals:
    void cardPreferenceChanged();

private:
    CardInfoPictureWidget *cardInfoPicture;
    AllZonesCardAmountWidget *allZonesCardAmountWidget;
    TabGenericDeckEditor *deckEditor;
    DeckListModel *deckModel;
    QTreeView *deckView;
    QSlider *cardSizeSlider;
    CardInfoPtr rootCard;
    CardInfoPtr setCard;
    CardInfoPerSet setInfoForCard;
};

#endif // PRINTING_SELECTOR_CARD_OVERLAY_WIDGET_H
