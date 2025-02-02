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
    QStringList gatherAllTags();
    void filterDecksBySelectedTags(const QList<DeckPreviewWidget *> &deckPreviews) const;
    void removeTagsNotInList(const QStringList &tags);
    void addTagsIfNotPresent(const QStringList &tags);
    void addTagIfNotPresent(const QString &tag);
    QStringList getAllKnownTags();
    VisualDeckStorageWidget *parent;

public slots:
    void refreshTags();
};

#endif // VISUAL_DECK_STORAGE_TAG_FILTER_WIDGET_H
