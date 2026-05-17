#include "storage_settings_page.h"

#include "../../../client/settings/cache_settings.h"
#include "../interface/card_picture_loader/card_picture_loader.h"

#include <QDir>
#include <QGridLayout>
#include <QMessageBox>

StorageSettingsPage::StorageSettingsPage()
{
    auto *lpNetworkCacheGrid = new QGridLayout;
    auto *lpImageBackupGrid = new QGridLayout;
    auto *lpPixmapCacheGrid = new QGridLayout;

    networkCacheExplainerLabel.setWordWrap(true);
    imageBackupExplainerLabel.setWordWrap(true);
    pixmapCacheExplainerLabel.setWordWrap(true);

    connect(&clearDownloadedPicsButton, &QPushButton::clicked, this,
            &StorageSettingsPage::clearDownloadedPicsButtonClicked);

    connect(&clearPixmapCacheButton, &QPushButton::clicked, this, &StorageSettingsPage::clearPixmapCacheButtonClicked);

    // pixmap cache
    pixmapCacheEdit.setMinimum(PIXMAPCACHE_SIZE_MIN);
    // 2047 is the max value to avoid overflowing of QPixmapCache::setCacheLimit(int size)
    pixmapCacheEdit.setMaximum(PIXMAPCACHE_SIZE_MAX);
    pixmapCacheEdit.setSingleStep(64);
    pixmapCacheEdit.setValue(SettingsCache::instance().getPixmapCacheSize());
    pixmapCacheEdit.setSuffix(" MB");

    // Caching method

    cardPictureLoaderCacheMethodComboBox = new QComboBox;
    for (auto method : CardPictureLoaderCacheMethod::methods()) {
        cardPictureLoaderCacheMethodComboBox->addItem(method.displayName, static_cast<int>(method.id));
    }

    int currentCacheMethod = static_cast<int>(SettingsCache::instance().getCardPictureLoaderCacheMethod());

    int currentIndex = cardPictureLoaderCacheMethodComboBox->findData(currentCacheMethod);
    if (currentIndex >= 0) {
        cardPictureLoaderCacheMethodComboBox->setCurrentIndex(currentIndex);
    }

    connect(cardPictureLoaderCacheMethodComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this,
            [this](int index) {
                auto cacheMethod = static_cast<CardPictureLoaderCacheMethod::CacheMethod>(
                    cardPictureLoaderCacheMethodComboBox->itemData(index).toInt());

                bool useNetworkCache = (cacheMethod == CardPictureLoaderCacheMethod::CacheMethod::NETWORK_CACHE);

                if (useNetworkCache) {
                    clearImageBackupsButtonClicked();
                } else {
                    clearDownloadedPicsButtonClicked();
                }

                mpNetworkCacheGroupBox->setEnabled(useNetworkCache);
                mpImageBackupGroupBox->setEnabled(!useNetworkCache);

                SettingsCache::instance().setCardImageCacheMethod(cacheMethod);
            });

    // Network Cache

    networkCacheEdit.setMinimum(NETWORK_CACHE_SIZE_MIN);
    networkCacheEdit.setMaximum(NETWORK_CACHE_SIZE_MAX);
    networkCacheEdit.setSingleStep(1);
    networkCacheEdit.setValue(SettingsCache::instance().getNetworkCacheSizeInMB());
    networkCacheEdit.setSuffix(" MB");

    networkRedirectCacheTtlEdit.setMinimum(NETWORK_REDIRECT_CACHE_TTL_MIN);
    networkRedirectCacheTtlEdit.setMaximum(NETWORK_REDIRECT_CACHE_TTL_MAX);
    networkRedirectCacheTtlEdit.setSingleStep(1);
    networkRedirectCacheTtlEdit.setValue(SettingsCache::instance().getRedirectCacheTtl());

    // Image Backup
    localCardImageStorageNamingSchemeComboBox = new QComboBox;
    for (const auto &scheme : CardPictureLoaderLocalSchemes::exportSchemes()) {
        localCardImageStorageNamingSchemeComboBox->addItem(scheme.displayName, static_cast<int>(scheme.id));
    }

    int current = static_cast<int>(SettingsCache::instance().getLocalCardImageStorageNamingScheme());

    int index = localCardImageStorageNamingSchemeComboBox->findData(current);
    if (index >= 0) {
        localCardImageStorageNamingSchemeComboBox->setCurrentIndex(index);
    }

    connect(localCardImageStorageNamingSchemeComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this,
            [this](int index) {
                auto scheme = static_cast<CardPictureLoaderLocalSchemes::NamingScheme>(
                    localCardImageStorageNamingSchemeComboBox->itemData(index).toInt());
                SettingsCache::instance().setLocalCardImageStorageNamingScheme(scheme);
            });

    connect(&clearBackupsButton, &QPushButton::clicked, this, &StorageSettingsPage::clearImageBackupsButtonClicked);

    auto cacheMethodLayout = new QHBoxLayout;
    cacheMethodLayout->addWidget(&cardPictureLoaderCacheMethodLabel);
    cacheMethodLayout->addWidget(cardPictureLoaderCacheMethodComboBox);

    auto networkCacheLayout = new QHBoxLayout;
    networkCacheLayout->addWidget(&clearDownloadedPicsButton);
    networkCacheLayout->addStretch();
    networkCacheLayout->addWidget(&networkCacheLabel);
    networkCacheLayout->addWidget(&networkCacheEdit);

    auto networkRedirectCacheLayout = new QHBoxLayout;
    networkRedirectCacheLayout->addStretch();
    networkRedirectCacheLayout->addWidget(&networkRedirectCacheTtlLabel);
    networkRedirectCacheLayout->addWidget(&networkRedirectCacheTtlEdit);

    auto pixmapCacheLayout = new QHBoxLayout;
    pixmapCacheLayout->addWidget(&clearPixmapCacheButton);
    pixmapCacheLayout->addStretch();
    pixmapCacheLayout->addWidget(&pixmapCacheLabel);
    pixmapCacheLayout->addWidget(&pixmapCacheEdit);

    lpNetworkCacheGrid->addWidget(&networkCacheExplainerLabel, 0, 0);
    lpNetworkCacheGrid->addLayout(networkCacheLayout, 1, 0);
    lpNetworkCacheGrid->addLayout(networkRedirectCacheLayout, 2, 0);

    // Image Backup Layout
    lpImageBackupGrid->addWidget(&imageBackupExplainerLabel, 0, 0, 1, 2);
    lpImageBackupGrid->addWidget(&localCardImageStorageNamingSchemeLabel, 1, 0);
    lpImageBackupGrid->addWidget(localCardImageStorageNamingSchemeComboBox, 1, 1);
    lpImageBackupGrid->addWidget(&clearBackupsButton, 2, 0);

    lpPixmapCacheGrid->addWidget(&pixmapCacheExplainerLabel, 0, 0);
    lpPixmapCacheGrid->addLayout(pixmapCacheLayout, 1, 0);

    connect(&pixmapCacheEdit, qOverload<int>(&QSpinBox::valueChanged), &SettingsCache::instance(),
            &SettingsCache::setPixmapCacheSize);
    connect(&networkCacheEdit, qOverload<int>(&QSpinBox::valueChanged), &SettingsCache::instance(),
            &SettingsCache::setNetworkCacheSizeInMB);
    connect(&networkRedirectCacheTtlEdit, qOverload<int>(&QSpinBox::valueChanged), &SettingsCache::instance(),
            &SettingsCache::setNetworkRedirectCacheTtl);

    mpCacheMethodGroupBox = new QGroupBox;
    mpCacheMethodGroupBox->setLayout(cacheMethodLayout);

    mpNetworkCacheGroupBox = new QGroupBox;
    mpNetworkCacheGroupBox->setLayout(lpNetworkCacheGrid);

    mpImageBackupGroupBox = new QGroupBox;
    mpImageBackupGroupBox->setLayout(lpImageBackupGrid);

    mpPixmapCacheGroupBox = new QGroupBox;
    mpPixmapCacheGroupBox->setLayout(lpPixmapCacheGrid);

    auto *lpMainLayout = new QVBoxLayout;

    lpMainLayout->addWidget(mpCacheMethodGroupBox);
    lpMainLayout->addWidget(mpNetworkCacheGroupBox);
    lpMainLayout->addWidget(mpImageBackupGroupBox);
    lpMainLayout->addWidget(mpPixmapCacheGroupBox);
    lpMainLayout->addStretch();

    setLayout(lpMainLayout);

    bool useNetworkCache = SettingsCache::instance().getCardPictureLoaderCacheMethod() ==
                           CardPictureLoaderCacheMethod::CacheMethod::NETWORK_CACHE;

    mpNetworkCacheGroupBox->setEnabled(useNetworkCache);
    mpImageBackupGroupBox->setEnabled(!useNetworkCache);

    connect(&SettingsCache::instance(), &SettingsCache::langChanged, this, &StorageSettingsPage::retranslateUi);
    retranslateUi();
}

