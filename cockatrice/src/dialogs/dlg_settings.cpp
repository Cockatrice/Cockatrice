#include "dlg_settings.h"

#include "../client/get_text_with_max.h"
#include "../client/network/release_channel.h"
#include "../client/network/spoiler_background_updater.h"
#include "../client/sound_engine.h"
#include "../client/ui/picture_loader.h"
#include "../client/ui/theme_manager.h"
#include "../game/cards/card_database.h"
#include "../game/cards/card_database_manager.h"
#include "../main.h"
#include "../settings/cache_settings.h"
#include "../utility/sequence_edit.h"

#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QDebug>
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QScreen>
#include <QSlider>
#include <QSpinBox>
#include <QStackedWidget>
#include <QToolBar>
#include <QTranslator>
#include <QTreeWidget>
#include <QTreeWidgetItem>

#define WIKI_CUSTOM_PIC_URL "https://github.com/Cockatrice/Cockatrice/wiki/Custom-Picture-Download-URLs"
#define WIKI_CUSTOM_SHORTCUTS "https://github.com/Cockatrice/Cockatrice/wiki/Custom-Keyboard-Shortcuts"

GeneralSettingsPage::GeneralSettingsPage()
{
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

    // updates
    SettingsCache &settings = SettingsCache::instance();
    QList<ReleaseChannel *> channels = settings.getUpdateReleaseChannels();
    foreach (ReleaseChannel *chan, channels) {
        updateReleaseChannelBox.insertItem(chan->getIndex(), tr(chan->getName().toUtf8()));
    }
    updateReleaseChannelBox.setCurrentIndex(settings.getUpdateReleaseChannel()->getIndex());

    updateNotificationCheckBox.setChecked(settings.getNotifyAboutUpdates());
    newVersionOracleCheckBox.setChecked(settings.getNotifyAboutNewVersion());

    showTipsOnStartup.setChecked(settings.getShowTipsOnStartup());

    connect(&languageBox, SIGNAL(currentIndexChanged(int)), this, SLOT(languageBoxChanged(int)));
    connect(&updateReleaseChannelBox, SIGNAL(currentIndexChanged(int)), &settings, SLOT(setUpdateReleaseChannel(int)));
    connect(&updateNotificationCheckBox, &QCheckBox::QT_STATE_CHANGED, &settings, &SettingsCache::setNotifyAboutUpdate);
    connect(&newVersionOracleCheckBox, &QCheckBox::QT_STATE_CHANGED, &settings,
            &SettingsCache::setNotifyAboutNewVersion);
    connect(&showTipsOnStartup, SIGNAL(clicked(bool)), &settings, SLOT(setShowTipsOnStartup(bool)));

    auto *personalGrid = new QGridLayout;
    personalGrid->addWidget(&languageLabel, 0, 0);
    personalGrid->addWidget(&languageBox, 0, 1);
    personalGrid->addWidget(&updateReleaseChannelLabel, 1, 0);
    personalGrid->addWidget(&updateReleaseChannelBox, 1, 1);
    personalGrid->addWidget(&updateNotificationCheckBox, 3, 0, 1, 2);
    personalGrid->addWidget(&newVersionOracleCheckBox, 4, 0, 1, 2);
    personalGrid->addWidget(&showTipsOnStartup, 5, 0, 1, 2);

    personalGroupBox = new QGroupBox;
    personalGroupBox->setLayout(personalGrid);

    deckPathEdit = new QLineEdit(settings.getDeckPath());
    deckPathEdit->setReadOnly(true);
    QPushButton *deckPathButton = new QPushButton("...");
    connect(deckPathButton, SIGNAL(clicked()), this, SLOT(deckPathButtonClicked()));

    replaysPathEdit = new QLineEdit(settings.getReplaysPath());
    replaysPathEdit->setReadOnly(true);
    QPushButton *replaysPathButton = new QPushButton("...");
    connect(replaysPathButton, SIGNAL(clicked()), this, SLOT(replaysPathButtonClicked()));

    picsPathEdit = new QLineEdit(settings.getPicsPath());
    picsPathEdit->setReadOnly(true);
    QPushButton *picsPathButton = new QPushButton("...");
    connect(picsPathButton, SIGNAL(clicked()), this, SLOT(picsPathButtonClicked()));

    cardDatabasePathEdit = new QLineEdit(settings.getCardDatabasePath());
    cardDatabasePathEdit->setReadOnly(true);
    QPushButton *cardDatabasePathButton = new QPushButton("...");
    connect(cardDatabasePathButton, SIGNAL(clicked()), this, SLOT(cardDatabasePathButtonClicked()));

    customCardDatabasePathEdit = new QLineEdit(settings.getCustomCardDatabasePath());
    customCardDatabasePathEdit->setReadOnly(true);
    QPushButton *customCardDatabasePathButton = new QPushButton("...");
    connect(customCardDatabasePathButton, SIGNAL(clicked()), this, SLOT(customCardDatabaseButtonClicked()));

    tokenDatabasePathEdit = new QLineEdit(settings.getTokenDatabasePath());
    tokenDatabasePathEdit->setReadOnly(true);
    QPushButton *tokenDatabasePathButton = new QPushButton("...");
    connect(tokenDatabasePathButton, SIGNAL(clicked()), this, SLOT(tokenDatabasePathButtonClicked()));

    bool isPortable = settings.getIsPortableBuild();
    if (isPortable) {
        deckPathEdit->setEnabled(false);
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
        resetAllPathsButton = new QPushButton(tr("Reset all paths"));
        connect(resetAllPathsButton, SIGNAL(clicked()), this, SLOT(resetAllPathsClicked()));
        allPathsResetLabel = new QLabel(tr("All paths have been reset"));
        allPathsResetLabel->setVisible(false);
    }

    auto *pathsGrid = new QGridLayout;
    pathsGrid->addWidget(&deckPathLabel, 0, 0);
    pathsGrid->addWidget(deckPathEdit, 0, 1);
    pathsGrid->addWidget(deckPathButton, 0, 2);
    pathsGrid->addWidget(&replaysPathLabel, 1, 0);
    pathsGrid->addWidget(replaysPathEdit, 1, 1);
    pathsGrid->addWidget(replaysPathButton, 1, 2);
    pathsGrid->addWidget(&picsPathLabel, 2, 0);
    pathsGrid->addWidget(picsPathEdit, 2, 1);
    pathsGrid->addWidget(picsPathButton, 2, 2);
    pathsGrid->addWidget(&cardDatabasePathLabel, 3, 0);
    pathsGrid->addWidget(cardDatabasePathEdit, 3, 1);
    pathsGrid->addWidget(cardDatabasePathButton, 3, 2);
    pathsGrid->addWidget(&customCardDatabasePathLabel, 4, 0);
    pathsGrid->addWidget(customCardDatabasePathEdit, 4, 1);
    pathsGrid->addWidget(customCardDatabasePathButton, 4, 2);
    pathsGrid->addWidget(&tokenDatabasePathLabel, 5, 0);
    pathsGrid->addWidget(tokenDatabasePathEdit, 5, 1);
    pathsGrid->addWidget(tokenDatabasePathButton, 5, 2);
    if (!isPortable) {
        pathsGrid->addWidget(resetAllPathsButton, 6, 0);
        pathsGrid->addWidget(allPathsResetLabel, 6, 1);
    }
    pathsGroupBox = new QGroupBox;
    pathsGroupBox->setLayout(pathsGrid);

    auto *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(personalGroupBox);
    mainLayout->addWidget(pathsGroupBox);
    mainLayout->addStretch();

    setLayout(mainLayout);
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
        qDebug() << "Unable to load" << translationPrefix << "translation" << appNameHint << "at" << translationPath;
    }

    return qTranslator.translate("i18n", DEFAULT_LANG_NAME);
}

void GeneralSettingsPage::deckPathButtonClicked()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Choose path"), deckPathEdit->text());
    if (path.isEmpty())
        return;

    deckPathEdit->setText(path);
    SettingsCache::instance().setDeckPath(path);
}

