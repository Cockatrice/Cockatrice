/**
 * @file visual_deck_storage_widget.h
 * @ingroup VisualDeckStorageWidgets
 * @brief TODO: Document this.
 */

#ifndef VISUAL_DECK_STORAGE_WIDGET_H
#define VISUAL_DECK_STORAGE_WIDGET_H

#include <QWidget>

class VisualDeckStorageQuickSettingsWidget;
struct LoadedDeck;
class QSpinBox;
class QVBoxLayout;
class QHBoxLayout;
class QLabel;
class QToolButton;
class QScrollArea;
class DeckListModel;
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

    [[nodiscard]] const VisualDeckStorageQuickSettingsWidget *settings() const;

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
    void openDeckEditor(const LoadedDeck &deck);

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
