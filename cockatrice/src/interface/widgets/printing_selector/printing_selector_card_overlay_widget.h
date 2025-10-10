/**
 * @file printing_selector_card_overlay_widget.h
 * @ingroup PrintingWidgets
 * @brief TODO: Document this.
 */

#ifndef PRINTING_SELECTOR_CARD_OVERLAY_WIDGET_H
#define PRINTING_SELECTOR_CARD_OVERLAY_WIDGET_H

#include "../../../interface/widgets/tabs/abstract_tab_deck_editor.h"
#include "../cards/card_info_picture_widget.h"
#include "all_zones_card_amount_widget.h"
#include "card_amount_widget.h"
#include "set_name_and_collectors_number_display_widget.h"

#include <libcockatrice/card/card_info.h>
#include <libcockatrice/deck_list/deck_list_model.h>

class PrintingSelectorCardOverlayWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PrintingSelectorCardOverlayWidget(QWidget *parent,
                                               AbstractTabDeckEditor *_deckEditor,
                                               DeckListModel *_deckModel,
                                               QTreeView *_deckView,
                                               QSlider *_cardSizeSlider,
                                               const ExactCard &_rootCard);

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
    AbstractTabDeckEditor *deckEditor;
    DeckListModel *deckModel;
    QTreeView *deckView;
    QSlider *cardSizeSlider;
    ExactCard rootCard;
};

#endif // PRINTING_SELECTOR_CARD_OVERLAY_WIDGET_H
