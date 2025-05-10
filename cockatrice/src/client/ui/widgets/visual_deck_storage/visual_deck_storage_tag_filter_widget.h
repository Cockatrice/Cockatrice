#ifndef VISUAL_DECK_STORAGE_TAG_FILTER_WIDGET_H
#define VISUAL_DECK_STORAGE_TAG_FILTER_WIDGET_H

#include "visual_deck_storage_widget.h"

#include <QWidget>

class VisualDeckStorageWidget;
class VisualDeckStorageTagFilterWidget : public QWidget
{
    Q_OBJECT

    VisualDeckStorageWidget *parent;

    QSet<QString> gatherAllTags() const;
    void removeTagsNotInList(const QSet<QString> &tags);
    void addTagsIfNotPresent(const QSet<QString> &tags);
    void addTagIfNotPresent(const QString &tag);
    void sortTags();

public:
    explicit VisualDeckStorageTagFilterWidget(VisualDeckStorageWidget *_parent);
    QStringList getAllKnownTags() const;
    void filterDecksBySelectedTags(const QList<DeckPreviewWidget *> &deckPreviews) const;

public slots:
    void refreshTags();
    void showEvent(QShowEvent *event) override;
};

#endif // VISUAL_DECK_STORAGE_TAG_FILTER_WIDGET_H