void GeneralSettingsPage::replaysPathButtonClicked()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Choose path"), replaysPathEdit->text());
    if (path.isEmpty())
        return;

    replaysPathEdit->setText(path);
    SettingsCache::instance().setReplaysPath(path);
}

void GeneralSettingsPage::picsPathButtonClicked()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Choose path"), picsPathEdit->text());
    if (path.isEmpty())
        return;

    picsPathEdit->setText(path);
    SettingsCache::instance().setPicsPath(path);
}

void GeneralSettingsPage::cardDatabasePathButtonClicked()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Choose path"), cardDatabasePathEdit->text());
    if (path.isEmpty())
        return;

    cardDatabasePathEdit->setText(path);
    SettingsCache::instance().setCardDatabasePath(path);
}

void GeneralSettingsPage::customCardDatabaseButtonClicked()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Choose path"), customCardDatabasePathEdit->text());
    if (path.isEmpty())
        return;

    customCardDatabasePathEdit->setText(path);
    SettingsCache::instance().setCustomCardDatabasePath(path);
}

void GeneralSettingsPage::tokenDatabasePathButtonClicked()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Choose path"), tokenDatabasePathEdit->text());
    if (path.isEmpty())
        return;

    tokenDatabasePathEdit->setText(path);
    SettingsCache::instance().setTokenDatabasePath(path);
}

void GeneralSettingsPage::resetAllPathsClicked()
{
    SettingsCache &settings = SettingsCache::instance();
    settings.resetPaths();
    deckPathEdit->setText(settings.getDeckPath());
    replaysPathEdit->setText(settings.getReplaysPath());
    picsPathEdit->setText(settings.getPicsPath());
    cardDatabasePathEdit->setText(settings.getCardDatabasePath());
    customCardDatabasePathEdit->setText(settings.getCustomCardDatabasePath());
    tokenDatabasePathEdit->setText(settings.getTokenDatabasePath());
    allPathsResetLabel->setVisible(true);
}

void GeneralSettingsPage::languageBoxChanged(int index)
{
    SettingsCache::instance().setLang(languageBox.itemData(index).toString());
}

void GeneralSettingsPage::retranslateUi()
{
    personalGroupBox->setTitle(tr("Personal settings"));
    languageLabel.setText(tr("Language:"));

    if (SettingsCache::instance().getIsPortableBuild()) {
        pathsGroupBox->setTitle(tr("Paths (editing disabled in portable mode)"));
    } else {
        pathsGroupBox->setTitle(tr("Paths"));
    }

    deckPathLabel.setText(tr("Decks directory:"));
    replaysPathLabel.setText(tr("Replays directory:"));
    picsPathLabel.setText(tr("Pictures directory:"));
    cardDatabasePathLabel.setText(tr("Card database:"));
    customCardDatabasePathLabel.setText(tr("Custom database directory:"));
    tokenDatabasePathLabel.setText(tr("Token database:"));
    updateReleaseChannelLabel.setText(tr("Update channel"));
    updateNotificationCheckBox.setText(tr("Notify if a feature supported by the server is missing in my client"));
    newVersionOracleCheckBox.setText(tr("Automatically run Oracle when running a new version of Cockatrice"));
    showTipsOnStartup.setText(tr("Show tips on startup"));
}

AppearanceSettingsPage::AppearanceSettingsPage()
{
    SettingsCache &settings = SettingsCache::instance();
    QString themeName = SettingsCache::instance().getThemeName();

    QStringList themeDirs = themeManager->getAvailableThemes().keys();
    for (int i = 0; i < themeDirs.size(); i++) {
        themeBox.addItem(themeDirs[i]);
        if (themeDirs[i] == themeName)
            themeBox.setCurrentIndex(i);
    }

    connect(&themeBox, SIGNAL(currentIndexChanged(int)), this, SLOT(themeBoxChanged(int)));
    connect(&openThemeButton, SIGNAL(clicked()), this, SLOT(openThemeLocation()));

    auto *themeGrid = new QGridLayout;
    themeGrid->addWidget(&themeLabel, 0, 0);
    themeGrid->addWidget(&themeBox, 0, 1);
    themeGrid->addWidget(&openThemeButton, 1, 1);

    themeGroupBox = new QGroupBox;
    themeGroupBox->setLayout(themeGrid);

    displayCardNamesCheckBox.setChecked(settings.getDisplayCardNames());
    connect(&displayCardNamesCheckBox, &QCheckBox::QT_STATE_CHANGED, &settings, &SettingsCache::setDisplayCardNames);

    cardScalingCheckBox.setChecked(settings.getScaleCards());
    connect(&cardScalingCheckBox, &QCheckBox::QT_STATE_CHANGED, &settings, &SettingsCache::setCardScaling);

    verticalCardOverlapPercentBox.setValue(settings.getStackCardOverlapPercent());
    verticalCardOverlapPercentBox.setRange(0, 80);
    connect(&verticalCardOverlapPercentBox, SIGNAL(valueChanged(int)), &settings,
            SLOT(setStackCardOverlapPercent(int)));

    auto *cardsGrid = new QGridLayout;
    cardsGrid->addWidget(&displayCardNamesCheckBox, 0, 0, 1, 2);
    cardsGrid->addWidget(&cardScalingCheckBox, 1, 0, 1, 2);
    cardsGrid->addWidget(&verticalCardOverlapPercentLabel, 2, 0, 1, 1);
    cardsGrid->addWidget(&verticalCardOverlapPercentBox, 2, 1, 1, 1);

    cardsGroupBox = new QGroupBox;
    cardsGroupBox->setLayout(cardsGrid);

    horizontalHandCheckBox.setChecked(settings.getHorizontalHand());
    connect(&horizontalHandCheckBox, &QCheckBox::QT_STATE_CHANGED, &settings, &SettingsCache::setHorizontalHand);

    leftJustifiedHandCheckBox.setChecked(settings.getLeftJustified());
    connect(&leftJustifiedHandCheckBox, &QCheckBox::QT_STATE_CHANGED, &settings, &SettingsCache::setLeftJustified);

    auto *handGrid = new QGridLayout;
    handGrid->addWidget(&horizontalHandCheckBox, 0, 0, 1, 2);
    handGrid->addWidget(&leftJustifiedHandCheckBox, 1, 0, 1, 2);

    handGroupBox = new QGroupBox;
    handGroupBox->setLayout(handGrid);

    invertVerticalCoordinateCheckBox.setChecked(settings.getInvertVerticalCoordinate());
    connect(&invertVerticalCoordinateCheckBox, &QCheckBox::QT_STATE_CHANGED, &settings,
            &SettingsCache::setInvertVerticalCoordinate);

    minPlayersForMultiColumnLayoutEdit.setMinimum(2);
    minPlayersForMultiColumnLayoutEdit.setValue(settings.getMinPlayersForMultiColumnLayout());
    connect(&minPlayersForMultiColumnLayoutEdit, SIGNAL(valueChanged(int)), &settings,
            SLOT(setMinPlayersForMultiColumnLayout(int)));
    minPlayersForMultiColumnLayoutLabel.setBuddy(&minPlayersForMultiColumnLayoutEdit);

    connect(&maxFontSizeForCardsEdit, SIGNAL(valueChanged(int)), &settings, SLOT(setMaxFontSize(int)));
    maxFontSizeForCardsEdit.setValue(settings.getMaxFontSize());
    maxFontSizeForCardsLabel.setBuddy(&maxFontSizeForCardsEdit);
    maxFontSizeForCardsEdit.setMinimum(9);
    maxFontSizeForCardsEdit.setMaximum(100);

    auto *tableGrid = new QGridLayout;
    tableGrid->addWidget(&invertVerticalCoordinateCheckBox, 0, 0, 1, 2);
    tableGrid->addWidget(&minPlayersForMultiColumnLayoutLabel, 1, 0, 1, 1);
    tableGrid->addWidget(&minPlayersForMultiColumnLayoutEdit, 1, 1, 1, 1);
    tableGrid->addWidget(&maxFontSizeForCardsLabel, 2, 0, 1, 1);
    tableGrid->addWidget(&maxFontSizeForCardsEdit, 2, 1, 1, 1);

    tableGroupBox = new QGroupBox;
    tableGroupBox->setLayout(tableGrid);

    auto *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(themeGroupBox);
    mainLayout->addWidget(cardsGroupBox);
    mainLayout->addWidget(handGroupBox);
    mainLayout->addWidget(tableGroupBox);
    mainLayout->addStretch();

    setLayout(mainLayout);
}

