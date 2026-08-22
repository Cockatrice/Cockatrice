#include "general_settings_page.h"

#include "../../../client/settings/cache_settings.h"
#include "../main.h"
#include "../server/user/user_info_connection.h"
#include "update/client/release_channel.h"

#include <QCoreApplication>
#include <QFileDialog>
#include <QGridLayout>
#include <QLineEdit>
#include <QTranslator>
#include <libcockatrice/settings/paths_settings.h>
#include <libcockatrice/settings/personal_settings.h>
#include <libcockatrice/settings/tabs_settings.h>
#include <libcockatrice/settings/updates_settings.h>
#include <libcockatrice/utility/macros.h>

enum startupCardUpdateCheckBehaviorIndex
{
    startupCardUpdateCheckBehaviorIndexNone,
    startupCardUpdateCheckBehaviorIndexPrompt,
    startupCardUpdateCheckBehaviorIndexAlways
};

GeneralSettingsPage::GeneralSettingsPage()
{
    // language settings
    QStringList languageCodes = findQmFiles();
    for (const QString &code : languageCodes) {
        QString langName = languageName(code);
        languageBox.addItem(langName, code);
    }

    QString setLanguage = QCoreApplication::translate("i18n", DEFAULT_LANG_NAME);
    int index = languageBox.findText(setLanguage, Qt::MatchExactly);
    if (index == -1) {
        qWarning() << "could not find language" << setLanguage;
    } else {
        languageBox.setCurrentIndex(index);
    }

    advertiseTranslationPageLabel.setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    advertiseTranslationPageLabel.setOpenExternalLinks(true);

    connect(&languageBox, qOverload<int>(&QComboBox::currentIndexChanged), this,
            &GeneralSettingsPage::languageBoxChanged);

    auto *languageGrid = new QGridLayout;
    languageGrid->addWidget(&languageLabel, 0, 0);
    languageGrid->addWidget(&languageBox, 0, 1);
    languageGrid->addWidget(&advertiseTranslationPageLabel, 1, 1, Qt::AlignRight);

    languageGroupBox = new QGroupBox;
    languageGroupBox->setLayout(languageGrid);

    // version settings
    SettingsCache &settings = SettingsCache::instance();
    startupUpdateCheckCheckBox.setChecked(settings.updates().getCheckUpdatesOnStartup());

    connect(&startupUpdateCheckCheckBox, &QCheckBox::QT_STATE_CHANGED, &settings.updates(),
            &UpdatesSettings::setCheckUpdatesOnStartup);

    updateNotificationCheckBox.setChecked(settings.getNotifyAboutUpdates());

    connect(&updateNotificationCheckBox, &QCheckBox::QT_STATE_CHANGED, &settings.updates(),
            &UpdatesSettings::setNotifyAboutUpdates);

    connect(&newVersionOracleCheckBox, &QCheckBox::QT_STATE_CHANGED, &settings.updates(),
            &UpdatesSettings::setNotifyAboutNewVersion);

    auto *versionGrid = new QGridLayout;
    versionGrid->addWidget(&updateReleaseChannelLabel, 0, 0);
    versionGrid->addWidget(&updateReleaseChannelBox, 0, 1);
    versionGrid->addWidget(&startupUpdateCheckCheckBox, 1, 0, 1, 2);
    versionGrid->addWidget(&updateNotificationCheckBox, 2, 0, 1, 2);
    versionGrid->addWidget(&newVersionOracleCheckBox, 3, 0, 1, 2);

    versionGroupBox = new QGroupBox;
    versionGroupBox->setLayout(versionGrid);

    // card database settings
    startupCardUpdateCheckBehaviorSelector.addItem(""); // these will be set in retranslateUI
    startupCardUpdateCheckBehaviorSelector.addItem("");
    startupCardUpdateCheckBehaviorSelector.addItem("");
    if (SettingsCache::instance().updates().getStartupCardUpdateCheckPromptForUpdate()) {
        startupCardUpdateCheckBehaviorSelector.setCurrentIndex(startupCardUpdateCheckBehaviorIndexPrompt);
    } else if (SettingsCache::instance().updates().getStartupCardUpdateCheckAlwaysUpdate()) {
        startupCardUpdateCheckBehaviorSelector.setCurrentIndex(startupCardUpdateCheckBehaviorIndexAlways);
    } else {
        startupCardUpdateCheckBehaviorSelector.setCurrentIndex(startupCardUpdateCheckBehaviorIndexNone);
    }

    connect(&startupCardUpdateCheckBehaviorSelector, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [](int index) {
                SettingsCache::instance().updates().setStartupCardUpdateCheckPromptForUpdate(
                    index == startupCardUpdateCheckBehaviorIndexPrompt);
                SettingsCache::instance().updates().setStartupCardUpdateCheckAlwaysUpdate(
                    index == startupCardUpdateCheckBehaviorIndexAlways);
            });

    cardUpdateCheckIntervalSpinBox.setMinimum(1);
    cardUpdateCheckIntervalSpinBox.setMaximum(30);
    cardUpdateCheckIntervalSpinBox.setValue(settings.updates().getCardUpdateCheckInterval());

    connect(&cardUpdateCheckIntervalSpinBox, qOverload<int>(&QSpinBox::valueChanged), &settings.updates(),
            &UpdatesSettings::setCardUpdateCheckInterval);

    newVersionOracleCheckBox.setChecked(settings.updates().getNotifyAboutNewVersion());

    auto *cardDatabaseGrid = new QGridLayout;
    cardDatabaseGrid->addWidget(&startupCardUpdateCheckBehaviorLabel, 0, 0);
    cardDatabaseGrid->addWidget(&startupCardUpdateCheckBehaviorSelector, 0, 1);
    cardDatabaseGrid->addWidget(&cardUpdateCheckIntervalLabel, 1, 0);
    cardDatabaseGrid->addWidget(&cardUpdateCheckIntervalSpinBox, 1, 1);
    cardDatabaseGrid->addWidget(&lastCardUpdateCheckDateLabel, 2, 1);

    cardDatabaseGroupBox = new QGroupBox;
    cardDatabaseGroupBox->setLayout(cardDatabaseGrid);

    // startup settings
    showTipsOnStartup.setChecked(settings.personal().getShowTipsOnStartup());

    connect(&showTipsOnStartup, &QCheckBox::clicked, &settings.personal(), &PersonalSettings::setShowTipsOnStartup);

    // startup destination
    for (int i = 0; i < 8; ++i) {
        startupTabSelector.addItem(""); // texts set in retranslateUi
    }
    startupTabSelector.setCurrentIndex(settings.tabs().getStartupTabIndex());

    connect(&startupTabSelector, qOverload<int>(&QComboBox::currentIndexChanged), &settings.tabs(),
            &TabsSettings::setStartupTabIndex);
    connect(&startupTabSelector, qOverload<int>(&QComboBox::currentIndexChanged), this,
            &GeneralSettingsPage::updateStartupServerControlsVisibility);

    const QString savedHost = settings.tabs().getStartupServerHost();
    const QString savedPort = settings.tabs().getStartupServerPort();
    int startupServerIndex = -1;
    UserConnection_Information uci;
    for (const auto &savedServer : uci.getServerInfo()) {
        const UserConnection_Information &info = savedServer.second;
        const QString saveName = info.getSaveName();
        if (saveName.isEmpty()) {
            continue;
        }
        startupServerSelector.addItem(saveName, QVariantList{info.getServer(), info.getPort()});
        if (startupServerIndex == -1 && info.getServer() == savedHost && info.getPort() == savedPort) {
            startupServerIndex = startupServerSelector.count() - 1;
        }
    }
    startupServerSelector.setCurrentIndex(startupServerIndex);

    connect(&startupServerSelector, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int index) {
        const QVariantList serverInfo = startupServerSelector.itemData(index).toList();
        if (serverInfo.size() != 2) {
            return;
        }
        TabsSettings &tabs = SettingsCache::instance().tabs();
        tabs.setStartupServerHost(serverInfo[0].toString());
        tabs.setStartupServerPort(serverInfo[1].toString());
    });

    startupRoomNameEdit = new QLineEdit(settings.tabs().getStartupRoomName());
    // Default (Expanding) would stretch the whole controls column when this row becomes visible,
    // so size it like the combo boxes instead: fills the column, never widens it.
    startupRoomNameEdit->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    connect(startupRoomNameEdit, &QLineEdit::editingFinished, this,
            [this] { SettingsCache::instance().tabs().setStartupRoomName(startupRoomNameEdit->text().trimmed()); });

    auto *startupGrid = new QGridLayout;
    startupGrid->addWidget(&showTipsOnStartup, 0, 0, 1, 2);
    startupGrid->addWidget(&startupTabLabel, 1, 0);
    startupGrid->addWidget(&startupTabSelector, 1, 1);
    startupGrid->addWidget(&startupServerLabel, 2, 0);
    startupGrid->addWidget(&startupServerSelector, 2, 1);
    startupGrid->addWidget(&startupRoomLabel, 3, 0);
    startupGrid->addWidget(startupRoomNameEdit, 3, 1);

    updateStartupServerControlsVisibility();

    startupGroupBox = new QGroupBox;
    startupGroupBox->setLayout(startupGrid);

    // paths settings
    deckPathEdit = new QLineEdit(settings.paths().getDeckPath());
    deckPathEdit->setReadOnly(true);
    auto *deckPathButton = new QPushButton("...");
    connect(deckPathButton, &QPushButton::clicked, this, &GeneralSettingsPage::deckPathButtonClicked);

    filtersPathEdit = new QLineEdit(settings.paths().getFiltersPath());
    filtersPathEdit->setReadOnly(true);
    auto *filtersPathButton = new QPushButton("...");
    connect(filtersPathButton, &QPushButton::clicked, this, &GeneralSettingsPage::filtersPathButtonClicked);

    replaysPathEdit = new QLineEdit(settings.paths().getReplaysPath());
    replaysPathEdit->setReadOnly(true);
    auto *replaysPathButton = new QPushButton("...");
    connect(replaysPathButton, &QPushButton::clicked, this, &GeneralSettingsPage::replaysPathButtonClicked);

    picsPathEdit = new QLineEdit(settings.paths().getPicsPath());
    picsPathEdit->setReadOnly(true);
    auto *picsPathButton = new QPushButton("...");
    connect(picsPathButton, &QPushButton::clicked, this, &GeneralSettingsPage::picsPathButtonClicked);

    cardDatabasePathEdit = new QLineEdit(settings.getCardDatabasePath());
    cardDatabasePathEdit->setReadOnly(true);
    auto *cardDatabasePathButton = new QPushButton("...");
    connect(cardDatabasePathButton, &QPushButton::clicked, this, &GeneralSettingsPage::cardDatabasePathButtonClicked);

    customCardDatabasePathEdit = new QLineEdit(settings.getCustomCardDatabasePath());
    customCardDatabasePathEdit->setReadOnly(true);
    auto *customCardDatabasePathButton = new QPushButton("...");
    connect(customCardDatabasePathButton, &QPushButton::clicked, this,
            &GeneralSettingsPage::customCardDatabaseButtonClicked);

    tokenDatabasePathEdit = new QLineEdit(settings.getTokenDatabasePath());
    tokenDatabasePathEdit->setReadOnly(true);
    auto *tokenDatabasePathButton = new QPushButton("...");
    connect(tokenDatabasePathButton, &QPushButton::clicked, this, &GeneralSettingsPage::tokenDatabasePathButtonClicked);

    // Required init here to avoid crashing on Portable builds
    resetAllPathsButton = new QPushButton;

    bool isPortable = settings.getIsPortableBuild();
    if (isPortable) {
        deckPathEdit->setEnabled(false);
        filtersPathEdit->setEnabled(false);
        replaysPathEdit->setEnabled(false);
        picsPathEdit->setEnabled(false);
        cardDatabasePathEdit->setEnabled(false);
        customCardDatabasePathEdit->setEnabled(false);
        tokenDatabasePathEdit->setEnabled(false);

        deckPathButton->setVisible(false);
        replaysPathButton->setVisible(false);
        picsPathButton->setVisible(false);
        cardDatabasePathButton->setVisible(false);
        customCardDatabasePathButton->setVisible(false);
        tokenDatabasePathButton->setVisible(false);
    } else {
        connect(resetAllPathsButton, &QPushButton::clicked, this, &GeneralSettingsPage::resetAllPathsClicked);
        allPathsResetLabel = new QLabel(tr("All paths have been reset"));
        allPathsResetLabel->setVisible(false);
    }

    auto *pathsGrid = new QGridLayout;
    pathsGrid->addWidget(&deckPathLabel, 0, 0);
    pathsGrid->addWidget(deckPathEdit, 0, 1);
    pathsGrid->addWidget(deckPathButton, 0, 2);
    pathsGrid->addWidget(&filtersPathLabel, 1, 0);
    pathsGrid->addWidget(filtersPathEdit, 1, 1);
    pathsGrid->addWidget(filtersPathButton, 1, 2);
    pathsGrid->addWidget(&replaysPathLabel, 2, 0);
    pathsGrid->addWidget(replaysPathEdit, 2, 1);
    pathsGrid->addWidget(replaysPathButton, 2, 2);
    pathsGrid->addWidget(&picsPathLabel, 3, 0);
    pathsGrid->addWidget(picsPathEdit, 3, 1);
    pathsGrid->addWidget(picsPathButton, 3, 2);
    pathsGrid->addWidget(&cardDatabasePathLabel, 4, 0);
    pathsGrid->addWidget(cardDatabasePathEdit, 4, 1);
    pathsGrid->addWidget(cardDatabasePathButton, 4, 2);
    pathsGrid->addWidget(&customCardDatabasePathLabel, 5, 0);
    pathsGrid->addWidget(customCardDatabasePathEdit, 5, 1);
    pathsGrid->addWidget(customCardDatabasePathButton, 5, 2);
    pathsGrid->addWidget(&tokenDatabasePathLabel, 6, 0);
    pathsGrid->addWidget(tokenDatabasePathEdit, 6, 1);
    pathsGrid->addWidget(tokenDatabasePathButton, 6, 2);
    if (!isPortable) {
        pathsGrid->addWidget(resetAllPathsButton, 7, 0);
        pathsGrid->addWidget(allPathsResetLabel, 7, 1);
    }
    pathsGroupBox = new QGroupBox;
    pathsGroupBox->setLayout(pathsGrid);

    auto *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(languageGroupBox);
    mainLayout->addWidget(versionGroupBox);
    mainLayout->addWidget(cardDatabaseGroupBox);
    mainLayout->addWidget(startupGroupBox);
    mainLayout->addWidget(pathsGroupBox);
    mainLayout->addStretch();

    GeneralSettingsPage::retranslateUi();

    // connect the ReleaseChannel combo box only after the entries are inserted in retranslateUi
    connect(&updateReleaseChannelBox, qOverload<int>(&QComboBox::currentIndexChanged), &settings.updates(),
            &UpdatesSettings::setUpdateReleaseChannelIndex);
    updateReleaseChannelBox.setCurrentIndex(settings.getUpdateReleaseChannelIndex());

    setLayout(mainLayout);

    connect(&SettingsCache::instance().personal(), &PersonalSettings::langChanged, this,
            &GeneralSettingsPage::retranslateUi);
    retranslateUi();
}

