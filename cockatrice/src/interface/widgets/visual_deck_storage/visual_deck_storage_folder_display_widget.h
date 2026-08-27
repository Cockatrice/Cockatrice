/**
 * @file visual_deck_storage_folder_display_widget.h
 * @ingroup VisualDeckStorageWidgets
 * @brief Renders the decks of one folder of the Visual Deck Storage.
 *
 * This is a pure view: it keeps one persistent DeckPreviewWidget alive per deck
 * in its folder, and shows or hides those widgets according to each row's
 * FilterMatchRole in the VisualDeckStorageSortFilterProxyModel. Subfolders are
 * shown as nested VisualDeckStorageFolderDisplayWidgets when the "show folders"
 * setting is enabled.
 *
 * Reconciling runs as a time-budgeted chunked pass that yields to the event loop
 * between chunks, so scanning a large collection never stalls the ui thread.
 */

#ifndef VISUAL_DECK_STORAGE_FOLDER_DISPLAY_WIDGET_H
#define VISUAL_DECK_STORAGE_FOLDER_DISPLAY_WIDGET_H

#include <QHash>
#include <QSet>
#include <QString>
#include <QWidget>

class BannerWidget;
class DeckPreviewWidget;
class FlowWidget;
class QTimer;
class QVBoxLayout;
class VisualDeckStorageWidget;

class VisualDeckStorageFolderDisplayWidget : public QWidget
{
    Q_OBJECT
public:
    VisualDeckStorageFolderDisplayWidget(QWidget *parent,
                                         VisualDeckStorageWidget *visualDeckStorageWidget,
                                         const QString &folderPath,
                                         bool canBeHidden,
                                         bool showFolders);

public slots:
    /**
     * @brief Starts a new chunked reconcile pass that re-reads the proxy and rebuilds
     * the deck previews and subfolder widgets to match.
     */
    void reconcile();

    /**
     * @brief Coalesces proxy change signals (a burst of deck loads or filter
     * invalidations) into a single reconcile on the next event loop turn.
     */
    void scheduleReconcile();
    void updateShowFolders(bool enabled);

signals:
    /**
     * @brief Emitted whenever this folder's visible content changes, so parent
     * folders can re-evaluate their own visibility.
     */
    void contentVisibilityChanged();

private:
    void beginDeckPass();
    void continueDeckPass();
    void finishDeckPass();
    [[nodiscard]] DeckPreviewWidget *createDeckPreviewWidget(const QString &filePath);
    void createSubFolderWidgets();
    void refreshVisibility();
    [[nodiscard]] bool hasContent() const;
    [[nodiscard]] QStringList childFolderPaths() const;

    /**
     * @brief The maximum time in milliseconds spent creating deck previews per event loop turn.
     *
     * Creating all previews of a large folder at once blocks the ui thread for hundreds of
     * milliseconds, so the pass is split into chunks that yield to the event loop instead.
     */
    static constexpr int DECK_PASS_TIME_BUDGET_MS = 20;

    bool showFolders;
    QString folderPath;       ///< Path relative to the deck folder, empty for the root folder.
    int visibleDeckCount = 0; ///< The number of this folder's deck previews not filtered out.
    QVBoxLayout *layout;
    QWidget *container;
    QVBoxLayout *containerLayout;
    FlowWidget *flowWidget;
    BannerWidget *header;
    VisualDeckStorageWidget *visualDeckStorageWidget;
    QHash<QString, DeckPreviewWidget *> deckWidgets;                         ///< Deck file path -> preview widget.
    QHash<QString, VisualDeckStorageFolderDisplayWidget *> subFolderWidgets; ///< Folder path -> subfolder widget.
    QTimer *reconcileTimer = nullptr;                                        ///< Coalesces proxy change bursts.
    QStringList lastOrderedFilePaths; ///< The deck order last applied to the flow layout.

    /// Whether a chunked reconcile pass is currently running.
    bool deckPassActive = false;
    /// Set when the model changes mid-pass. Discards progress and restarts the scan once
    /// the current chunk finishes so the pass always converges on the latest model state.
    bool deckPassRestartRequested = false;
    /// Whether the first reconcile pass has run to completion at least once.
    bool initialPassCompleted = false;
    int deckPassRow = 0;                ///< Next proxy row to scan in the active pass.
    QSet<QString> deckPassPresentPaths; ///< File paths seen so far in the active pass.
};

#endif // VISUAL_DECK_STORAGE_FOLDER_DISPLAY_WIDGET_H
