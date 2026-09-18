#include "deck_editor_settings_page.h"

#include "../../../client/settings/cache_settings.h"
#include "../../pixel_map_generator.h"
#include "update/card_spoiler/spoiler_background_updater.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QSet>
#include <QToolBar>
#include <QUrl>
#include <libcockatrice/settings/download_settings.h>
#include <libcockatrice/settings/paths_settings.h>
#include <libcockatrice/settings/personal_settings.h>
#include <libcockatrice/utility/macros.h>

DeckEditorSettingsPage::DeckEditorSettingsPage()
{
    picDownloadCheckBox.setChecked(SettingsCache::instance().downloads().getPicDownload());
    connect(&picDownloadCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance().downloads(),
            &DownloadSettings::setPicDownload);

    urlLinkLabel.setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    urlLinkLabel.setOpenExternalLinks(true);

    connect(&resetDownloadURLs, &QPushButton::clicked, this, &DeckEditorSettingsPage::resetDownloadedURLsButtonClicked);

    auto *lpGeneralGrid = new QGridLayout;
    auto *lpSpoilerGrid = new QGridLayout;

    mcDownloadSpoilersCheckBox.setChecked(SettingsCache::instance().downloads().getDownloadSpoilersStatus());

    mpSpoilerSavePathLineEdit = new QLineEdit(SettingsCache::instance().getSpoilerCardDatabasePath());
    mpSpoilerSavePathLineEdit->setReadOnly(true);
    mpSpoilerPathButton = new QPushButton("...");
    connect(mpSpoilerPathButton, &QPushButton::clicked, this, &DeckEditorSettingsPage::spoilerPathButtonClicked);

    updateNowButton = new QPushButton;
    updateNowButton->setFixedWidth(150);
    connect(updateNowButton, &QPushButton::clicked, this, &DeckEditorSettingsPage::updateSpoilers);

    // Update the GUI depending on if the box is ticked or not
    setSpoilersEnabled(mcDownloadSpoilersCheckBox.isChecked());

    urlList = new QListWidget;
    urlList->setSelectionMode(QAbstractItemView::SingleSelection);
    urlList->setAlternatingRowColors(true);
    urlList->setDragEnabled(true);
    urlList->setDragDropMode(QAbstractItemView::InternalMove);
    connect(urlList->model(), &QAbstractItemModel::rowsMoved, this, &DeckEditorSettingsPage::urlListChanged);

    for (const QString &url : SettingsCache::instance().downloads().getAllURLs()) {
        addUrlItem(url);
    }

    aAdd = new QAction(this);
    aAdd->setIcon(themePixmap(QStringLiteral("icons/increment")));
    connect(aAdd, &QAction::triggered, this, &DeckEditorSettingsPage::actAddURL);

    aEdit = new QAction(this);
    aEdit->setIcon(themePixmap(QStringLiteral("icons/pencil")));
    connect(aEdit, &QAction::triggered, this, &DeckEditorSettingsPage::actEditURL);

    aRemove = new QAction(this);
    aRemove->setIcon(themePixmap(QStringLiteral("icons/decrement")));
    connect(aRemove, &QAction::triggered, this, &DeckEditorSettingsPage::actRemoveURL);

    aRateLimit = new QAction(this);
    aRateLimit->setIcon(themePixmap(QStringLiteral("icons/cogwheel")));
    connect(aRateLimit, &QAction::triggered, this, &DeckEditorSettingsPage::actAdjustRateLimit);

    auto *urlToolBar = new QToolBar;
    urlToolBar->setOrientation(Qt::Vertical);
    urlToolBar->addAction(aAdd);
    urlToolBar->addAction(aRemove);
    urlToolBar->addAction(aEdit);
    urlToolBar->addAction(aRateLimit);
    urlToolBar->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);

    auto *urlListLayout = new QHBoxLayout;
    urlListLayout->addWidget(urlToolBar);
    urlListLayout->addWidget(urlList);

    // Top Layout
    lpGeneralGrid->addWidget(&picDownloadCheckBox, 0, 0);
    lpGeneralGrid->addWidget(&resetDownloadURLs, 0, 1);
    lpGeneralGrid->addLayout(urlListLayout, 1, 0, 1, 2);
    lpGeneralGrid->addWidget(&urlLinkLabel, 4, 0);

    // Spoiler Layout
    lpSpoilerGrid->addWidget(&mcDownloadSpoilersCheckBox, 0, 0);
    lpSpoilerGrid->addWidget(&mcSpoilerSaveLabel, 1, 0);
    lpSpoilerGrid->addWidget(mpSpoilerSavePathLineEdit, 1, 1);
    lpSpoilerGrid->addWidget(mpSpoilerPathButton, 1, 2);
    lpSpoilerGrid->addWidget(&lastUpdatedLabel, 2, 0);
    lpSpoilerGrid->addWidget(updateNowButton, 2, 1);
    lpSpoilerGrid->addWidget(&infoOnSpoilersLabel, 3, 0, 1, 3, Qt::AlignTop);

    // On a change to the checkbox, hide/un-hide the other fields
    connect(&mcDownloadSpoilersCheckBox, &QCheckBox::toggled, &SettingsCache::instance().downloads(),
            &DownloadSettings::setDownloadSpoilerStatus);
    connect(&mcDownloadSpoilersCheckBox, &QCheckBox::toggled, this, &DeckEditorSettingsPage::setSpoilersEnabled);

    mpGeneralGroupBox = new QGroupBox;
    mpGeneralGroupBox->setLayout(lpGeneralGrid);

    mpSpoilerGroupBox = new QGroupBox;
    mpSpoilerGroupBox->setLayout(lpSpoilerGrid);

    auto *lpMainLayout = new QVBoxLayout;
    lpMainLayout->addWidget(mpGeneralGroupBox);
    lpMainLayout->addWidget(mpSpoilerGroupBox);

    setLayout(lpMainLayout);

    connect(&SettingsCache::instance().personal(), &PersonalSettings::langChanged, this,
            &DeckEditorSettingsPage::retranslateUi);
    retranslateUi();
}