QStringList GeneralSettingsPage::findQmFiles()
{
    QDir dir(translationPath);
    QStringList fileNames = dir.entryList(QStringList(translationPrefix + "_*.qm"), QDir::Files, QDir::Name);
    fileNames.replaceInStrings(QRegularExpression(translationPrefix + "_(.*)\\.qm"), "\\1");
    return fileNames;
}

QString GeneralSettingsPage::languageName(const QString &lang)
{
    QTranslator qTranslator;

    QString appNameHint = translationPrefix + "_" + lang;
    bool appTranslationLoaded = qTranslator.load(appNameHint, translationPath);
    if (!appTranslationLoaded) {
        qCWarning(GeneralSettingsPageLog)
            << "Unable to load" << translationPrefix << "translation" << appNameHint << "at" << translationPath;
    }

    return qTranslator.translate("i18n", DEFAULT_LANG_NAME);
}

void GeneralSettingsPage::deckPathButtonClicked()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Choose path"), deckPathEdit->text());
    if (path.isEmpty()) {
        return;
    }

    deckPathEdit->setText(path);
    SettingsCache::instance().paths().setDeckPath(path);
}

void GeneralSettingsPage::filtersPathButtonClicked()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Choose path"), filtersPathEdit->text());
    if (path.isEmpty()) {
        return;
    }

    filtersPathEdit->setText(path);
    SettingsCache::instance().paths().setFiltersPath(path);
}