void AppearanceSettingsPage::themeBoxChanged(int index)
{
    QStringList themeDirs = themeManager->getAvailableThemes().keys();
    if (index >= 0 && index < themeDirs.count())
        SettingsCache::instance().setThemeName(themeDirs.at(index));
}

void AppearanceSettingsPage::openThemeLocation()
{
    QString dir = SettingsCache::instance().getThemesPath();
    QDir dirDir = dir;
    dirDir.cdUp();
    // open if dir exists, create if parent dir does exist
    if (dirDir.exists() && dirDir.mkpath(dir)) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(dir));
    } else {
        QMessageBox::critical(this, tr("Error"), tr("Could not create themes directory at '%1'.").arg(dir));
    }
}

void AppearanceSettingsPage::retranslateUi()
{
    themeGroupBox->setTitle(tr("Theme settings"));
    themeLabel.setText(tr("Current theme:"));
    openThemeButton.setText(tr("Open themes folder"));

    cardsGroupBox->setTitle(tr("Card rendering"));
    displayCardNamesCheckBox.setText(tr("Display card names on cards having a picture"));
    cardScalingCheckBox.setText(tr("Scale cards on mouse over"));
    verticalCardOverlapPercentLabel.setText(
        tr("Minimum overlap percentage of cards on the stack and in vertical hand"));

    handGroupBox->setTitle(tr("Hand layout"));
    horizontalHandCheckBox.setText(tr("Display hand horizontally (wastes space)"));
    leftJustifiedHandCheckBox.setText(tr("Enable left justification"));

    tableGroupBox->setTitle(tr("Table grid layout"));
    invertVerticalCoordinateCheckBox.setText(tr("Invert vertical coordinate"));
    minPlayersForMultiColumnLayoutLabel.setText(tr("Minimum player count for multi-column layout:"));
    maxFontSizeForCardsLabel.setText(tr("Maximum font size for information displayed on cards:"));
}

UserInterfaceSettingsPage::UserInterfaceSettingsPage()
{
    // general settings and notification settings
    notificationsEnabledCheckBox.setChecked(SettingsCache::instance().getNotificationsEnabled());
    connect(&notificationsEnabledCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance(),
            &SettingsCache::setNotificationsEnabled);
    connect(&notificationsEnabledCheckBox, &QCheckBox::QT_STATE_CHANGED, this,
            &UserInterfaceSettingsPage::setNotificationEnabled);

    specNotificationsEnabledCheckBox.setChecked(SettingsCache::instance().getSpectatorNotificationsEnabled());
    specNotificationsEnabledCheckBox.setEnabled(SettingsCache::instance().getNotificationsEnabled());
    connect(&specNotificationsEnabledCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance(),
            &SettingsCache::setSpectatorNotificationsEnabled);

    buddyConnectNotificationsEnabledCheckBox.setChecked(
        SettingsCache::instance().getBuddyConnectNotificationsEnabled());
    buddyConnectNotificationsEnabledCheckBox.setEnabled(SettingsCache::instance().getNotificationsEnabled());
    connect(&buddyConnectNotificationsEnabledCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance(),
            &SettingsCache::setBuddyConnectNotificationsEnabled);

    doubleClickToPlayCheckBox.setChecked(SettingsCache::instance().getDoubleClickToPlay());
    connect(&doubleClickToPlayCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance(),
            &SettingsCache::setDoubleClickToPlay);

    playToStackCheckBox.setChecked(SettingsCache::instance().getPlayToStack());
    connect(&playToStackCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance(),
            &SettingsCache::setPlayToStack);

    annotateTokensCheckBox.setChecked(SettingsCache::instance().getAnnotateTokens());
    connect(&annotateTokensCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance(),
            &SettingsCache::setAnnotateTokens);

    useTearOffMenusCheckBox.setChecked(SettingsCache::instance().getUseTearOffMenus());
    connect(&useTearOffMenusCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance(),
            [](const QT_STATE_CHANGED_T state) { SettingsCache::instance().setUseTearOffMenus(state == Qt::Checked); });

    auto *generalGrid = new QGridLayout;
    generalGrid->addWidget(&doubleClickToPlayCheckBox, 0, 0);
    generalGrid->addWidget(&playToStackCheckBox, 1, 0);
    generalGrid->addWidget(&annotateTokensCheckBox, 2, 0);
    generalGrid->addWidget(&useTearOffMenusCheckBox, 3, 0);

    generalGroupBox = new QGroupBox;
    generalGroupBox->setLayout(generalGrid);

    auto *notificationsGrid = new QGridLayout;
    notificationsGrid->addWidget(&notificationsEnabledCheckBox, 0, 0);
    notificationsGrid->addWidget(&specNotificationsEnabledCheckBox, 1, 0);
    notificationsGrid->addWidget(&buddyConnectNotificationsEnabledCheckBox, 2, 0);

    notificationsGroupBox = new QGroupBox;
    notificationsGroupBox->setLayout(notificationsGrid);

    // animation settings
    tapAnimationCheckBox.setChecked(SettingsCache::instance().getTapAnimation());
    connect(&tapAnimationCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance(),
            &SettingsCache::setTapAnimation);

    auto *animationGrid = new QGridLayout;
    animationGrid->addWidget(&tapAnimationCheckBox, 0, 0);

    animationGroupBox = new QGroupBox;
    animationGroupBox->setLayout(animationGrid);

    // deck editor settings
    openDeckInNewTabCheckBox.setChecked(SettingsCache::instance().getOpenDeckInNewTab());
    connect(&openDeckInNewTabCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance(),
            &SettingsCache::setOpenDeckInNewTab);

    auto *deckEditorGrid = new QGridLayout;
    deckEditorGrid->addWidget(&openDeckInNewTabCheckBox, 0, 0);

    deckEditorGroupBox = new QGroupBox;
    deckEditorGroupBox->setLayout(deckEditorGrid);

    // putting it all together
    auto *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(generalGroupBox);
    mainLayout->addWidget(notificationsGroupBox);
    mainLayout->addWidget(animationGroupBox);
    mainLayout->addWidget(deckEditorGroupBox);
    mainLayout->addStretch();

    setLayout(mainLayout);
}

void UserInterfaceSettingsPage::setNotificationEnabled(QT_STATE_CHANGED_T i)
{
    specNotificationsEnabledCheckBox.setEnabled(i != 0);
    buddyConnectNotificationsEnabledCheckBox.setEnabled(i != 0);
    if (i == 0) {
        specNotificationsEnabledCheckBox.setChecked(false);
        buddyConnectNotificationsEnabledCheckBox.setChecked(false);
    }
}

void UserInterfaceSettingsPage::retranslateUi()
{
    generalGroupBox->setTitle(tr("General interface settings"));
    doubleClickToPlayCheckBox.setText(tr("&Double-click cards to play them (instead of single-click)"));
    playToStackCheckBox.setText(tr("&Play all nonlands onto the stack (not the battlefield) by default"));
    annotateTokensCheckBox.setText(tr("Annotate card text on tokens"));
    useTearOffMenusCheckBox.setText(tr("Use tear-off menus, allowing right click menus to persist on screen"));
    notificationsGroupBox->setTitle(tr("Notifications settings"));
    notificationsEnabledCheckBox.setText(tr("Enable notifications in taskbar"));
    specNotificationsEnabledCheckBox.setText(tr("Notify in the taskbar for game events while you are spectating"));
    buddyConnectNotificationsEnabledCheckBox.setText(tr("Notify in the taskbar when users in your buddy list connect"));
    animationGroupBox->setTitle(tr("Animation settings"));
    tapAnimationCheckBox.setText(tr("&Tap/untap animation"));
    deckEditorGroupBox->setTitle(tr("Deck editor settings"));
    openDeckInNewTabCheckBox.setText(tr("Always open deck in new tab"));
}

