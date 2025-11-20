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

inline bool confirmOverwriteIfExists(QWidget *parent, const QString &filePath);

class DeckPreviewWidget;
class DeckPreviewDeckTagsDisplayWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DeckPreviewDeckTagsDisplayWidget(QWidget *_parent, DeckLoader *_deckLoader);
    void connectDeckList();
    void refreshTags();
    DeckLoader *deckLoader;
    FlowWidget *flowWidget;

public slots:
    void openTagEditDlg();
};
#endif // DECK_PREVIEW_DECK_TAGS_DISPLAY_WIDGET_H