void DeckEditorSettingsPage::resetDownloadedURLsButtonClicked()
{
    SettingsCache::instance().downloads().resetToDefaultURLs();
    urlList->clear();
    for (const QString &url : SettingsCache::instance().downloads().getAllURLs()) {
        addUrlItem(url);
    }
    QMessageBox::information(this, tr("Success"), tr("Download URLs have been reset."));
}

void DeckEditorSettingsPage::actAddURL()
{
    bool ok;
    QString msg = QInputDialog::getText(this, tr("Add URL"), tr("URL:"), QLineEdit::Normal, QString(), &ok);
    if (ok) {
        addUrlItem(msg);
        storeSettings();
    }
}

void DeckEditorSettingsPage::actRemoveURL()
{
    if (urlList->currentItem() != nullptr) {
        delete urlList->takeItem(urlList->currentRow());
        storeSettings();
    }
}

void DeckEditorSettingsPage::actEditURL()
{
    QListWidgetItem *item = urlList->currentItem();
    if (item) {
        const QString oldText = urlForItem(item);
        bool ok;
        QString msg = QInputDialog::getText(this, tr("Edit URL"), tr("URL:"), QLineEdit::Normal, oldText, &ok);
        if (ok) {
            item->setData(Qt::UserRole, msg);
            item->setText(urlLabel(msg));
            storeSettings();
        }
    }
}

void DeckEditorSettingsPage::storeSettings()
{
    qInfo() << "URL Priority Reset";

    QStringList downloadUrls;
    for (int i = 0; i < urlList->count(); i++) {
        const QString url = urlForItem(urlList->item(i));
        qInfo() << "Priority" << i << ":" << url;
        downloadUrls << url;
    }
    SettingsCache::instance().downloads().setDownloadUrls(downloadUrls);

    // Drop per-host limits whose host is no longer referenced by any configured URL, so removing
    // a URL doesn't leave a stale throttle behind that reactivates if the host is re-added.
    QSet<QString> usedHosts;
    for (const QString &url : downloadUrls) {
        const QString host = QUrl(url).host();
        if (!host.isEmpty()) {
            usedHosts.insert(host);
        }
    }
    QHash<QString, int> limits = SettingsCache::instance().downloads().getHostRequestLimits();
    bool limitsChanged = false;
    for (auto it = limits.begin(); it != limits.end();) {
        if (!usedHosts.contains(it.key())) {
            it = limits.erase(it);
            limitsChanged = true;
        } else {
            ++it;
        }
    }
    if (limitsChanged) {
        SettingsCache::instance().downloads().setHostRequestLimits(limits);
    }

    refreshUrlItems();
}