DeckEditorSettingsPage::DeckEditorSettingsPage()
{
    picDownloadCheckBox.setChecked(SettingsCache::instance().getPicDownload());
    connect(&picDownloadCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance(),
            &SettingsCache::setPicDownload);

    urlLinkLabel.setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    urlLinkLabel.setOpenExternalLinks(true);

    connect(&clearDownloadedPicsButton, SIGNAL(clicked()), this, SLOT(clearDownloadedPicsButtonClicked()));
    connect(&resetDownloadURLs, SIGNAL(clicked()), this, SLOT(resetDownloadedURLsButtonClicked()));

    auto *lpGeneralGrid = new QGridLayout;
    auto *lpSpoilerGrid = new QGridLayout;

    mcDownloadSpoilersCheckBox.setChecked(SettingsCache::instance().getDownloadSpoilersStatus());

    mpSpoilerSavePathLineEdit = new QLineEdit(SettingsCache::instance().getSpoilerCardDatabasePath());
    mpSpoilerSavePathLineEdit->setReadOnly(true);
    mpSpoilerPathButton = new QPushButton("...");
    connect(mpSpoilerPathButton, SIGNAL(clicked()), this, SLOT(spoilerPathButtonClicked()));

    updateNowButton = new QPushButton(tr("Update Spoilers"));
    updateNowButton->setFixedWidth(150);
    connect(updateNowButton, SIGNAL(clicked()), this, SLOT(updateSpoilers()));

    // Update the GUI depending on if the box is ticked or not
    setSpoilersEnabled(mcDownloadSpoilersCheckBox.isChecked());

    urlList = new QListWidget;
    urlList->setSelectionMode(QAbstractItemView::SingleSelection);
    urlList->setAlternatingRowColors(true);
    urlList->setDragEnabled(true);
    urlList->setDragDropMode(QAbstractItemView::InternalMove);
    connect(urlList->model(), SIGNAL(rowsMoved(const QModelIndex, int, int, const QModelIndex, int)), this,
            SLOT(urlListChanged(const QModelIndex, int, int, const QModelIndex, int)));

    urlList->addItems(SettingsCache::instance().downloads().getAllURLs());

    auto aAdd = new QAction(this);
    aAdd->setIcon(QPixmap("theme:icons/increment"));
    aAdd->setToolTip(tr("Add New URL"));
    connect(aAdd, SIGNAL(triggered()), this, SLOT(actAddURL()));
    auto aEdit = new QAction(this);
    aEdit->setIcon(QPixmap("theme:icons/pencil"));
    aEdit->setToolTip(tr("Edit URL"));
    connect(aEdit, SIGNAL(triggered()), this, SLOT(actEditURL()));
    auto aRemove = new QAction(this);
    aRemove->setIcon(QPixmap("theme:icons/decrement"));
    aRemove->setToolTip(tr("Remove URL"));
    connect(aRemove, SIGNAL(triggered()), this, SLOT(actRemoveURL()));

    auto *urlToolBar = new QToolBar;
    urlToolBar->setOrientation(Qt::Vertical);
    urlToolBar->addAction(aAdd);
    urlToolBar->addAction(aRemove);
    urlToolBar->addAction(aEdit);
    urlToolBar->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);

    auto *urlListLayout = new QHBoxLayout;
    urlListLayout->addWidget(urlToolBar);
    urlListLayout->addWidget(urlList);

    // pixmap cache
    pixmapCacheEdit.setMinimum(PIXMAPCACHE_SIZE_MIN);
    // 2047 is the max value to avoid overflowing of QPixmapCache::setCacheLimit(int size)
    pixmapCacheEdit.setMaximum(PIXMAPCACHE_SIZE_MAX);
    pixmapCacheEdit.setSingleStep(64);
    pixmapCacheEdit.setValue(SettingsCache::instance().getPixmapCacheSize());
    pixmapCacheEdit.setSuffix(" MB");

    networkCacheEdit.setMinimum(NETWORK_CACHE_SIZE_MIN);
    networkCacheEdit.setMaximum(NETWORK_CACHE_SIZE_MAX);
    networkCacheEdit.setSingleStep(1);
    networkCacheEdit.setValue(SettingsCache::instance().getNetworkCacheSizeInMB());
    networkCacheEdit.setSuffix(" MB");

    networkRedirectCacheTtlEdit.setMinimum(NETWORK_REDIRECT_CACHE_TTL_MIN);
    networkRedirectCacheTtlEdit.setMaximum(NETWORK_REDIRECT_CACHE_TTL_MAX);
    networkRedirectCacheTtlEdit.setSingleStep(1);
    networkRedirectCacheTtlEdit.setValue(SettingsCache::instance().getRedirectCacheTtl());
    networkRedirectCacheTtlEdit.setSuffix(" " + tr("Day(s)"));

    auto networkCacheLayout = new QHBoxLayout;
    networkCacheLayout->addStretch();
    networkCacheLayout->addWidget(&networkCacheLabel);
    networkCacheLayout->addWidget(&networkCacheEdit);

    auto networkRedirectCacheLayout = new QHBoxLayout;
    networkRedirectCacheLayout->addStretch();
    networkRedirectCacheLayout->addWidget(&networkRedirectCacheTtlLabel);
    networkRedirectCacheLayout->addWidget(&networkRedirectCacheTtlEdit);

    auto pixmapCacheLayout = new QHBoxLayout;
    pixmapCacheLayout->addStretch();
    pixmapCacheLayout->addWidget(&pixmapCacheLabel);
    pixmapCacheLayout->addWidget(&pixmapCacheEdit);

    // Top Layout
    lpGeneralGrid->addWidget(&picDownloadCheckBox, 0, 0);
    lpGeneralGrid->addWidget(&resetDownloadURLs, 0, 1);
    lpGeneralGrid->addLayout(urlListLayout, 1, 0, 1, 2);
    lpGeneralGrid->addLayout(networkCacheLayout, 2, 0, 1, 2);
    lpGeneralGrid->addLayout(pixmapCacheLayout, 3, 0, 1, 2);
    lpGeneralGrid->addLayout(networkRedirectCacheLayout, 4, 0, 1, 2);
    lpGeneralGrid->addWidget(&urlLinkLabel, 5, 0);
    lpGeneralGrid->addWidget(&clearDownloadedPicsButton, 5, 1);

    // Spoiler Layout
    lpSpoilerGrid->addWidget(&mcDownloadSpoilersCheckBox, 0, 0);
    lpSpoilerGrid->addWidget(&mcSpoilerSaveLabel, 1, 0);
    lpSpoilerGrid->addWidget(mpSpoilerSavePathLineEdit, 1, 1);
    lpSpoilerGrid->addWidget(mpSpoilerPathButton, 1, 2);
    lpSpoilerGrid->addWidget(&lastUpdatedLabel, 2, 0);
    lpSpoilerGrid->addWidget(updateNowButton, 2, 1);
    lpSpoilerGrid->addWidget(&infoOnSpoilersLabel, 3, 0, 1, 3, Qt::AlignTop);

    // On a change to the checkbox, hide/un-hide the other fields
    connect(&mcDownloadSpoilersCheckBox, SIGNAL(toggled(bool)), &SettingsCache::instance(),
            SLOT(setDownloadSpoilerStatus(bool)));
    connect(&mcDownloadSpoilersCheckBox, SIGNAL(toggled(bool)), this, SLOT(setSpoilersEnabled(bool)));
    connect(&pixmapCacheEdit, SIGNAL(valueChanged(int)), &SettingsCache::instance(), SLOT(setPixmapCacheSize(int)));
    connect(&networkCacheEdit, SIGNAL(valueChanged(int)), &SettingsCache::instance(),
            SLOT(setNetworkCacheSizeInMB(int)));
    connect(&networkRedirectCacheTtlEdit, SIGNAL(valueChanged(int)), &SettingsCache::instance(),
            SLOT(setNetworkRedirectCacheTtl(int)));

    mpGeneralGroupBox = new QGroupBox;
    mpGeneralGroupBox->setLayout(lpGeneralGrid);

    mpSpoilerGroupBox = new QGroupBox;
    mpSpoilerGroupBox->setLayout(lpSpoilerGrid);

    auto *lpMainLayout = new QVBoxLayout;
    lpMainLayout->addWidget(mpGeneralGroupBox);
    lpMainLayout->addWidget(mpSpoilerGroupBox);

    setLayout(lpMainLayout);
}

