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
    explicit DeckPreviewDeckTagsDisplayWidget(QWidget *_parent, DeckList *_deckList);
    void connectDeckList(DeckList *_deckList);
    void refreshTags();
    DeckList *deckList;
    FlowWidget *flowWidget;

public slots:
    void openTagEditDlg();
};
#endif // DECK_PREVIEW_DECK_TAGS_DISPLAY_WIDGET_H
