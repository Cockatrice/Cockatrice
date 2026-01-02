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

#include <QWidget>
#include <libcockatrice/models/deck_list/deck_list_model.h>

class PrintingSelectorCardDisplayWidget : public QWidget
{
    Q_OBJECT

public:
    PrintingSelectorCardDisplayWidget(QWidget *parent,
                                      AbstractTabDeckEditor *deckEditor,
                                      DeckStateManager *deckStateManager,
                                      QSlider *cardSizeSlider,
                                      const ExactCard &rootCard);

public slots:
    void clampSetNameToPicture();
    void updateCardAmounts(const QMap<QString, QPair<int, int>> &uuidToAmounts);

signals:
    void cardPreferenceChanged();

private:
    ExactCard rootCard;
    QVBoxLayout *layout;
    SetNameAndCollectorsNumberDisplayWidget *setNameAndCollectorsNumberDisplayWidget;
    PrintingSelectorCardOverlayWidget *overlayWidget;
};

#endif // PRINTING_SELECTOR_CARD_DISPLAY_WIDGET_H