void DeckEditorSettingsPage::resetDownloadedURLsButtonClicked()
{
    SettingsCache::instance().downloads().resetToDefaultURLs();
    urlList->clear();
    urlList->addItems(SettingsCache::instance().downloads().getAllURLs());
    QMessageBox::information(this, tr("Success"), tr("Download URLs have been reset."));
}

void DeckEditorSettingsPage::clearDownloadedPicsButtonClicked()
{
    PictureLoader::clearNetworkCache();

    // These are not used anymore, but we don't delete them automatically, so
    // we should do it here lest we leave pictures hanging around on users'
    // machines.
    QString picsPath = SettingsCache::instance().getPicsPath() + "/downloadedPics/";
    QStringList dirs = QDir(picsPath).entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
    bool outerSuccessRemove = true;
    for (const auto &dir : dirs) {
        QString currentPath = picsPath + dir + "/";
        QStringList files = QDir(currentPath).entryList(QDir::Files);
        bool innerSuccessRemove = true;
        for (int j = 0; j < files.length(); j++) {
            if (!QDir(currentPath).remove(files.at(j))) {
                qInfo() << "Failed to remove " + currentPath.toUtf8() + files.at(j).toUtf8();
                outerSuccessRemove = false;
                innerSuccessRemove = false;
            }
            qInfo() << "Removed " << currentPath << files.at(j);
        }

        if (innerSuccessRemove) {
            bool success = QDir(picsPath).rmdir(dir);
            if (!success) {
                qInfo() << "Failed to remove inner directory" << picsPath;
            } else {
                qInfo() << "Removed" << currentPath;
            }
        }
    }
    if (outerSuccessRemove) {
        QMessageBox::information(this, tr("Success"), tr("Downloaded card pictures have been reset."));
        QDir(SettingsCache::instance().getPicsPath()).rmdir("downloadedPics");
    } else {
        QMessageBox::critical(this, tr("Error"), tr("One or more downloaded card pictures could not be cleared."));
    }
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
    connect(sbu, SIGNAL(spoilerCheckerDone()), this, SLOT(unlockSettings()));
    connect(sbu, SIGNAL(spoilersUpdatedSuccessfully()), this, SLOT(unlockSettings()));
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
    clearDownloadedPicsButton.setText(tr("Delete Downloaded Images"));
    resetDownloadURLs.setText(tr("Reset Download URLs"));
    networkCacheLabel.setText(tr("Network Cache Size:"));
    networkCacheEdit.setToolTip(tr("On-disk cache for downloaded pictures"));
    networkRedirectCacheTtlLabel.setText(tr("Redirect Cache TTL:"));
    networkRedirectCacheTtlEdit.setToolTip(tr("How long cached redirects for urls are valid for."));
    pixmapCacheLabel.setText(tr("Picture Cache Size:"));
    pixmapCacheEdit.setToolTip(tr("In-memory cache for pictures not currently on screen"));
}

MessagesSettingsPage::MessagesSettingsPage()
{
    chatMentionCheckBox.setChecked(SettingsCache::instance().getChatMention());
    connect(&chatMentionCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance(),
            &SettingsCache::setChatMention);

    chatMentionCompleterCheckbox.setChecked(SettingsCache::instance().getChatMentionCompleter());
    connect(&chatMentionCompleterCheckbox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance(),
            &SettingsCache::setChatMentionCompleter);

    explainMessagesLabel.setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    explainMessagesLabel.setOpenExternalLinks(true);

    ignoreUnregUsersMainChat.setChecked(SettingsCache::instance().getIgnoreUnregisteredUsers());
    ignoreUnregUserMessages.setChecked(SettingsCache::instance().getIgnoreUnregisteredUserMessages());
    connect(&ignoreUnregUsersMainChat, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance(),
            &SettingsCache::setIgnoreUnregisteredUsers);
    connect(&ignoreUnregUserMessages, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance(),
            &SettingsCache::setIgnoreUnregisteredUserMessages);

    invertMentionForeground.setChecked(SettingsCache::instance().getChatMentionForeground());
    connect(&invertMentionForeground, &QCheckBox::QT_STATE_CHANGED, this, &MessagesSettingsPage::updateTextColor);

    invertHighlightForeground.setChecked(SettingsCache::instance().getChatHighlightForeground());
    connect(&invertHighlightForeground, &QCheckBox::QT_STATE_CHANGED, this,
            &MessagesSettingsPage::updateTextHighlightColor);

    mentionColor = new QLineEdit();
    mentionColor->setText(SettingsCache::instance().getChatMentionColor());
    updateMentionPreview();
    connect(mentionColor, SIGNAL(textChanged(QString)), this, SLOT(updateColor(QString)));

    messagePopups.setChecked(SettingsCache::instance().getShowMessagePopup());
    connect(&messagePopups, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance(),
            &SettingsCache::setShowMessagePopups);

    mentionPopups.setChecked(SettingsCache::instance().getShowMentionPopup());
    connect(&mentionPopups, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance(),
            &SettingsCache::setShowMentionPopups);

    roomHistory.setChecked(SettingsCache::instance().getRoomHistory());
    connect(&roomHistory, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance(), &SettingsCache::setRoomHistory);

    customAlertString = new QLineEdit();
    customAlertString->setPlaceholderText(tr("Word1 Word2 Word3"));
    customAlertString->setText(SettingsCache::instance().getHighlightWords());
    connect(customAlertString, SIGNAL(textChanged(QString)), &SettingsCache::instance(),
            SLOT(setHighlightWords(QString)));

    auto *chatGrid = new QGridLayout;
    chatGrid->addWidget(&chatMentionCheckBox, 0, 0);
    chatGrid->addWidget(&invertMentionForeground, 0, 1);
    chatGrid->addWidget(mentionColor, 0, 2);
    chatGrid->addWidget(&chatMentionCompleterCheckbox, 1, 0);
    chatGrid->addWidget(&ignoreUnregUsersMainChat, 2, 0);
    chatGrid->addWidget(&hexLabel, 1, 2);
    chatGrid->addWidget(&ignoreUnregUserMessages, 3, 0);
    chatGrid->addWidget(&messagePopups, 4, 0);
    chatGrid->addWidget(&mentionPopups, 5, 0);
    chatGrid->addWidget(&roomHistory, 6, 0);
    chatGroupBox = new QGroupBox;
    chatGroupBox->setLayout(chatGrid);

    highlightColor = new QLineEdit();
    highlightColor->setText(SettingsCache::instance().getChatHighlightColor());
    updateHighlightPreview();
    connect(highlightColor, SIGNAL(textChanged(QString)), this, SLOT(updateHighlightColor(QString)));

    auto *highlightNotice = new QGridLayout;
    highlightNotice->addWidget(highlightColor, 0, 2);
    highlightNotice->addWidget(&invertHighlightForeground, 0, 1);
    highlightNotice->addWidget(&hexHighlightLabel, 1, 2);
    highlightNotice->addWidget(customAlertString, 0, 0);
    highlightNotice->addWidget(&customAlertStringLabel, 1, 0);
    highlightGroupBox = new QGroupBox;
    highlightGroupBox->setLayout(highlightNotice);

    messageList = new QListWidget;

    int count = SettingsCache::instance().messages().getCount();
    for (int i = 0; i < count; i++)
        messageList->addItem(SettingsCache::instance().messages().getMessageAt(i));

    aAdd = new QAction(this);
    aAdd->setIcon(QPixmap("theme:icons/increment"));
    aAdd->setToolTip(tr("Add New Message"));
    connect(aAdd, SIGNAL(triggered()), this, SLOT(actAdd()));
    aEdit = new QAction(this);
    aEdit->setIcon(QPixmap("theme:icons/pencil"));
    aEdit->setToolTip(tr("Edit Message"));
    connect(aEdit, SIGNAL(triggered()), this, SLOT(actEdit()));
    aRemove = new QAction(this);
    aRemove->setIcon(QPixmap("theme:icons/decrement"));
    aRemove->setToolTip(tr("Remove Message"));
    connect(aRemove, SIGNAL(triggered()), this, SLOT(actRemove()));

    auto *messageToolBar = new QToolBar;
    messageToolBar->setOrientation(Qt::Vertical);
    messageToolBar->addAction(aAdd);
    messageToolBar->addAction(aRemove);
    messageToolBar->addAction(aEdit);
    messageToolBar->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);

    auto *messageListLayout = new QHBoxLayout;
    messageListLayout->addWidget(messageToolBar);
    messageListLayout->addWidget(messageList);

    auto *messagesLayout = new QVBoxLayout; // combines the explainer label with the actual messages widget pieces
    messagesLayout->addLayout(messageListLayout);
    messagesLayout->addWidget(&explainMessagesLabel);

    messageGroupBox = new QGroupBox; // draws a box around the above layout and allows it to be titled
    messageGroupBox->setLayout(messagesLayout);

    auto *mainLayout = new QVBoxLayout; // combines the messages groupbox with the rest of the menu
    mainLayout->addWidget(messageGroupBox);
    mainLayout->addWidget(chatGroupBox);
    mainLayout->addWidget(highlightGroupBox);

    setLayout(mainLayout);

    retranslateUi();
}