void GeneralSettingsPage::replaysPathButtonClicked()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Choose path"), replaysPathEdit->text());
    if (path.isEmpty()) {
        return;
    }

    replaysPathEdit->setText(path);
    SettingsCache::instance().paths().setReplaysPath(path);
}

void GeneralSettingsPage::picsPathButtonClicked()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Choose path"), picsPathEdit->text());
    if (path.isEmpty()) {
        return;
    }

    picsPathEdit->setText(path);
    SettingsCache::instance().paths().setPicsPath(path);
}

void GeneralSettingsPage::cardDatabasePathButtonClicked()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Choose path"), cardDatabasePathEdit->text());
    if (path.isEmpty()) {
        return;
    }

    cardDatabasePathEdit->setText(path);
    SettingsCache::instance().paths().setCardDatabasePath(path);
}

void GeneralSettingsPage::customCardDatabaseButtonClicked()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Choose path"), customCardDatabasePathEdit->text());
    if (path.isEmpty()) {
        return;
    }

    customCardDatabasePathEdit->setText(path);
    SettingsCache::instance().paths().setCustomCardDatabasePath(path);
}

void GeneralSettingsPage::tokenDatabasePathButtonClicked()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Choose path"), tokenDatabasePathEdit->text());
    if (path.isEmpty()) {
        return;
    }

    tokenDatabasePathEdit->setText(path);
    SettingsCache::instance().paths().setTokenDatabasePath(path);
}

