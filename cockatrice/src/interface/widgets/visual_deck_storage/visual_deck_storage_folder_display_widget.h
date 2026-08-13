/**
 * @file visual_deck_storage_folder_display_widget.h
 * @ingroup VisualDeckStorageWidgets
 * @brief Renders the decks of one folder of the Visual Deck Storage.
 *
 * This is a pure view: it reads the accepted rows of the
 * VisualDeckStorageSortFilterProxyModel whose folder matches this widget's
 * folder path, and keeps a set of child DeckPreviewWidgets in sync with those
 * rows. Subfolders are shown as nested VisualDeckStorageFolderDisplayWidgets
 * when the "show folders" setting is enabled.
 */

#ifndef VISUAL_DECK_STORAGE_FOLDER_DISPLAY_WIDGET_H
#define VISUAL_DECK_STORAGE_FOLDER_DISPLAY_WIDGET_H

#include <QHash>
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
     * @brief Re-reads the proxy and rebuilds the deck previews and subfolder
     * widgets to match.
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
    void reconcileDecks();
    void createSubFolderWidgets();
    void refreshVisibility();
    [[nodiscard]] bool hasContent() const;
    [[nodiscard]] QStringList childFolderPaths() const;

    bool showFolders;
    QString folderPath; ///< Path relative to the deck folder; empty for the root folder.
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
};

#endif // VISUAL_DECK_STORAGE_FOLDER_DISPLAY_WIDGET_H
