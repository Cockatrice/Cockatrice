#include "visual_deck_storage_folder_display_widget.h"

#include "../../../../settings/cache_settings.h"
#include "deck_preview/deck_preview_widget.h"

#include <QDirIterator>

VisualDeckStorageFolderDisplayWidget::VisualDeckStorageFolderDisplayWidget(
    QWidget *parent,
    VisualDeckStorageWidget *_visualDeckStorageWidget,
    QString _filePath)
    : QWidget(parent), visualDeckStorageWidget(_visualDeckStorageWidget), filePath(_filePath)
{
    layout = new QVBoxLayout(this);
    setLayout(layout);

    QString bannerText = "Deck Storage";
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

    header = new BannerWidget(this, bannerText);
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
}

void VisualDeckStorageFolderDisplayWidget::createWidgetsForFiles()
{
    QList<DeckPreviewWidget *> allDecks;
    for (const QString &file : getAllFiles()) {
        auto *display = new DeckPreviewWidget(visualDeckStorageWidget, file);

        connect(display, &DeckPreviewWidget::deckPreviewClicked, visualDeckStorageWidget,
                &VisualDeckStorageWidget::deckPreviewClickedEvent);
        connect(display, &DeckPreviewWidget::deckPreviewDoubleClicked, visualDeckStorageWidget,
                &VisualDeckStorageWidget::deckPreviewDoubleClickedEvent);
        connect(visualDeckStorageWidget->cardSizeWidget->getSlider(), &QSlider::valueChanged, display->bannerCardDisplayWidget,
                &CardInfoPictureWidget::setScaleFactor);
        connect(display, &DeckPreviewWidget::visibilityUpdated, this, &VisualDeckStorageFolderDisplayWidget::updateVisibility);
        display->bannerCardDisplayWidget->setScaleFactor(visualDeckStorageWidget->cardSizeWidget->getSlider()->value());
        allDecks.append(display);
    }

    /*auto filteredByColorIdentity =
        deckPreviewColorIdentityFilterWidget->filterWidgets(sortWidget->filterFiles(allDecks));
    auto filteredByTags = tagFilterWidget->filterDecksBySelectedTags(filteredByColorIdentity);
    auto filteredFiles = searchWidget->filterFiles(filteredByTags, searchWidget->getSearchText());
    */

    flowWidget->clearLayout(); // Clear existing widgets in the flow layout

    for (DeckPreviewWidget *deck : allDecks) {
        flowWidget->addWidget(deck);
    }

    //emit bannerCardsRefreshed();
}

void VisualDeckStorageFolderDisplayWidget::updateVisibility()
{
    bool atLeastOneWidgetVisible = false;
    if (flowWidget) {
        // Iterate through all DeckPreviewWidgets
        for (DeckPreviewWidget *display : flowWidget->findChildren<DeckPreviewWidget *>()) {
            // Get tags from each DeckPreviewWidget
            if (display->isVisible()) {
                atLeastOneWidgetVisible = true;
            }
        }
    }
    if (atLeastOneWidgetVisible) {
        //setVisible(true);
    } else {
        //setVisible(false);
    }
}

void VisualDeckStorageFolderDisplayWidget::createWidgetsForFolders()
{
    for (const QString &dir : getAllSubFolders()) {
        auto *display = new VisualDeckStorageFolderDisplayWidget(this, visualDeckStorageWidget, dir);
        containerLayout->addWidget(display);
    }
}

QStringList VisualDeckStorageFolderDisplayWidget::gatherAllTagsFromFlowWidget() const
{
    QStringList allTags;

    if (flowWidget) {
        // Iterate through all DeckPreviewWidgets
        for (DeckPreviewWidget *display : flowWidget->findChildren<DeckPreviewWidget *>()) {
            // Get tags from each DeckPreviewWidget
            QStringList tags = display->deckLoader->getTags();

            // Add tags to the list while avoiding duplicates
            allTags.append(tags);
        }
    }

    // Remove duplicates by calling 'removeDuplicates'
    allTags.removeDuplicates();

    return allTags;
}

QStringList VisualDeckStorageFolderDisplayWidget::getAllFiles() const
{
    QStringList allFiles;

    // QDirIterator with QDir::Files and QDir::NoSymLinks ensures only files are listed (no directories or symlinks)
    QDirIterator it(filePath, QDir::Files | QDir::NoSymLinks);

    while (it.hasNext()) {
        allFiles << it.next(); // Add each file path to the list
    }

    return allFiles;
}

QStringList VisualDeckStorageFolderDisplayWidget::getAllSubFolders() const
{
    QStringList allFolders;

    // QDirIterator with QDir::Files and QDir::NoSymLinks ensures only files are listed (no directories or symlinks)
    QDirIterator it(filePath, QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot);

    while (it.hasNext()) {
        allFolders << it.next(); // Add each file path to the list
    }

    return allFolders;
}