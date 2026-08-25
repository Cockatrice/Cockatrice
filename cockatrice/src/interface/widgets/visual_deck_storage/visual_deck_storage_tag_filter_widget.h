/**
 * @file visual_deck_storage_tag_filter_widget.h
 * @ingroup VisualDeckStorageWidgets
 */

#ifndef VISUAL_DECK_STORAGE_TAG_FILTER_WIDGET_H
#define VISUAL_DECK_STORAGE_TAG_FILTER_WIDGET_H

#include <QSet>
#include <QStringList>
#include <QWidget>

class FlowWidget;
class VisualDeckStorageWidget;
class VisualDeckStorageTagFilterWidget : public QWidget
{
    Q_OBJECT

    VisualDeckStorageWidget *parent;
    FlowWidget *flowWidget;

    [[nodiscard]] QSet<QString> gatherAllTags() const;
    void removeTagsNotInList(const QSet<QString> &tags);
    void addTagsIfNotPresent(const QSet<QString> &tags);
    void addTagIfNotPresent(const QString &tag);
    void sortTags();

public:
    explicit VisualDeckStorageTagFilterWidget(VisualDeckStorageWidget *_parent);
    [[nodiscard]] QStringList getAllKnownTags() const;

    /**
     * @brief The tags currently in "selected" state.
     */
    [[nodiscard]] QStringList selectedTags() const;

    /**
     * @brief The tags currently in "excluded" state.
     */
    [[nodiscard]] QStringList excludedTags() const;

public slots:
    /**
     * @brief Rebuilds the tag chips from the tags of the currently visible decks.
     */
    void refreshTags();
    void showEvent(QShowEvent *event) override;
};

#endif // VISUAL_DECK_STORAGE_TAG_FILTER_WIDGET_H