void GeneralSettingsPage::resetAllPathsClicked()
{
    SettingsCache &settings = SettingsCache::instance();
    settings.resetPaths();
    deckPathEdit->setText(settings.paths().getDeckPath());
    replaysPathEdit->setText(settings.paths().getReplaysPath());
    picsPathEdit->setText(settings.paths().getPicsPath());
    cardDatabasePathEdit->setText(settings.getCardDatabasePath());
    customCardDatabasePathEdit->setText(settings.getCustomCardDatabasePath());
    tokenDatabasePathEdit->setText(settings.getTokenDatabasePath());
    allPathsResetLabel->setVisible(true);
}

void GeneralSettingsPage::languageBoxChanged(int index)
{
    SettingsCache::instance().personal().setLang(languageBox.itemData(index).toString());
}

void GeneralSettingsPage::updateStartupServerControlsVisibility()
{
    const int index = startupTabSelector.currentIndex();
    const bool serverNeeded = index == StartupTab::StartupTabServer || index == StartupTab::StartupTabServerRoom;
    const bool roomNeeded = index == StartupTab::StartupTabServerRoom;
    startupServerLabel.setVisible(serverNeeded);
    startupServerSelector.setVisible(serverNeeded);
    startupRoomLabel.setVisible(roomNeeded);
    startupRoomNameEdit->setVisible(roomNeeded);
}

