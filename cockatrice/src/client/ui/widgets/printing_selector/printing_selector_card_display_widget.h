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
                                      TabDeckEditor *_deckEditor,
                                      DeckListModel *_deckModel,
                                      QTreeView *_deckView,
                                      QSlider *_cardSizeSlider,
                                      CardInfoPtr _rootCard,
                                      CardInfoPerSet *_setInfoForCard,
                                      QString &_currentZone);

public slots:
    void clampSetNameToPicture();

private:
    QVBoxLayout *layout;
    SetNameAndCollectorsNumberDisplayWidget *setNameAndCollectorsNumberDisplayWidget;
    TabDeckEditor *deckEditor;
    DeckListModel *deckModel;
    QTreeView *deckView;
    QSlider *cardSizeSlider;
    CardInfoPtr rootCard;
    CardInfoPtr setCard;
    CardInfoPerSet *setInfoForCard;
    QString currentZone;
    PrintingSelectorCardOverlayWidget *overlayWidget;
};

#endif // PRINTING_SELECTOR_CARD_DISPLAY_WIDGET_H