void StorageSettingsPage::clearDownloadedPicsButtonClicked()
{
    CardPictureLoader::clearNetworkCache();
    CardPictureLoader::clearPixmapCache();
    QMessageBox::information(this, tr("Success"), tr("Cached card pictures have been reset."));
}

void StorageSettingsPage::clearImageBackupsButtonClicked()
{
    QString picsPath = SettingsCache::instance().getPicsPath() + "/downloadedPics";

    QDir dir(picsPath);
    bool success = dir.removeRecursively();

    CardPictureLoader::clearPixmapCache();

    if (success) {
        QMessageBox::information(this, tr("Success"), tr("Downloaded card pictures have been reset."));
    } else {
        QMessageBox::critical(this, tr("Error"), tr("One or more downloaded card pictures could not be cleared."));
    }
}

void StorageSettingsPage::clearPixmapCacheButtonClicked()
{
    CardPictureLoader::clearPixmapCache();
    QMessageBox::information(this, tr("Success"), tr("In-memory (currently loaded) card pictures have been reset."));
}

void StorageSettingsPage::retranslateUi()
{
    cardPictureLoaderCacheMethodLabel.setText(tr("Card Picture Loader Caching Method:"));

    networkCacheExplainerLabel.setText(
        tr("The network cache is the preferred way of storing images. Downloaded images "
           "are stored here until the size of the cache exceeds the configured size. Cockatrice automatically monitors "
           "this cache and deletes the least recently seen card images to ensure the cache does not exceed the "
           "configured size."));
    imageBackupExplainerLabel.setText(
        tr("Writing card images directly to a folder on your hard drive is another way "
           "of storing images. This does not change how Cockatrice accesses or downloads "
           "images. Cockatrice will NOT automatically monitor and clear this folder, so if you enable this option, it "
           "is up to you to ensure sufficient available space. It should also be noted that if a provider outage "
           "causes you to download the wrong picture (i.e. wrong printing) you will be stuck with it until you "
           "manually delete the file, as opposed to using the network cache, which automatically rotates and thus "
           "correct errors after a while."));
    pixmapCacheExplainerLabel.setText(
        tr("This is the in-memory picture cache used by the application at runtime. It determines how much memory "
           "(RAM) Cockatrice can use before it has to fetch card images from the hard disk again. Increasing this will "
           "allow more card images to be displayed at once but shouldn't be necessary. Clearing this will make "
           "Cockatrice reload all images from the network cache or the disk."));

    clearDownloadedPicsButton.setText(tr("Delete Cached Images"));
    clearBackupsButton.setText(tr("Delete Saved Images"));
    clearPixmapCacheButton.setText(tr("Clear In-Memory Images"));

    mpCacheMethodGroupBox->setTitle(tr("Card Picture Loader Cache Method"));
    mpNetworkCacheGroupBox->setTitle(tr("Network Cache"));
    mpImageBackupGroupBox->setTitle(tr("Filesystem"));
    mpPixmapCacheGroupBox->setTitle(tr("In-Memory Picture Cache"));

    networkCacheLabel.setText(tr("Network Cache Size:"));
    networkCacheEdit.setToolTip(tr("On-disk cache for downloaded pictures"));
    networkRedirectCacheTtlLabel.setText(tr("Redirect Cache TTL:"));
    networkRedirectCacheTtlEdit.setToolTip(tr("How long cached redirects for urls are valid for."));
    pixmapCacheLabel.setText(tr("Picture Cache Size:"));
    pixmapCacheEdit.setToolTip(tr("In-memory cache for pictures not currently on screen"));
    localCardImageStorageNamingSchemeLabel.setText(tr("Naming scheme:"));

    networkRedirectCacheTtlEdit.setSuffix(" " + tr("Day(s)"));
}