void MessagesSettingsPage::updateColor(const QString &value)
{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 4, 0))
    QColor colorToSet = QColor::fromString("#" + value);
#else
    QColor colorToSet;
    colorToSet.setNamedColor("#" + value);
#endif
    if (colorToSet.isValid()) {
        SettingsCache::instance().setChatMentionColor(value);
        updateMentionPreview();
    }
}

void MessagesSettingsPage::updateHighlightColor(const QString &value)
{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 4, 0))
    QColor colorToSet = QColor::fromString("#" + value);
#else
    QColor colorToSet;
    colorToSet.setNamedColor("#" + value);
#endif
    if (colorToSet.isValid()) {
        SettingsCache::instance().setChatHighlightColor(value);
        updateHighlightPreview();
    }
}

void MessagesSettingsPage::updateTextColor(QT_STATE_CHANGED_T value)
{
    SettingsCache::instance().setChatMentionForeground(value);
    updateMentionPreview();
}

void MessagesSettingsPage::updateTextHighlightColor(QT_STATE_CHANGED_T value)
{
    SettingsCache::instance().setChatHighlightForeground(value);
    updateHighlightPreview();
}

void MessagesSettingsPage::updateMentionPreview()
{
    mentionColor->setStyleSheet(
        "QLineEdit{background:#" + SettingsCache::instance().getChatMentionColor() +
        ";color: " + (SettingsCache::instance().getChatMentionForeground() ? "white" : "black") + ";}");
}

void MessagesSettingsPage::updateHighlightPreview()
{
    highlightColor->setStyleSheet(
        "QLineEdit{background:#" + SettingsCache::instance().getChatHighlightColor() +
        ";color: " + (SettingsCache::instance().getChatHighlightForeground() ? "white" : "black") + ";}");
}

void MessagesSettingsPage::storeSettings()
{
    SettingsCache::instance().messages().setCount(messageList->count());
    for (int i = 0; i < messageList->count(); i++)
        SettingsCache::instance().messages().setMessageAt(i, messageList->item(i)->text());
    emit SettingsCache::instance().messages().messageMacrosChanged();
}

void MessagesSettingsPage::actAdd()
{
    bool ok;
    QString msg =
        getTextWithMax(this, tr("Add message"), tr("Message:"), QLineEdit::Normal, QString(), &ok, MAX_TEXT_LENGTH);
    if (ok) {
        messageList->addItem(msg);
        storeSettings();
    }
}

void MessagesSettingsPage::actEdit()
{
    if (messageList->currentItem()) {
        QString oldText = messageList->currentItem()->text();
        bool ok;
        QString msg =
            getTextWithMax(this, tr("Edit message"), tr("Message:"), QLineEdit::Normal, oldText, &ok, MAX_TEXT_LENGTH);
        if (ok) {
            messageList->currentItem()->setText(msg);
            storeSettings();
        }
    }
}

void MessagesSettingsPage::actRemove()
{
    if (messageList->currentItem() != nullptr) {
        delete messageList->takeItem(messageList->currentRow());
        storeSettings();
    }
}

void MessagesSettingsPage::retranslateUi()
{
    chatGroupBox->setTitle(tr("Chat settings"));
    highlightGroupBox->setTitle(tr("Custom alert words"));
    chatMentionCheckBox.setText(tr("Enable chat mentions"));
    chatMentionCompleterCheckbox.setText(tr("Enable mention completer"));
    messageGroupBox->setTitle(tr("In-game message macros"));
    explainMessagesLabel.setText(
        QString("<a href='%1'>%2</a>").arg(WIKI_CUSTOM_SHORTCUTS).arg(tr("How to use in-game message macros")));
    ignoreUnregUsersMainChat.setText(tr("Ignore chat room messages sent by unregistered users"));
    ignoreUnregUserMessages.setText(tr("Ignore private messages sent by unregistered users"));
    invertMentionForeground.setText(tr("Invert text color"));
    invertHighlightForeground.setText(tr("Invert text color"));
    messagePopups.setText(tr("Enable desktop notifications for private messages"));
    mentionPopups.setText(tr("Enable desktop notification for mentions"));
    roomHistory.setText(tr("Enable room message history on join"));
    hexLabel.setText(tr("(Color is hexadecimal)"));
    hexHighlightLabel.setText(tr("(Color is hexadecimal)"));
    customAlertStringLabel.setText(tr("Separate words with a space, alphanumeric characters only"));
}

SoundSettingsPage::SoundSettingsPage()
{
    soundEnabledCheckBox.setChecked(SettingsCache::instance().getSoundEnabled());
    connect(&soundEnabledCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance(),
            &SettingsCache::setSoundEnabled);

    QString themeName = SettingsCache::instance().getSoundThemeName();

    QStringList themeDirs = soundEngine->getAvailableThemes().keys();
    for (int i = 0; i < themeDirs.size(); i++) {
        themeBox.addItem(themeDirs[i]);
        if (themeDirs[i] == themeName)
            themeBox.setCurrentIndex(i);
    }

    connect(&themeBox, SIGNAL(currentIndexChanged(int)), this, SLOT(themeBoxChanged(int)));
    connect(&soundTestButton, SIGNAL(clicked()), soundEngine, SLOT(testSound()));

    masterVolumeSlider = new QSlider(Qt::Horizontal);
    masterVolumeSlider->setMinimum(0);
    masterVolumeSlider->setMaximum(100);
    masterVolumeSlider->setValue(SettingsCache::instance().getMasterVolume());
    masterVolumeSlider->setToolTip(QString::number(SettingsCache::instance().getMasterVolume()));
    connect(&SettingsCache::instance(), SIGNAL(masterVolumeChanged(int)), this, SLOT(masterVolumeChanged(int)));
    connect(masterVolumeSlider, SIGNAL(sliderReleased()), soundEngine, SLOT(testSound()));
    connect(masterVolumeSlider, SIGNAL(valueChanged(int)), &SettingsCache::instance(), SLOT(setMasterVolume(int)));

    masterVolumeSpinBox = new QSpinBox();
    masterVolumeSpinBox->setMinimum(0);
    masterVolumeSpinBox->setMaximum(100);
    masterVolumeSpinBox->setValue(SettingsCache::instance().getMasterVolume());
    connect(masterVolumeSlider, SIGNAL(valueChanged(int)), masterVolumeSpinBox, SLOT(setValue(int)));
    connect(masterVolumeSpinBox, SIGNAL(valueChanged(int)), masterVolumeSlider, SLOT(setValue(int)));

    auto *soundGrid = new QGridLayout;
    soundGrid->addWidget(&soundEnabledCheckBox, 0, 0, 1, 3);
    soundGrid->addWidget(&masterVolumeLabel, 1, 0);
    soundGrid->addWidget(masterVolumeSlider, 1, 1);
    soundGrid->addWidget(masterVolumeSpinBox, 1, 2);
    soundGrid->addWidget(&themeLabel, 2, 0);
    soundGrid->addWidget(&themeBox, 2, 1);
    soundGrid->addWidget(&soundTestButton, 3, 1);

    soundGroupBox = new QGroupBox;
    soundGroupBox->setLayout(soundGrid);

    auto *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(soundGroupBox);
    mainLayout->addStretch();

    setLayout(mainLayout);
}

