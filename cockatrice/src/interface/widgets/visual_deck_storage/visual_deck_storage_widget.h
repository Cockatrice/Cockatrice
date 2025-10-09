/**
 * @file visual_deck_storage_widget.h
 * @ingroup VisualDeckStorageWidgets
 * @brief TODO: Document this.
 */

#ifndef VISUAL_DECK_STORAGE_WIDGET_H
#define VISUAL_DECK_STORAGE_WIDGET_H

#include "../cards/card_size_widget.h"
#include "../general/layout_containers/flow_widget.h"
#include "../quick_settings/settings_button_widget.h"
#include "deck_preview/deck_preview_color_identity_filter_widget.h"
#include "deck_preview/deck_preview_widget.h"
#include "visual_deck_storage_folder_display_widget.h"
#include "visual_deck_storage_quick_settings_widget.h"
#include "visual_deck_storage_search_widget.h"
#include "visual_deck_storage_sort_widget.h"
#include "visual_deck_storage_tag_filter_widget.h"

#include <QCheckBox>
#include <QFileSystemModel>
#include <libcockatrice/deck_list/deck_list_model.h>

class QSpinBox;
class VisualDeckStorageSearchWidget;
class VisualDeckStorageSortWidget;
class VisualDeckStorageTagFilterWidget;
class VisualDeckStorageFolderDisplayWidget;
class DeckPreviewColorIdentityFilterWidget;
class VisualDeckStorageWidget final : public QWidget
{
    Q_OBJECT
public:
    explicit VisualDeckStorageWidget(QWidget *parent);
    void refreshIfPossible();
    void retranslateUi();

    VisualDeckStorageTagFilterWidget *tagFilterWidget;
    bool deckPreviewSelectionAnimationEnabled;

    const VisualDeckStorageQuickSettingsWidget *settings() const;

public slots:
    void createRootFolderWidget(); // Refresh the display of cards based on the current sorting option
    void updateShowFolders(bool enabled);
    void updateTagFilter();
    void updateColorFilter();
    void updateSearchFilter();
    void updateTagsVisibility(bool visible);
    void updateSelectionAnimationEnabled(bool enabled);
    void updateSortOrder();
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;

signals:
    void bannerCardsRefreshed();
    void deckLoadRequested(const QString &filePath);
    void openDeckEditor(const DeckLoader *deck);

private:
    QVBoxLayout *layout;
    QWidget *searchAndSortContainer;
    QHBoxLayout *searchAndSortLayout;
    DeckListModel *deckListModel;
    QLabel *databaseLoadIndicator;
    VisualDeckStorageSortWidget *sortWidget;
    VisualDeckStorageSearchWidget *searchWidget;
    DeckPreviewColorIdentityFilterWidget *deckPreviewColorIdentityFilterWidget;
    QToolButton *refreshButton;
    VisualDeckStorageQuickSettingsWidget *quickSettingsWidget;
    QScrollArea *scrollArea;
    VisualDeckStorageFolderDisplayWidget *folderWidget;

    void reapplySortAndFilters();
};

#endif // VISUAL_DECK_STORAGE_WIDGET_H
