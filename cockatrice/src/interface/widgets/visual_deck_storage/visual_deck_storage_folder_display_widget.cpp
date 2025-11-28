#include "visual_deck_storage_folder_display_widget.h"

#include "../../../client/settings/cache_settings.h"
#include "deck_preview/deck_preview_widget.h"
#include "visual_deck_storage_widget.h"

#include <QDirIterator>
#include <QMouseEvent>

VisualDeckStorageFolderDisplayWidget::VisualDeckStorageFolderDisplayWidget(
    QWidget *parent,
    VisualDeckStorageWidget *_visualDeckStorageWidget,
    QString _filePath,
    bool canBeHidden,
    bool _showFolders)
    : QWidget(parent), showFolders(_showFolders), visualDeckStorageWidget(_visualDeckStorageWidget), filePath(_filePath)
{
    layout = new QVBoxLayout(this);
    setLayout(layout);

    header = new BannerWidget(this, "");
    header->setClickable(canBeHidden);
    header->setHidden(!showFolders);
    layout->addWidget(header);

    container = new QWidget(this);
    containerLayout = new QVBoxLayout(container);
    container->setLayout(containerLayout);

    header->setBuddy(container);

    layout->addWidget(container);

    flowWidget = new FlowWidget(this, Qt::Horizontal, Qt::ScrollBarAlwaysOff, Qt::ScrollBarAlwaysOff);
    containerLayout->addWidget(flowWidget);

    createWidgetsForFiles();
    createWidgetsForFolders();

    refreshUi();
}

void VisualDeckStorageFolderDisplayWidget::refreshUi()
{
    QString bannerText = tr("Deck Storage");
    QString deckPath = SettingsCache::instance().getDeckPath();
    if (filePath != deckPath) {
        QString relativePath = filePath;

        if (filePath.startsWith(deckPath)) {
            relativePath = filePath.mid(deckPath.length()); // Remove the deckPath prefix
            if (relativePath.startsWith('/')) {
                relativePath.remove(0, 1); // Remove leading '/' if it exists
            }
        }

        bannerText = relativePath;
    }
    header->setText(bannerText);
}

/**
 * Gets all files in the directory that have an accepted decklist file extension
 *
 * @param filePath The directory to search through
 * @param recursive Whether to search through subdirectories
 */
static QStringList getAllFiles(const QString &filePath, bool recursive)
{
    QStringList allFiles;

    // QDirIterator with QDir::Files ensures only files are listed (no directories)
    auto flags =
        recursive ? QDirIterator::Subdirectories | QDirIterator::FollowSymlinks : QDirIterator::NoIteratorFlags;
    QDirIterator it(filePath, DeckLoader::ACCEPTED_FILE_EXTENSIONS, QDir::Files, flags);

    while (it.hasNext()) {
        allFiles << it.next(); // Add each file path to the list
    }

    return allFiles;
}

void VisualDeckStorageFolderDisplayWidget::createWidgetsForFiles()
{
    QList<DeckPreviewWidget *> allDecks;
    for (const QString &file : getAllFiles(filePath, !showFolders)) {
        auto *display = new DeckPreviewWidget(flowWidget, visualDeckStorageWidget, file);

        connect(display, &DeckPreviewWidget::deckLoadRequested, visualDeckStorageWidget,
                &VisualDeckStorageWidget::deckLoadRequested);
        connect(display, &DeckPreviewWidget::openDeckEditor, visualDeckStorageWidget,
                &VisualDeckStorageWidget::openDeckEditor);
        connect(visualDeckStorageWidget->settings(), &VisualDeckStorageQuickSettingsWidget::cardSizeChanged,
                display->bannerCardDisplayWidget, &CardInfoPictureWidget::setScaleFactor);
        display->bannerCardDisplayWidget->setScaleFactor(visualDeckStorageWidget->settings()->getCardSize());
        allDecks.append(display);
    }

    flowWidget->clearLayout(); // Clear existing widgets in the flow layout

    for (DeckPreviewWidget *deck : allDecks) {
        flowWidget->addWidget(deck);
    }
}

