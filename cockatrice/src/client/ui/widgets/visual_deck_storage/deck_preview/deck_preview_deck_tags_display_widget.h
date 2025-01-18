#ifndef DECK_PREVIEW_DECK_TAGS_DISPLAY_WIDGET_H
#define DECK_PREVIEW_DECK_TAGS_DISPLAY_WIDGET_H

#include "../../../../../deck/deck_loader.h"
#include "deck_preview_widget.h"

#include <QWidget>

class DeckPreviewWidget;
class DeckPreviewDeckTagsDisplayWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DeckPreviewDeckTagsDisplayWidget(DeckPreviewWidget *_parent, DeckLoader *_deckLoader);
    void refreshTags();
    DeckPreviewWidget *parent;
    DeckLoader *deckLoader;
    FlowWidget *flowWidget;
};
#endif // DECK_PREVIEW_DECK_TAGS_DISPLAY_WIDGET_H
