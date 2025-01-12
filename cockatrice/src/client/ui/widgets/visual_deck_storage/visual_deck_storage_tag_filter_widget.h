#ifndef VISUAL_DECK_STORAGE_TAG_FILTER_WIDGET_H
#define VISUAL_DECK_STORAGE_TAG_FILTER_WIDGET_H

#include "visual_deck_storage_widget.h"

#include <QWidget>

class VisualDeckStorageWidget;
class VisualDeckStorageTagFilterWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VisualDeckStorageTagFilterWidget(VisualDeckStorageWidget *_parent);
    void refreshTags();
    QList<DeckPreviewWidget *> filterDecksBySelectedTags(const QList<DeckPreviewWidget *> &deckPreviews) const;
    void removeTagsNotInList(const QStringList &tags);
    void addTagsIfNotPresent(const QStringList &tags);
    void addTagIfNotPresent(const QString &tag);
    VisualDeckStorageWidget *parent;
};

#endif // VISUAL_DECK_STORAGE_TAG_FILTER_WIDGET_H
