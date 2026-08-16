/**
 * @file deck_preview_color_identity_filter_widget.h
 * @ingroup VisualDeckPreviewWidgets
 */
//! \todo Document this file.

#ifndef DECK_PREVIEW_COLOR_IDENTITY_FILTER_WIDGET_H
#define DECK_PREVIEW_COLOR_IDENTITY_FILTER_WIDGET_H

#include "../visual_deck_storage_widget.h"

#include <QHBoxLayout>
#include <QPushButton>
#include <QWidget>

class DeckPreviewWidget;
class VisualDeckStorageWidget;

class DeckPreviewColorIdentityFilterWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * How the active colors are matched against a deck's color identity.
     */
    enum FilterMode
    {
        ExactMatch, ///< The color identity consists of exactly the active colors.
        Includes,   ///< The color identity contains all of the active colors.
        Excludes    ///< The color identity contains none of the active colors.
    };
    Q_ENUM(FilterMode)

    explicit DeckPreviewColorIdentityFilterWidget(VisualDeckStorageWidget *parent);
    void retranslateUi();
    void filterWidgets(QList<DeckPreviewWidget *> widgets);

signals:
    void filterModeChanged(FilterMode mode);
    void activeColorsChanged();

private slots:
    void handleColorToggled(QChar color, bool active);
    void updateFilterMode();

private:
    QHBoxLayout *layout;
    QPushButton *toggleButton;
    QMap<QChar, bool> activeColors;
    FilterMode filterMode = Includes; // Default to "includes" mode
};

#endif // DECK_PREVIEW_COLOR_IDENTITY_FILTER_WIDGET_H
