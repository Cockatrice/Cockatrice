/**
 * @file printing_selector_card_display_widget.h
 * @ingroup PrintingWidgets
 * @brief TODO: Document this.
 */

#ifndef PRINTING_SELECTOR_CARD_DISPLAY_WIDGET_H
#define PRINTING_SELECTOR_CARD_DISPLAY_WIDGET_H

#include "../../../interface/widgets/tabs/abstract_tab_deck_editor.h"
#include "printing_selector_card_overlay_widget.h"
#include "set_name_and_collectors_number_display_widget.h"

#include <QPainter>
#include <QWidget>
#include <libcockatrice/card/card_info.h>
#include <libcockatrice/deck_list/deck_list_model.h>

class PrintingSelectorCardDisplayWidget : public QWidget
{
    Q_OBJECT

public:
    PrintingSelectorCardDisplayWidget(QWidget *parent,
                                      AbstractTabDeckEditor *_deckEditor,
                                      DeckListModel *_deckModel,
                                      QTreeView *_deckView,
                                      QSlider *_cardSizeSlider,
                                      const ExactCard &_rootCard,
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
    ExactCard rootCard;
    QString currentZone;
    PrintingSelectorCardOverlayWidget *overlayWidget;
};

#endif // PRINTING_SELECTOR_CARD_DISPLAY_WIDGET_H
