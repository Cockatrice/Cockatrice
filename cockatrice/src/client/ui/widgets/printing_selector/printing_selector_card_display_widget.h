#ifndef PRINTING_SELECTOR_CARD_DISPLAY_WIDGET_H
#define PRINTING_SELECTOR_CARD_DISPLAY_WIDGET_H

#include "../../../../client/ui/widgets/cards/card_info_picture_widget.h"
#include "../../../../deck/deck_list_model.h"
#include "../../../../deck/deck_view.h"
#include "../../../../game/cards/card_database.h"
#include "../../../tabs/tab_deck_editor.h"
#include "all_zones_card_amount_widget.h"
#include "card_amount_widget.h"
#include "printing_selector_card_overlay_widget.h"
#include "set_name_and_collectors_number_display_widget.h"

#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QTreeView>
#include <QVBoxLayout>
#include <QWidget>

class PrintingSelectorCardDisplayWidget : public QWidget
{
    Q_OBJECT

public:
    PrintingSelectorCardDisplayWidget(QWidget *parent,
                                      TabDeckEditor *deckEditor,
                                      DeckListModel *deckModel,
                                      QTreeView *deckView,
                                      QSlider *cardSizeSlider,
                                      CardInfoPtr rootCard,
                                      CardInfoPerSet setInfoForCard,
                                      QString &currentZone);
    void enterEvent(QEnterEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    QVBoxLayout *layout;
    AllZonesCardAmountWidget *allZonesCardAmountWidget;
    SetNameAndCollectorsNumberDisplayWidget *setNameAndCollectorsNumberDisplayWidget;
    TabDeckEditor *deckEditor;
    DeckListModel *deckModel;
    QTreeView *deckView;
    QSlider *cardSizeSlider;
    CardInfoPtr rootCard;
    CardInfoPtr setCard;
    CardInfoPerSet setInfoForCard;
    QString currentZone;
    CardInfoPictureWidget *cardInfoPicture;
    PrintingSelectorCardOverlayWidget *overlayWidget;
};

#endif // PRINTING_SELECTOR_CARD_DISPLAY_WIDGET_H
