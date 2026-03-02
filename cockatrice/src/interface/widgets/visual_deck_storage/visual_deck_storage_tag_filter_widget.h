/**
 * @file visual_deck_storage_tag_filter_widget.h
 * @ingroup VisualDeckStorageWidgets
 * @brief TODO: Document this.
 */

#ifndef VISUAL_DECK_STORAGE_TAG_FILTER_WIDGET_H
#define VISUAL_DECK_STORAGE_TAG_FILTER_WIDGET_H

#include "deck_preview/deck_preview_widget.h"

#include <QWidget>

class VisualDeckStorageWidget;
class VisualDeckStorageTagFilterWidget : public QWidget
{
    Q_OBJECT

    VisualDeckStorageWidget *parent;

    [[nodiscard]] QSet<QString> gatherAllTags() const;
    void removeTagsNotInList(const QSet<QString> &tags);
    void addTagsIfNotPresent(const QSet<QString> &tags);
    void addTagIfNotPresent(const QString &tag);
    void sortTags();

public:
    explicit VisualDeckStorageTagFilterWidget(VisualDeckStorageWidget *_parent);
    [[nodiscard]] QStringList getAllKnownTags() const;
    void filterDecksBySelectedTags(const QList<DeckPreviewWidget *> &deckPreviews) const;

public slots:
    void refreshTags();
    void showEvent(QShowEvent *event) override;
};

#endif // VISUAL_DECK_STORAGE_TAG_FILTER_WIDGET_H
