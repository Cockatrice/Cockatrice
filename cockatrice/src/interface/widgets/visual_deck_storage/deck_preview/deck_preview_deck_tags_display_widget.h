/**
 * @file deck_preview_deck_tags_display_widget.h
 * @ingroup VisualDeckPreviewWidgets
 * @brief TODO: Document this.
 */

#ifndef DECK_PREVIEW_DECK_TAGS_DISPLAY_WIDGET_H
#define DECK_PREVIEW_DECK_TAGS_DISPLAY_WIDGET_H

#include "../../../deck_loader/deck_loader.h"
#include "deck_preview_widget.h"

#include <QWidget>

class DeckPreviewWidget;
class DeckPreviewDeckTagsDisplayWidget : public QWidget
{
    Q_OBJECT

    DeckList *deckList;
    FlowWidget *flowWidget;

public:
    explicit DeckPreviewDeckTagsDisplayWidget(QWidget *_parent, DeckList *_deckList);
    void setDeckList(DeckList *_deckList);
    void refreshTags();

public slots:
    void openTagEditDlg();

private:
    bool promptFileConversionIfRequired(DeckPreviewWidget *deckPreviewWidget);
};
#endif // DECK_PREVIEW_DECK_TAGS_DISPLAY_WIDGET_H