void SoundSettingsPage::themeBoxChanged(int index)
{
    QStringList themeDirs = soundEngine->getAvailableThemes().keys();
    if (index >= 0 && index < themeDirs.count())
        SettingsCache::instance().setSoundThemeName(themeDirs.at(index));
}

void SoundSettingsPage::masterVolumeChanged(int value)
{
    masterVolumeSlider->setToolTip(QString::number(value));
}

void SoundSettingsPage::retranslateUi()
{
    soundEnabledCheckBox.setText(tr("Enable &sounds"));
    themeLabel.setText(tr("Current sounds theme:"));
    soundTestButton.setText(tr("Test system sound engine"));
    soundGroupBox->setTitle(tr("Sound settings"));
    masterVolumeLabel.setText(tr("Master volume"));
}

ShortcutSettingsPage::ShortcutSettingsPage()
{
    // table
    shortcutsTable = new QTreeWidget();
    shortcutsTable->setColumnCount(2);
    shortcutsTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    shortcutsTable->setUniformRowHeights(true);
    shortcutsTable->setAlternatingRowColors(true);
    shortcutsTable->header()->resizeSection(0, shortcutsTable->width() / 3 * 2);

    // edit widget
    currentActionGroupLabel = new QLabel(this);
    currentActionGroupName = new QLabel(this);
    currentActionLabel = new QLabel(this);
    currentActionName = new QLabel(this);
    currentShortcutLabel = new QLabel(this);
    editTextBox = new SequenceEdit("", this);
    shortcutsTable->installEventFilter(editTextBox);

    // buttons
    faqLabel = new QLabel(this);
    faqLabel->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    faqLabel->setOpenExternalLinks(true);

    btnResetAll = new QPushButton(this);
    btnClearAll = new QPushButton(this);

    btnResetAll->setIcon(QPixmap("theme:icons/update"));
    btnClearAll->setIcon(QPixmap("theme:icons/clearsearch"));

    // layout
    auto *_editLayout = new QGridLayout;
    _editLayout->addWidget(currentActionGroupLabel, 0, 0);
    _editLayout->addWidget(currentActionGroupName, 0, 1);
    _editLayout->addWidget(currentActionLabel, 1, 0);
    _editLayout->addWidget(currentActionName, 1, 1);
    _editLayout->addWidget(currentShortcutLabel, 2, 0);
    _editLayout->addWidget(editTextBox, 2, 1);

    editShortcutGroupBox = new QGroupBox;
    editShortcutGroupBox->setLayout(_editLayout);

    auto *_buttonsLayout = new QHBoxLayout;
    _buttonsLayout->addWidget(faqLabel);
    _buttonsLayout->addWidget(btnResetAll);
    _buttonsLayout->addWidget(btnClearAll);

    auto *_mainLayout = new QVBoxLayout;
    _mainLayout->addWidget(shortcutsTable);
    _mainLayout->addWidget(editShortcutGroupBox);
    _mainLayout->addLayout(_buttonsLayout);

    setLayout(_mainLayout);

    connect(btnResetAll, SIGNAL(clicked()), this, SLOT(resetShortcuts()));
    connect(btnClearAll, SIGNAL(clicked()), this, SLOT(clearShortcuts()));
    connect(shortcutsTable, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this,
            SLOT(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)));
    connect(&SettingsCache::instance().shortcuts(), SIGNAL(shortCutChanged()), this, SLOT(refreshShortcuts()));

    createShortcuts();
}

void ShortcutSettingsPage::currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem * /*previous */)
{
    if (current == nullptr) {
        currentActionGroupName->setText("");
        currentActionName->setText("");
        editTextBox->setShortcutName("");
    } else {
        QString key = current->data(2, Qt::DisplayRole).toString();
        QString group = SettingsCache::instance().shortcuts().getShortcut(key).getGroupName();
        QString action = SettingsCache::instance().shortcuts().getShortcut(key).getName();
        currentActionGroupName->setText(group);
        currentActionName->setText(action);
        editTextBox->setShortcutName(key);
    }
}

void ShortcutSettingsPage::resetShortcuts()
{
    if (QMessageBox::question(this, tr("Restore all default shortcuts"),
                              tr("Do you really want to restore all default shortcuts?")) == QMessageBox::Yes) {
        SettingsCache::instance().shortcuts().resetAllShortcuts();
    }
}

void ShortcutSettingsPage::createShortcuts()
{
    QHash<QString, QTreeWidgetItem *> parentItems;
    QTreeWidgetItem *curParent = nullptr;
    for (const auto &key : SettingsCache::instance().shortcuts().getAllShortcutKeys()) {
        QString name = SettingsCache::instance().shortcuts().getShortcut(key).getName();
        QString group = SettingsCache::instance().shortcuts().getShortcut(key).getGroupName();
        QString shortcut = SettingsCache::instance().shortcuts().getShortcutString(key);

        if (parentItems.contains(group)) {
            curParent = parentItems.value(group);
        } else {
            curParent = new QTreeWidgetItem((QTreeWidget *)nullptr, QStringList({group, "", ""}));
            static QFont font = curParent->font(0);
            font.setBold(true);
            curParent->setFont(0, font);
            parentItems.insert(group, curParent);
        }

        new QTreeWidgetItem(curParent, QStringList({name, shortcut, key}));
    }
    shortcutsTable->clear();
    shortcutsTable->insertTopLevelItems(0, parentItems.values());
    shortcutsTable->setCurrentItem(nullptr);
    shortcutsTable->expandAll();
    shortcutsTable->sortItems(0, Qt::AscendingOrder);
}

void ShortcutSettingsPage::refreshShortcuts()
{
    QTreeWidgetItem *curParent = nullptr;
    QTreeWidgetItem *curChild = nullptr;
    for (int i = 0; i < shortcutsTable->topLevelItemCount(); ++i) {
        curParent = shortcutsTable->topLevelItem(i);
        for (int j = 0; j < curParent->childCount(); ++j) {
            curChild = curParent->child(j);
            QString key = curChild->data(2, Qt::DisplayRole).toString();
            QString name = SettingsCache::instance().shortcuts().getShortcut(key).getName();
            QString shortcut = SettingsCache::instance().shortcuts().getShortcutString(key);
            curChild->setText(0, name);
            curChild->setText(1, shortcut);

            if (j == 0) {
                // the first child also updates the parent's group name
                QString group = SettingsCache::instance().shortcuts().getShortcut(key).getGroupName();
                curParent->setText(0, group);
            }
        }
    }
    shortcutsTable->sortItems(0, Qt::AscendingOrder);
    currentItemChanged(shortcutsTable->currentItem(), nullptr);
}

void ShortcutSettingsPage::clearShortcuts()
{
    if (QMessageBox::question(this, tr("Clear all default shortcuts"),
                              tr("Do you really want to clear all shortcuts?")) == QMessageBox::Yes) {
        SettingsCache::instance().shortcuts().clearAllShortcuts();
    }
}