QListWidgetItem *DeckEditorSettingsPage::addUrlItem(const QString &url)
{
    auto *item = new QListWidgetItem(urlLabel(url));
    item->setData(Qt::UserRole, url);
    urlList->addItem(item);
    return item;
}

QString DeckEditorSettingsPage::urlForItem(const QListWidgetItem *item) const
{
    return item->data(Qt::UserRole).toString();
}

QString DeckEditorSettingsPage::urlLabel(const QString &url) const
{
    const QString host = QUrl(url).host();
    if (host.isEmpty()) {
        return url;
    }

    const QHash<QString, int> limits = SettingsCache::instance().downloads().getHostRequestLimits();
    const int devCap =
        DownloadSettings::getDeveloperHostCaps().value(host, DownloadSettings::DEFAULT_HOST_REQUEST_LIMIT);
    if (devCap == DownloadSettings::UNLIMITED_HOST_QUOTA && !limits.contains(host)) {
        return tr("%1  (unlimited)").arg(url);
    }

    const int requested = limits.value(
        host, devCap == DownloadSettings::UNLIMITED_HOST_QUOTA ? DownloadSettings::DEFAULT_HOST_REQUEST_LIMIT : devCap);
    const int effective = SettingsCache::instance().downloads().clampHostRequestLimit(host, requested);
    return tr("%1  (%2/s)").arg(url).arg(effective);
}

void DeckEditorSettingsPage::refreshUrlItems()
{
    for (int i = 0; i < urlList->count(); ++i) {
        QListWidgetItem *item = urlList->item(i);
        item->setText(urlLabel(urlForItem(item)));
    }
}

void DeckEditorSettingsPage::actAdjustRateLimit()
{
    if (urlList->currentItem() == nullptr) {
        QMessageBox::information(this, tr("Adjust Rate Limit"), tr("Select a URL in the list first."));
        return;
    }

    const QString host = QUrl(urlForItem(urlList->currentItem())).host();
    if (host.isEmpty()) {
        QMessageBox::information(this, tr("Adjust Rate Limit"), tr("The selected URL does not have a valid host."));
        return;
    }

    const QHash<QString, int> &devCaps = DownloadSettings::getDeveloperHostCaps();
    const QHash<QString, int> currentLimits = SettingsCache::instance().downloads().getHostRequestLimits();
    const int devCap = devCaps.value(host, DownloadSettings::DEFAULT_HOST_REQUEST_LIMIT);
    const bool unlocked = devCap == DownloadSettings::UNLIMITED_HOST_QUOTA;

    bool ok = false;
    int minimum;
    int maximum;
    int defaultValue;
    QString prompt;
    if (unlocked) {
        minimum = 0; // 0 means "unlimited"
        maximum = DownloadSettings::DEFAULT_HOST_REQUEST_LIMIT;
        defaultValue = currentLimits.value(host, 0);
        prompt = tr("Requests per second (0 = unlimited, fastest; up to %1):").arg(maximum);
    } else {
        minimum = DownloadSettings::MIN_HOST_REQUEST_LIMIT;
        maximum = devCap;
        defaultValue = currentLimits.value(host, devCap);
        prompt = tr("Requests per second (developer maximum is %1):").arg(maximum);
    }

    const int value = QInputDialog::getInt(this, tr("Adjust Rate Limit for %1").arg(host), prompt, defaultValue,
                                           minimum, maximum, 1, &ok);
    if (!ok) {
        return;
    }

    QHash<QString, int> limits = currentLimits;
    if (unlocked ? value == 0 : value == devCap) {
        limits.remove(host);
    } else {
        limits.insert(host, value);
    }
    SettingsCache::instance().downloads().setHostRequestLimits(limits);
    refreshUrlItems();
}

