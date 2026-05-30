#ifndef COCKATRICE_STORAGE_SETTINGS_PAGE_H
#define COCKATRICE_STORAGE_SETTINGS_PAGE_H

#include "abstract_settings_page.h"

#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>

class StorageSettingsPage : public AbstractSettingsPage
{
    Q_OBJECT
public:
    StorageSettingsPage();
    void retranslateUi() override;

private slots:
    void clearDownloadedPicsButtonClicked();
    void clearImageBackupsButtonClicked();
    void clearPixmapCacheButtonClicked();

private:
    QPushButton clearDownloadedPicsButton;
    QPushButton clearBackupsButton;
    QPushButton clearPixmapCacheButton;

    QGroupBox *mpCacheMethodGroupBox;
    QGroupBox *mpNetworkCacheGroupBox;
    QGroupBox *mpImageBackupGroupBox;
    QGroupBox *mpPixmapCacheGroupBox;

    QLabel networkCacheExplainerLabel;
    QLabel imageBackupExplainerLabel;
    QLabel pixmapCacheExplainerLabel;

    QLabel cardPictureLoaderCacheMethodLabel;
    QComboBox *cardPictureLoaderCacheMethodComboBox;
    QLabel networkCacheLabel;
    QSpinBox networkCacheEdit;
    QLabel networkRedirectCacheTtlLabel;
    QSpinBox networkRedirectCacheTtlEdit;
    QSpinBox pixmapCacheEdit;
    QLabel pixmapCacheLabel;
    QLabel localCardImageStorageNamingSchemeLabel;
    QComboBox *localCardImageStorageNamingSchemeComboBox;
};

#endif // COCKATRICE_STORAGE_SETTINGS_PAGE_H