/**
 * Updates the visibility of this folder and all its DeckPreviewWidgets
 *
 * @param recursive Also update the visibility of all subfolders and their DeckPreviewWidgets.
 */
void VisualDeckStorageFolderDisplayWidget::updateVisibility(bool recursive)
{
    bool atLeastOneWidgetVisible = checkVisibility();
    if (atLeastOneWidgetVisible) {
        setVisible(true);
        for (DeckPreviewWidget *display : flowWidget->findChildren<DeckPreviewWidget *>()) {
            display->updateVisibility();
        }
        if (recursive) {
            for (auto *subFolder : findChildren<VisualDeckStorageFolderDisplayWidget *>()) {
                subFolder->updateVisibility(false);
            }
        }
    } else {
        setVisible(false);
    }
}

bool VisualDeckStorageFolderDisplayWidget::checkVisibility()
{
    bool atLeastOneWidgetVisible = false;
    if (flowWidget) {
        // Iterate through all DeckPreviewWidgets
        for (DeckPreviewWidget *display : flowWidget->findChildren<DeckPreviewWidget *>()) {
            if (display->checkVisibility()) {
                atLeastOneWidgetVisible = true;
            }
        }
    }
    for (VisualDeckStorageFolderDisplayWidget *subFolder : findChildren<VisualDeckStorageFolderDisplayWidget *>()) {
        if (subFolder->checkVisibility()) {
            atLeastOneWidgetVisible = true;
        }
    }
    return atLeastOneWidgetVisible;
}

static QStringList getAllSubFolders(const QString &filePath)
{
    QStringList allFolders;

    // QDirIterator with QDir::Files ensures only files are listed (no directories)
    QDirIterator it(filePath, QDir::Dirs | QDir::NoDotAndDotDot);

    while (it.hasNext()) {
        allFolders << it.next(); // Add each file path to the list
    }

    return allFolders;
}

void VisualDeckStorageFolderDisplayWidget::createWidgetsForFolders()
{
    if (!showFolders) {
        return;
    }

    for (const QString &dir : getAllSubFolders(filePath)) {
        auto *display = new VisualDeckStorageFolderDisplayWidget(this, visualDeckStorageWidget, dir, true, showFolders);
        containerLayout->addWidget(display);
    }
}

void VisualDeckStorageFolderDisplayWidget::updateShowFolders(bool enabled)
{
    showFolders = enabled;

    if (!showFolders) {
        flattenFolderStructure();
    } else {
        // if setting was switched from disabled to enabled, we assume that there aren't any existing subfolders
        createWidgetsForFiles();
        createWidgetsForFolders();
    }

    header->setHidden(!showFolders);
}

/**
 * Steals all DeckPreviewWidgets from this widget's nested subfolders, and deletes those subfolders
 */
void VisualDeckStorageFolderDisplayWidget::flattenFolderStructure()
{
    for (auto *subFolder : findChildren<VisualDeckStorageFolderDisplayWidget *>()) {
        // steal all DeckPreviewWidgets from the subfolder
        for (auto *deck : subFolder->getFlowWidget()->findChildren<DeckPreviewWidget *>()) {
            flowWidget->addWidget(deck);
        }

        // delete the subfolder
        subFolder->deleteLater();
    }
}

QStringList VisualDeckStorageFolderDisplayWidget::gatherAllTagsFromFlowWidget() const
{
    QStringList allTags;

    if (flowWidget) {
        // Iterate through all DeckPreviewWidgets
        for (DeckPreviewWidget *display : flowWidget->findChildren<DeckPreviewWidget *>()) {
            // Get tags from each DeckPreviewWidget
            QStringList tags = display->deckLoader->getDeckList()->getTags();

            // Add tags to the list while avoiding duplicates
            allTags.append(tags);
        }
    }

    // Remove duplicates by calling 'removeDuplicates'
    allTags.removeDuplicates();

    return allTags;
}