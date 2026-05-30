#include "deck_editor_settings_page.h"

#include "../../../client/settings/cache_settings.h"
#include "update/card_spoiler/spoiler_background_updater.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QToolBar>

DeckEditorSettingsPage::DeckEditorSettingsPage()
{
    picDownloadCheckBox.setChecked(SettingsCache::instance().getPicDownload());
    connect(&picDownloadCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance(),
            &SettingsCache::setPicDownload);

    urlLinkLabel.setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    urlLinkLabel.setOpenExternalLinks(true);

    connect(&resetDownloadURLs, &QPushButton::clicked, this, &DeckEditorSettingsPage::resetDownloadedURLsButtonClicked);

    auto *lpGeneralGrid = new QGridLayout;
    auto *lpSpoilerGrid = new QGridLayout;

    mcDownloadSpoilersCheckBox.setChecked(SettingsCache::instance().getDownloadSpoilersStatus());

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

    urlList->addItems(SettingsCache::instance().downloads().getAllURLs());

    aAdd = new QAction(this);
    aAdd->setIcon(QPixmap("theme:icons/increment"));
    connect(aAdd, &QAction::triggered, this, &DeckEditorSettingsPage::actAddURL);

    aEdit = new QAction(this);
    aEdit->setIcon(QPixmap("theme:icons/pencil"));
    connect(aEdit, &QAction::triggered, this, &DeckEditorSettingsPage::actEditURL);

    aRemove = new QAction(this);
    aRemove->setIcon(QPixmap("theme:icons/decrement"));
    connect(aRemove, &QAction::triggered, this, &DeckEditorSettingsPage::actRemoveURL);

    auto *urlToolBar = new QToolBar;
    urlToolBar->setOrientation(Qt::Vertical);
    urlToolBar->addAction(aAdd);
    urlToolBar->addAction(aRemove);
    urlToolBar->addAction(aEdit);
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
    connect(&mcDownloadSpoilersCheckBox, &QCheckBox::toggled, &SettingsCache::instance(),
            &SettingsCache::setDownloadSpoilerStatus);
    connect(&mcDownloadSpoilersCheckBox, &QCheckBox::toggled, this, &DeckEditorSettingsPage::setSpoilersEnabled);

    mpGeneralGroupBox = new QGroupBox;
    mpGeneralGroupBox->setLayout(lpGeneralGrid);

    mpSpoilerGroupBox = new QGroupBox;
    mpSpoilerGroupBox->setLayout(lpSpoilerGrid);

    auto *lpMainLayout = new QVBoxLayout;
    lpMainLayout->addWidget(mpGeneralGroupBox);
    lpMainLayout->addWidget(mpSpoilerGroupBox);

    setLayout(lpMainLayout);

    connect(&SettingsCache::instance(), &SettingsCache::langChanged, this, &DeckEditorSettingsPage::retranslateUi);
    retranslateUi();
}

void DeckEditorSettingsPage::resetDownloadedURLsButtonClicked()
{
    SettingsCache::instance().downloads().resetToDefaultURLs();
    urlList->clear();
    urlList->addItems(SettingsCache::instance().downloads().getAllURLs());
    QMessageBox::information(this, tr("Success"), tr("Download URLs have been reset."));
}

void DeckEditorSettingsPage::actAddURL()
{
    bool ok;
    QString msg = QInputDialog::getText(this, tr("Add URL"), tr("URL:"), QLineEdit::Normal, QString(), &ok);
    if (ok) {
        urlList->addItem(msg);
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
    if (urlList->currentItem()) {
        QString oldText = urlList->currentItem()->text();
        bool ok;
        QString msg = QInputDialog::getText(this, tr("Edit URL"), tr("URL:"), QLineEdit::Normal, oldText, &ok);
        if (ok) {
            urlList->currentItem()->setText(msg);
            storeSettings();
        }
    }
}

void DeckEditorSettingsPage::storeSettings()
{
    qInfo() << "URL Priority Reset";

    QStringList downloadUrls;
    for (int i = 0; i < urlList->count(); i++) {
        qInfo() << "Priority" << i << ":" << urlList->item(i)->text();
        downloadUrls << urlList->item(i)->text();
    }
    SettingsCache::instance().downloads().setDownloadUrls(downloadUrls);
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
    SettingsCache::instance().setSpoilerDatabasePath(lsPath + "/spoiler.xml");
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
}