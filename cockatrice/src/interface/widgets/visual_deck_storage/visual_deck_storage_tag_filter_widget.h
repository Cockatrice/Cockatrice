/**
 * @file visual_deck_storage_tag_filter_widget.h
 * @ingroup VisualDeckStorageWidgets
 */

#ifndef VISUAL_DECK_STORAGE_TAG_FILTER_WIDGET_H
#define VISUAL_DECK_STORAGE_TAG_FILTER_WIDGET_H

#include <QSet>
#include <QStringList>
#include <QWidget>
#include <functional>

class DeckPreviewTagDisplayWidget;
class FlowWidget;

class VisualDeckStorageTagFilterWidget : public QWidget
{
    Q_OBJECT

    FlowWidget *flowWidget;
    std::function<QSet<QString>()> allTagsProvider;
    QList<DeckPreviewTagDisplayWidget *> currentChipOrder;

public:
    explicit VisualDeckStorageTagFilterWidget(QWidget *parent = nullptr);
    [[nodiscard]] QStringList getAllKnownTags() const;

    /**
     * @brief Sets a provider for the full set of tags to draw chips from.
     */
    void setAllTagsProvider(const std::function<QSet<QString>()> &provider);

    /**
     * @brief The tags currently in "selected" state.
     */
    [[nodiscard]] QStringList selectedTags() const;

    /**
     * @brief The tags currently in "excluded" state.
     */
    [[nodiscard]] QStringList excludedTags() const;

signals:
    /**
     * Emitted when a chip's selection or exclusion state changes.
     *
     * The chip only emits when its state actually changed, so this fires once per
     * effective toggle rather than on every click."
     */
    void filterChanged();

public slots:
    /**
     * @brief Rebuilds the tag chips from the currently available tags.
     */
    void refreshTags();
    void showEvent(QShowEvent *event) override;
};

#endif // VISUAL_DECK_STORAGE_TAG_FILTER_WIDGET_H
