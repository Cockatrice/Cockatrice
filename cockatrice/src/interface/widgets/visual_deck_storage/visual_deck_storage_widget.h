/**
 * @file visual_deck_storage_widget.h
 * @ingroup VisualDeckStorageWidgets
 */

#ifndef VISUAL_DECK_STORAGE_WIDGET_H
#define VISUAL_DECK_STORAGE_WIDGET_H

#include "../../deck_loader/deck_loader.h"
#include "../cards/card_size_widget.h"
#include "../general/tutorial/tutorial_controller.h"
#include "../quick_settings/settings_button_widget.h"
#include "deck_preview/deck_preview_color_identity_filter_widget.h"
#include "visual_deck_storage_folder_display_widget.h"
#include "visual_deck_storage_quick_settings_widget.h"
#include "visual_deck_storage_search_widget.h"
#include "visual_deck_storage_sort_widget.h"
#include "visual_deck_storage_tag_filter_widget.h"

#include <QSet>
#include <QString>
#include <QStringList>
#include <QWidget>
#include <qtmetamacros.h>

class QLabel;
class QTimer;
class DeckPreviewColorIdentityFilterWidget;
class VisualDeckStorageFolderDisplayWidget;
class VisualDeckStorageQuickSettingsWidget;
class VisualDeckStorageSearchWidget;
class VisualDeckStorageSortWidget;
class VisualDeckStorageTagFilterWidget;
class QHBoxLayout;
class QScrollArea;
class QToolButton;
class QVBoxLayout;
class VisualDeckStorageModel;
class VisualDeckStorageSortFilterProxyModel;
struct LoadedDeck;

class VisualDeckStorageWidget final : public QWidget
{
    Q_OBJECT
public:
    explicit VisualDeckStorageWidget(QWidget *parent);
    void refreshIfPossible();
    void retranslateUi();
    void generateTutorialSequence(TutorialSequence &vdsSequence);
    void setShareSelectable(bool selectable);
    [[nodiscard]] bool isShareSelectable() const;
    [[nodiscard]] QStringList selectedFilePaths() const;
    void setShareSelectedFiles(const QStringList &paths);
    void clearShareSelection();
    void setShareAvailable(bool available);

    VisualDeckStorageTagFilterWidget *tagFilterWidget;
    bool deckPreviewSelectionAnimationEnabled;

    [[nodiscard]] const VisualDeckStorageQuickSettingsWidget *settings() const;
    [[nodiscard]] VisualDeckStorageModel *model() const
    {
        return storageModel;
    }
    [[nodiscard]] VisualDeckStorageSortFilterProxyModel *proxyModel() const
    {
        return storageProxyModel;
    }

public slots:
    /**
     * @brief Starts scanning the deck folder and rebuilds the folder tree and previews.
     */
    void createRootFolderWidget();
    void updateShowFolders(bool enabled);
    void updateTagsVisibility(bool visible);
    void updateSelectionAnimationEnabled(bool enabled);
    void updateSortOrder();
    void updateTagFilter();
    void updateColorFilter();
    void updateSearchFilter(const QString &text);

signals:
    void deckLoadRequested(const QString &filePath);
    void openDeckEditor(const LoadedDeck &deck);
    void shareDeckRequested(const QString &filePath);
    void shareSelectionChanged();
    void shareRequested();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    void reapplySortAndFilters();
    [[nodiscard]] QSet<QString> gatherVisibleTags() const;

private:
    QVBoxLayout *layout;
    QWidget *searchAndSortContainer;
    QHBoxLayout *searchAndSortLayout;
    QLabel *databaseLoadIndicator;
    VisualDeckStorageSortWidget *sortWidget;
    VisualDeckStorageSearchWidget *searchWidget;
    DeckPreviewColorIdentityFilterWidget *deckPreviewColorIdentityFilterWidget;
    QToolButton *refreshButton;
    QToolButton *shareButton;
    VisualDeckStorageQuickSettingsWidget *quickSettingsWidget;
    QScrollArea *scrollArea;
    VisualDeckStorageFolderDisplayWidget *folderWidget = nullptr;
    VisualDeckStorageModel *storageModel = nullptr;
    VisualDeckStorageSortFilterProxyModel *storageProxyModel = nullptr;
    QTimer *refreshTimer = nullptr; ///< Coalesces the re-apply/refresh burst following a batch of deck loads.
    bool shareSelectable = false;
};

#endif // VISUAL_DECK_STORAGE_WIDGET_H
