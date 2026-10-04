/**
 * @file deck_preview_color_identity_filter_widget.h
 * @ingroup VisualDeckPreviewWidgets
 */

#ifndef DECK_PREVIEW_COLOR_IDENTITY_FILTER_WIDGET_H
#define DECK_PREVIEW_COLOR_IDENTITY_FILTER_WIDGET_H

#include "../visual_deck_storage_sort_filter_proxy_model.h"

#include <QHBoxLayout>
#include <QMap>
#include <QPushButton>
#include <QSet>
#include <QWidget>

class DeckPreviewColorIdentityFilterWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DeckPreviewColorIdentityFilterWidget(QWidget *parent = nullptr);
    void retranslateUi();

    /**
     * @brief The currently active color identity filter mode.
     */
    [[nodiscard]] VisualDeckStorageSortFilterProxyModel::FilterMode getFilterMode() const
    {
        return filterMode;
    }

    /**
     * @brief The colors that are currently toggled on.
     */
    [[nodiscard]] QSet<QChar> getActiveColors() const;

signals:
    /**
     * Emitted when the set of active colors changed due to user interaction.
     */
    void activeColorsChanged();

    /**
     * Emitted when the user cycles the color identity filter mode.
     * @param mode The new filter mode.
     */
    void filterModeChanged(VisualDeckStorageSortFilterProxyModel::FilterMode mode);

private slots:
    void handleColorToggled(QChar color, bool active);
    void updateFilterMode();

private:
    QHBoxLayout *layout;
    QPushButton *toggleButton;
    QMap<QChar, bool> activeColors;
    VisualDeckStorageSortFilterProxyModel::FilterMode filterMode = VisualDeckStorageSortFilterProxyModel::Includes;
};

#endif // DECK_PREVIEW_COLOR_IDENTITY_FILTER_WIDGET_H