void GeneralSettingsPage::retranslateUi()
{
    languageGroupBox->setTitle(tr("Language settings"));
    languageLabel.setText(tr("Language:"));

    versionGroupBox->setTitle(tr("Version settings"));
    cardDatabaseGroupBox->setTitle(tr("Card database"));
    startupGroupBox->setTitle(tr("Startup settings"));

    if (SettingsCache::instance().getIsPortableBuild()) {
        pathsGroupBox->setTitle(tr("Paths (editing disabled in portable mode)"));
    } else {
        pathsGroupBox->setTitle(tr("Paths"));
    }
    advertiseTranslationPageLabel.setText(
        QString("<a href='%1'>%2</a>").arg(WIKI_TRANSLATION_FAQ).arg(tr("How to help with translations")));
    deckPathLabel.setText(tr("Decks directory:"));
    filtersPathLabel.setText(tr("Filters directory:"));
    replaysPathLabel.setText(tr("Replays directory:"));
    picsPathLabel.setText(tr("Pictures directory:"));
    cardDatabasePathLabel.setText(tr("Card database:"));
    customCardDatabasePathLabel.setText(tr("Custom database directory:"));
    tokenDatabasePathLabel.setText(tr("Token database:"));
    updateReleaseChannelLabel.setText(tr("Update channel"));
    startupUpdateCheckCheckBox.setText(tr("Check for client updates on startup"));
    startupCardUpdateCheckBehaviorLabel.setText(tr("Check for card database updates on startup"));
    startupCardUpdateCheckBehaviorSelector.setItemText(startupCardUpdateCheckBehaviorIndexNone, tr("Don't check"));
    startupCardUpdateCheckBehaviorSelector.setItemText(startupCardUpdateCheckBehaviorIndexPrompt,
                                                       tr("Prompt for update"));
    startupCardUpdateCheckBehaviorSelector.setItemText(startupCardUpdateCheckBehaviorIndexAlways,
                                                       tr("Always update in the background"));
    cardUpdateCheckIntervalLabel.setText(tr("Check for card database updates every"));
    cardUpdateCheckIntervalSpinBox.setSuffix(tr(" days"));
    updateNotificationCheckBox.setText(tr("Notify if a feature supported by the server is missing in my client"));
    newVersionOracleCheckBox.setText(tr("Automatically run Oracle when running a new version of Cockatrice"));
    showTipsOnStartup.setText(tr("Show tips on startup"));
    startupTabLabel.setText(tr("Startup tab:"));
    startupTabSelector.setItemText(StartupTab::StartupTabHome, tr("Home"));
    startupTabSelector.setItemText(StartupTab::StartupTabVisualDeckStorage, tr("Visual Deck Storage"));
    startupTabSelector.setItemText(StartupTab::StartupTabDeckStorage, tr("Deck Storage"));
    startupTabSelector.setItemText(StartupTab::StartupTabReplays, tr("Game Replays"));
    startupTabSelector.setItemText(StartupTab::StartupTabDeckEditor, tr("Deck Editor"));
    startupTabSelector.setItemText(StartupTab::StartupTabVisualDeckEditor, tr("Visual Deck Editor"));
    startupTabSelector.setItemText(StartupTab::StartupTabServer, tr("Server"));
    startupTabSelector.setItemText(StartupTab::StartupTabServerRoom, tr("Server Room"));
    startupTabSelector.setToolTip(
        tr("The tab shown when Cockatrice starts. If the chosen tab is not open yet, it is opened."));
    startupServerLabel.setText(tr("Server:"));
    startupRoomLabel.setText(tr("Room:"));
    startupRoomNameEdit->setPlaceholderText(tr("Room name"));
    resetAllPathsButton->setText(tr("Reset all paths"));

    const auto &settings = SettingsCache::instance();

    QDate lastCheckDate = settings.updates().getLastCardUpdateCheck();
    int daysAgo = lastCheckDate.daysTo(QDate::currentDate());

    lastCardUpdateCheckDateLabel.setText(
        tr("Last update check on %1 (%2 days ago)").arg(lastCheckDate.toString()).arg(daysAgo));

    // We can't change the strings after they're put into the QComboBox, so this is our workaround
    int oldIndex = updateReleaseChannelBox.currentIndex();
    updateReleaseChannelBox.clear();
    for (ReleaseChannel *chan : settings.getUpdateReleaseChannels()) {
        updateReleaseChannelBox.addItem(tr(chan->getName().toUtf8()));
    }
    updateReleaseChannelBox.setCurrentIndex(oldIndex);
}