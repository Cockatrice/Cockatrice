/**
 * @file deck_preview_color_identity_filter_widget.h
 * @ingroup VisualDeckPreviewWidgets
 * @brief TODO: Document this.
 */

#ifndef DECK_PREVIEW_COLOR_IDENTITY_FILTER_WIDGET_H
#define DECK_PREVIEW_COLOR_IDENTITY_FILTER_WIDGET_H

#include "../visual_deck_storage_widget.h"

#include <QChar>
#include <QHBoxLayout>
#include <QList>
#include <QPushButton>
#include <QWidget>

class DeckPreviewWidget;
class VisualDeckStorageWidget;

class DeckPreviewColorIdentityFilterWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DeckPreviewColorIdentityFilterWidget(VisualDeckStorageWidget *parent);
    void retranslateUi();
    void filterWidgets(QList<DeckPreviewWidget *> widgets);

signals:
    void filterModeChanged(bool exactMatchMode);
    void activeColorsChanged();

private slots:
    void handleColorToggled(QChar color, bool active);
    void updateFilterMode(bool checked);

private:
    QHBoxLayout *layout;
    QPushButton *toggleButton;
    QMap<QChar, bool> activeColors;
    bool exactMatchMode = false; // Default to "includes" mode
};

#endif // DECK_PREVIEW_COLOR_IDENTITY_FILTER_WIDGET_H
