#include "visual_deck_storage_folder_display_widget.h"

#include "../../../../settings/cache_settings.h"
#include "deck_preview/deck_preview_widget.h"

#include <QDirIterator>
#include <QMouseEvent>

VisualDeckStorageFolderDisplayWidget::VisualDeckStorageFolderDisplayWidget(
    QWidget *parent,
    VisualDeckStorageWidget *_visualDeckStorageWidget,
    QString _filePath,
    bool canBeHidden)
    : QWidget(parent), visualDeckStorageWidget(_visualDeckStorageWidget), filePath(_filePath)
{
    layout = new QVBoxLayout(this);
    setLayout(layout);

    header = new BannerWidget(this, "");
    header->setClickable(canBeHidden);
    layout->addWidget(header);

    container = new QWidget(this);
    containerLayout = new QVBoxLayout(container);
    container->setLayout(containerLayout);

    header->setBuddy(container);

    layout->addWidget(container);

    flowWidget = new FlowWidget(this, Qt::Horizontal, Qt::ScrollBarAlwaysOff, Qt::ScrollBarAlwaysOff);
    containerLayout->addWidget(flowWidget);

    connect(visualDeckStorageWidget, &VisualDeckStorageWidget::tagFilterUpdated, this,
            &VisualDeckStorageFolderDisplayWidget::updateVisibility);
    connect(visualDeckStorageWidget, &VisualDeckStorageWidget::colorFilterUpdated, this,
            &VisualDeckStorageFolderDisplayWidget::updateVisibility);
    connect(visualDeckStorageWidget, &VisualDeckStorageWidget::searchFilterUpdated, this,
            &VisualDeckStorageFolderDisplayWidget::updateVisibility);

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

static QStringList getAllFiles(const QString &filePath)
{
    QStringList allFiles;

    // QDirIterator with QDir::Files ensures only files are listed (no directories)
    QDirIterator it(filePath, QDir::Files);

    while (it.hasNext()) {
        allFiles << it.next(); // Add each file path to the list
    }

    return allFiles;
}

void VisualDeckStorageFolderDisplayWidget::createWidgetsForFiles()
{
    QList<DeckPreviewWidget *> allDecks;
    for (const QString &file : getAllFiles(filePath)) {
        auto *display = new DeckPreviewWidget(flowWidget, visualDeckStorageWidget, file);

        connect(display, &DeckPreviewWidget::deckPreviewClicked, visualDeckStorageWidget,
                &VisualDeckStorageWidget::deckPreviewClickedEvent);
        connect(display, &DeckPreviewWidget::deckPreviewDoubleClicked, visualDeckStorageWidget,
                &VisualDeckStorageWidget::deckPreviewDoubleClickedEvent);
        connect(visualDeckStorageWidget->cardSizeWidget->getSlider(), &QSlider::valueChanged,
                display->bannerCardDisplayWidget, &CardInfoPictureWidget::setScaleFactor);
        display->bannerCardDisplayWidget->setScaleFactor(visualDeckStorageWidget->cardSizeWidget->getSlider()->value());
        allDecks.append(display);
    }

    flowWidget->clearLayout(); // Clear existing widgets in the flow layout

    for (DeckPreviewWidget *deck : allDecks) {
        flowWidget->addWidget(deck);
    }
}

void VisualDeckStorageFolderDisplayWidget::updateVisibility()
{
    bool atLeastOneWidgetVisible = checkVisibility();
    if (atLeastOneWidgetVisible) {
        setVisible(true);
        for (DeckPreviewWidget *display : flowWidget->findChildren<DeckPreviewWidget *>()) {
            display->updateVisibility();
        }
        for (VisualDeckStorageFolderDisplayWidget *subFolder : findChildren<VisualDeckStorageFolderDisplayWidget *>()) {
            subFolder->updateVisibility();
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
    for (const QString &dir : getAllSubFolders(filePath)) {
        auto *display = new VisualDeckStorageFolderDisplayWidget(this, visualDeckStorageWidget, dir, true);
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