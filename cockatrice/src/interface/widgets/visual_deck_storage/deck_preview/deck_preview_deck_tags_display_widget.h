/**
 * @file deck_preview_deck_tags_display_widget.h
 * @ingroup VisualDeckPreviewWidgets
 */

#ifndef DECK_PREVIEW_DECK_TAGS_DISPLAY_WIDGET_H
#define DECK_PREVIEW_DECK_TAGS_DISPLAY_WIDGET_H

#include <QStringList>
#include <QWidget>
#include <functional>

class FlowWidget;

class DeckPreviewDeckTagsDisplayWidget : public QWidget
{
    Q_OBJECT

    QStringList currentTags;
    FlowWidget *flowWidget;
    std::function<QStringList()> knownTagsProvider_;
    std::function<bool()> conversionPromptHandler_;

public:
    explicit DeckPreviewDeckTagsDisplayWidget(QWidget *_parent, const QStringList &_tags = {});
    void setTags(const QStringList &_tags);
    void refreshTags();

    /**
     * @brief Sets a provider for the tags shown in the edit dialog.
     * Defaults to scanning all deck files in the deck folder.
     */
    void setKnownTagsProvider(const std::function<QStringList()> &provider);

    /**
     * @brief Sets a handler run before opening the tag dialog. Returning false
     * cancels the dialog. Defaults to no handler (the deck editor path).
     */
    void setConversionPromptHandler(const std::function<bool()> &handler);

public slots:
    void openTagEditDlg();

signals:
    /**
     * Emitted when the tags have changed due to user interaction.
     * @param tags The new list of tags.
     */
    void tagsChanged(const QStringList &tags);

private:
    void execTagDialog(const QStringList &knownTags);
};
#endif // DECK_PREVIEW_DECK_TAGS_DISPLAY_WIDGET_H