void ShortcutSettingsPage::retranslateUi()
{
    shortcutsTable->setHeaderLabels(QStringList() << tr("Action") << tr("Shortcut"));
    refreshShortcuts();

    currentActionGroupLabel->setText(tr("Section:"));
    currentActionLabel->setText(tr("Action:"));
    currentShortcutLabel->setText(tr("Shortcut:"));
    editTextBox->retranslateUi();
    faqLabel->setText(QString("<a href='%1'>%2</a>").arg(WIKI_CUSTOM_SHORTCUTS).arg(tr("How to set custom shortcuts")));
    btnResetAll->setText(tr("Restore all default shortcuts"));
    btnClearAll->setText(tr("Clear all shortcuts"));
}

DlgSettings::DlgSettings(QWidget *parent) : QDialog(parent)
{
    auto rec = QGuiApplication::primaryScreen()->availableGeometry();
    this->setMinimumSize(qMin(700, rec.width()), qMin(700, rec.height()));

    connect(&SettingsCache::instance(), SIGNAL(langChanged()), this, SLOT(updateLanguage()));

    contentsWidget = new QListWidget;
    contentsWidget->setViewMode(QListView::IconMode);
    contentsWidget->setIconSize(QSize(58, 50));
    contentsWidget->setMovement(QListView::Static);
    contentsWidget->setMinimumHeight(85);
    contentsWidget->setMaximumHeight(85);
    contentsWidget->setSpacing(5);

    pagesWidget = new QStackedWidget;
    pagesWidget->addWidget(new GeneralSettingsPage);
    pagesWidget->addWidget(new AppearanceSettingsPage);
    pagesWidget->addWidget(new UserInterfaceSettingsPage);
    pagesWidget->addWidget(new DeckEditorSettingsPage);
    pagesWidget->addWidget(new MessagesSettingsPage);
    pagesWidget->addWidget(new SoundSettingsPage);
    pagesWidget->addWidget(new ShortcutSettingsPage);

    createIcons();
    contentsWidget->setCurrentRow(0);

    auto *vboxLayout = new QVBoxLayout;
    vboxLayout->addWidget(contentsWidget);
    vboxLayout->addWidget(pagesWidget);

    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(close()));

    auto *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(vboxLayout);
    mainLayout->addSpacing(2);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    retranslateUi();

    adjustSize();
}

void DlgSettings::createIcons()
{
    generalButton = new QListWidgetItem(contentsWidget);
    generalButton->setTextAlignment(Qt::AlignHCenter);
    generalButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    generalButton->setIcon(QPixmap("theme:config/general"));

    appearanceButton = new QListWidgetItem(contentsWidget);
    appearanceButton->setTextAlignment(Qt::AlignHCenter);
    appearanceButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    appearanceButton->setIcon(QPixmap("theme:config/appearance"));

    userInterfaceButton = new QListWidgetItem(contentsWidget);
    userInterfaceButton->setTextAlignment(Qt::AlignHCenter);
    userInterfaceButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    userInterfaceButton->setIcon(QPixmap("theme:config/interface"));

    deckEditorButton = new QListWidgetItem(contentsWidget);
    deckEditorButton->setTextAlignment(Qt::AlignHCenter);
    deckEditorButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    deckEditorButton->setIcon(QPixmap("theme:config/deckeditor"));

    messagesButton = new QListWidgetItem(contentsWidget);
    messagesButton->setTextAlignment(Qt::AlignHCenter);
    messagesButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    messagesButton->setIcon(QPixmap("theme:config/messages"));

    soundButton = new QListWidgetItem(contentsWidget);
    soundButton->setTextAlignment(Qt::AlignHCenter);
    soundButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    soundButton->setIcon(QPixmap("theme:config/sound"));

    shortcutsButton = new QListWidgetItem(contentsWidget);
    shortcutsButton->setTextAlignment(Qt::AlignHCenter);
    shortcutsButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    shortcutsButton->setIcon(QPixmap("theme:config/shorcuts"));

    connect(contentsWidget, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)), this,
            SLOT(changePage(QListWidgetItem *, QListWidgetItem *)));
}

void DlgSettings::changePage(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (!current)
        current = previous;

    pagesWidget->setCurrentIndex(contentsWidget->row(current));
}

void DlgSettings::setTab(int index)
{
    if (index <= contentsWidget->count() - 1 && index >= 0) {
        changePage(contentsWidget->item(index), contentsWidget->currentItem());
        contentsWidget->setCurrentRow(index);
    }
}

void DlgSettings::updateLanguage()
{
    qApp->removeTranslator(translator); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    installNewTranslator();
}

void DlgSettings::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
        retranslateUi();
    QDialog::changeEvent(event);
}

void DlgSettings::closeEvent(QCloseEvent *event)
{
    bool showLoadError = true;
    QString loadErrorMessage = tr("Unknown Error loading card database");
    LoadStatus loadStatus = CardDatabaseManager::getInstance()->getLoadStatus();
    qDebug() << "Card Database load status: " << loadStatus;
    switch (loadStatus) {
        case Ok:
            showLoadError = false;
            break;
        case Invalid:
            loadErrorMessage = tr("Your card database is invalid.\n\n"
                                  "Cockatrice may not function correctly with an invalid database\n\n"
                                  "You may need to rerun oracle to update your card database.\n\n"
                                  "Would you like to change your database location setting?");
            break;
        case VersionTooOld:
            loadErrorMessage = tr("Your card database version is too old.\n\n"
                                  "This can cause problems loading card information or images\n\n"
                                  "Usually this can be fixed by rerunning oracle to to update your card database.\n\n"
                                  "Would you like to change your database location setting?");
            break;
        case NotLoaded:
            loadErrorMessage = tr("Your card database did not finish loading\n\n"
                                  "Please file a ticket at https://github.com/Cockatrice/Cockatrice/issues with your "
                                  "cards.xml attached\n\n"
                                  "Would you like to change your database location setting?");
            break;
        case FileError:
            loadErrorMessage = tr("File Error loading your card database.\n\n"
                                  "Would you like to change your database location setting?");
            break;
        case NoCards:
            loadErrorMessage = tr("Your card database was loaded but contains no cards.\n\n"
                                  "Would you like to change your database location setting?");
            break;
        default:
            loadErrorMessage = tr("Unknown card database load status\n\n"
                                  "Please file a ticket at https://github.com/Cockatrice/Cockatrice/issues\n\n"
                                  "Would you like to change your database location setting?");

            break;
    }

    if (showLoadError) {
        if (QMessageBox::critical(this, tr("Error"), loadErrorMessage, QMessageBox::Yes | QMessageBox::No) ==
            QMessageBox::Yes) {
            event->ignore();
            return;
        }
    }

    if (!QDir(SettingsCache::instance().getDeckPath()).exists() || SettingsCache::instance().getDeckPath().isEmpty()) {
        // TODO: Prompt to create it
        if (QMessageBox::critical(
                this, tr("Error"),
                tr("The path to your deck directory is invalid. Would you like to go back and set the correct path?"),
                QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            event->ignore();
            return;
        }
    }

    if (!QDir(SettingsCache::instance().getPicsPath()).exists() || SettingsCache::instance().getPicsPath().isEmpty()) {
        // TODO: Prompt to create it
        if (QMessageBox::critical(this, tr("Error"),
                                  tr("The path to your card pictures directory is invalid. Would you like to go back "
                                     "and set the correct path?"),
                                  QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            event->ignore();
            return;
        }
    }
    event->accept();
}

void DlgSettings::retranslateUi()
{
    setWindowTitle(tr("Settings"));

    generalButton->setText(tr("General"));
    appearanceButton->setText(tr("Appearance"));
    userInterfaceButton->setText(tr("User Interface"));
    deckEditorButton->setText(tr("Card Sources"));
    messagesButton->setText(tr("Chat"));
    soundButton->setText(tr("Sound"));
    shortcutsButton->setText(tr("Shortcuts"));

    for (int i = 0; i < pagesWidget->count(); i++)
        dynamic_cast<AbstractSettingsPage *>(pagesWidget->widget(i))->retranslateUi();

    contentsWidget->reset();
}
