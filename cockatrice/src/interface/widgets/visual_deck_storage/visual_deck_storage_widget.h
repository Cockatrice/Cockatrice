/**
 * @file visual_deck_storage_widget.h
 * @ingroup VisualDeckStorageWidgets
 */

#ifndef VISUAL_DECK_STORAGE_WIDGET_H
#define VISUAL_DECK_STORAGE_WIDGET_H

#include "visual_deck_storage_model.h"
#include "visual_deck_storage_sort_filter_proxy_model.h"

#include <QHBoxLayout>
#include <QScrollArea>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>

class QLabel;
class QResizeEvent;
class QShowEvent;
class QTimer;
class DeckPreviewColorIdentityFilterWidget;
class VisualDeckStorageFolderDisplayWidget;
class VisualDeckStorageQuickSettingsWidget;
class VisualDeckStorageSearchWidget;
class VisualDeckStorageSortWidget;
class VisualDeckStorageTagFilterWidget;

class VisualDeckStorageWidget final : public QWidget
{
    Q_OBJECT
public:
    explicit VisualDeckStorageWidget(QWidget *parent);
    void refreshIfPossible();
    void retranslateUi();

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

protected:
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    void reapplySortAndFilters();

private:
    QVBoxLayout *layout;
    QWidget *searchAndSortContainer;
    QHBoxLayout *searchAndSortLayout;
    QLabel *databaseLoadIndicator;
    VisualDeckStorageSortWidget *sortWidget;
    VisualDeckStorageSearchWidget *searchWidget;
    DeckPreviewColorIdentityFilterWidget *deckPreviewColorIdentityFilterWidget;
    QToolButton *refreshButton;
    VisualDeckStorageQuickSettingsWidget *quickSettingsWidget;
    QScrollArea *scrollArea;
    VisualDeckStorageFolderDisplayWidget *folderWidget = nullptr;
    VisualDeckStorageModel *storageModel = nullptr;
    VisualDeckStorageSortFilterProxyModel *storageProxyModel = nullptr;
    QTimer *refreshTimer = nullptr; ///< Coalesces the re-apply/refresh burst following a batch of deck loads.
};

#endif // VISUAL_DECK_STORAGE_WIDGET_H