void DeckEditorSettingsPage::urlListChanged(const QModelIndex &, int, int, const QModelIndex &, int)
{
    storeSettings();
}

void DeckEditorSettingsPage::updateSpoilers()
{
    // Disable the button so the user can only press it once at a time
    updateNowButton->setDisabled(true);
    updateNowButton->setText(tr("Updating..."));

    // Create a new SBU that will act as if the client was just reloaded
    auto *sbu = new SpoilerBackgroundUpdater();
    connect(sbu, &SpoilerBackgroundUpdater::spoilerCheckerDone, this, &DeckEditorSettingsPage::unlockSettings);
    connect(sbu, &SpoilerBackgroundUpdater::spoilersUpdatedSuccessfully, this, &DeckEditorSettingsPage::unlockSettings);
}

void DeckEditorSettingsPage::unlockSettings()
{
    updateNowButton->setDisabled(false);
    updateNowButton->setText(tr("Update Spoilers"));
}

QString DeckEditorSettingsPage::getLastUpdateTime()
{
    QString fileName = SettingsCache::instance().getSpoilerCardDatabasePath();
    QFileInfo fi(fileName);
    QDir fileDir(fi.path());
    QFile file(fileName);

    if (file.exists()) {
        return fi.lastModified().toString("MMM d, hh:mm");
    }

    return QString();
}

void DeckEditorSettingsPage::spoilerPathButtonClicked()
{
    QString lsPath = QFileDialog::getExistingDirectory(this, tr("Choose path"), mpSpoilerSavePathLineEdit->text());
    if (lsPath.isEmpty()) {
        return;
    }

    mpSpoilerSavePathLineEdit->setText(lsPath + "/spoiler.xml");
    SettingsCache::instance().paths().setSpoilerDatabasePath(lsPath + "/spoiler.xml");
}

void DeckEditorSettingsPage::setSpoilersEnabled(bool anInput)
{
    msDownloadSpoilersLabel.setEnabled(anInput);
    mcSpoilerSaveLabel.setEnabled(anInput);
    mpSpoilerSavePathLineEdit->setEnabled(anInput);
    mpSpoilerPathButton->setEnabled(anInput);
    lastUpdatedLabel.setEnabled(anInput);
    updateNowButton->setEnabled(anInput);
    infoOnSpoilersLabel.setEnabled(anInput);

    if (!anInput) {
        SpoilerBackgroundUpdater::deleteSpoilerFile();
    }
}

void DeckEditorSettingsPage::retranslateUi()
{
    mpGeneralGroupBox->setTitle(tr("URL Download Priority"));
    mpSpoilerGroupBox->setTitle(tr("Spoilers"));
    mcDownloadSpoilersCheckBox.setText(tr("Download Spoilers Automatically"));
    mcSpoilerSaveLabel.setText(tr("Spoiler Location:"));
    lastUpdatedLabel.setText(tr("Last Change") + ": " + getLastUpdateTime());
    infoOnSpoilersLabel.setText(tr("Spoilers download automatically on launch") + "\n" +
                                tr("Press the button to manually update without relaunching") + "\n\n" +
                                tr("Do not close settings until manual update is complete"));
    picDownloadCheckBox.setText(tr("Download card pictures on the fly"));
    urlLinkLabel.setText(QString("<a href='%1'>%2</a>").arg(WIKI_CUSTOM_PIC_URL).arg(tr("How to add a custom URL")));
    resetDownloadURLs.setText(tr("Reset Download URLs"));
    updateNowButton->setText(tr("Update Spoilers"));
    aAdd->setText(tr("Add New URL"));
    aEdit->setText(tr("Edit URL"));
    aRemove->setText(tr("Remove URL"));
    aRateLimit->setText(tr("Adjust Rate Limit"));

    // The per-URL rate limit suffixes are translated, so refresh them when the language changes.
    refreshUrlItems();
}
