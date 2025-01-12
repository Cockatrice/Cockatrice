#include "visual_deck_storage_widget.h"

#include "../../../../deck/deck_loader.h"
#include "../../../../game/cards/card_database_manager.h"
#include "../../../../settings/cache_settings.h"
#include "deck_preview/deck_preview_widget.h"
#include "visual_deck_storage_search_widget.h"
#include "visual_deck_storage_sort_widget.h"

#include <QComboBox>
#include <QDirIterator>
#include <QMouseEvent>
#include <QVBoxLayout>

VisualDeckStorageWidget::VisualDeckStorageWidget(QWidget *parent) : QWidget(parent)
{
    deckListModel = new DeckListModel(this);
    deckListModel->setObjectName("visualDeckModel");

    layout = new QVBoxLayout();
    setLayout(layout);

    searchAndSortLayout = new QHBoxLayout();

    sortWidget = new VisualDeckStorageSortWidget(this);
    searchWidget = new VisualDeckStorageSearchWidget(this);
    deckPreviewColorIdentityFilterWidget = new DeckPreviewColorIdentityFilterWidget(this);

    searchAndSortLayout->addWidget(sortWidget);
    searchAndSortLayout->addWidget(searchWidget);
    layout->addLayout(searchAndSortLayout);
    layout->addWidget(deckPreviewColorIdentityFilterWidget);

    flowWidget = new FlowWidget(this, Qt::ScrollBarAlwaysOff, Qt::ScrollBarAsNeeded);
    layout->addWidget(flowWidget);

    cardSizeWidget = new CardSizeWidget(this, flowWidget, SettingsCache::instance().getVisualDeckStorageCardSize());
    layout->addWidget(cardSizeWidget);
}

void VisualDeckStorageWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    updateSortOrder();
}

void VisualDeckStorageWidget::updateSortOrder()
{
    refreshBannerCards(); // Refresh the banner cards with the new sort order
}

void VisualDeckStorageWidget::deckPreviewClickedEvent(QMouseEvent *event, DeckPreviewWidget *instance)
{
    emit deckPreviewClicked(event, instance);
}

void VisualDeckStorageWidget::deckPreviewDoubleClickedEvent(QMouseEvent *event, DeckPreviewWidget *instance)
{
    emit deckPreviewDoubleClicked(event, instance);
}

void VisualDeckStorageWidget::refreshBannerCards()
{
    QStringList allFiles;
    QList<DeckPreviewWidget *> allDecks;

    // QDirIterator with QDir::Files and QDir::NoSymLinks ensures only files are listed (no directories or symlinks)
    QDirIterator it(SettingsCache::instance().getDeckPath(), QDir::Files | QDir::NoSymLinks,
                    QDirIterator::Subdirectories);

    while (it.hasNext()) {
        allFiles << it.next(); // Add each file path to the list
    }

    foreach (const QString &file, allFiles) {
        auto *display = new DeckPreviewWidget(flowWidget, file);

        connect(display, &DeckPreviewWidget::deckPreviewClicked, this,
                &VisualDeckStorageWidget::deckPreviewClickedEvent);
        connect(display, &DeckPreviewWidget::deckPreviewDoubleClicked, this,
                &VisualDeckStorageWidget::deckPreviewDoubleClickedEvent);
        connect(cardSizeWidget->getSlider(), &QSlider::valueChanged, display->bannerCardDisplayWidget,
                &CardInfoPictureWidget::setScaleFactor);
        display->bannerCardDisplayWidget->setScaleFactor(cardSizeWidget->getSlider()->value());
        allDecks.append(display);
    }

    auto filteredByColorIdentity =
        deckPreviewColorIdentityFilterWidget->filterWidgets(sortWidget->filterFiles(allDecks));
    auto filteredFiles = searchWidget->filterFiles(filteredByColorIdentity, searchWidget->getSearchText());

    flowWidget->clearLayout(); // Clear existing widgets in the flow layout

    foreach (DeckPreviewWidget *deck, filteredFiles) {
        flowWidget->addWidget(deck);
    }
}
