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
#include <QWidget>

class VisualDeckStorageWidget;

class DeckPreviewColorIdentityFilterWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DeckPreviewColorIdentityFilterWidget(VisualDeckStorageWidget *parent);
    void retranslateUi();

private slots:
    void handleColorToggled(QChar color, bool active);
    void updateFilterMode();

private:
    void applyFilter();

    QHBoxLayout *layout;
    QPushButton *toggleButton;
    QMap<QChar, bool> activeColors;
    VisualDeckStorageSortFilterProxyModel::FilterMode filterMode = VisualDeckStorageSortFilterProxyModel::Includes;
    VisualDeckStorageWidget *visualDeckStorageWidget; ///< The VDS owning the proxy model.
};

#endif // DECK_PREVIEW_COLOR_IDENTITY_FILTER_WIDGET_H
