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

    QStringList currentTags;
    FlowWidget *flowWidget;

public:
    explicit DeckPreviewDeckTagsDisplayWidget(QWidget *_parent, const QStringList &_tags);
    void setTags(const QStringList &_tags);
    void refreshTags();

public slots:
    void openTagEditDlg();

private:
    bool promptFileConversionIfRequired(DeckPreviewWidget *deckPreviewWidget);
    void execTagDialog(const QStringList &knownTags);

signals:
    /**
     * Emitted when the tags have changed due to user interaction.
     * @param tags The new list of tags.
     */
    void tagsChanged(const QStringList &tags);
};
#endif // DECK_PREVIEW_DECK_TAGS_DISPLAY_WIDGET_H
