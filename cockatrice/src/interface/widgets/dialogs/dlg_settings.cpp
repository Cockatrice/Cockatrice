#include "dlg_settings.h"

#include "../client/network/update/card_spoiler/spoiler_background_updater.h"
#include "../client/network/update/client/release_channel.h"
#include "../client/sound_engine.h"
#include "../interface/card_picture_loader/card_picture_loader.h"
#include "../interface/theme_manager.h"
#include "../interface/widgets/general/background_sources.h"
#include "../interface/widgets/tabs/tab_supervisor.h"
#include "../interface/widgets/utility/custom_line_edit.h"
#include "../interface/widgets/utility/get_text_with_max.h"
#include "../interface/widgets/utility/sequence_edit.h"
#include "../main.h"

#include <QAbstractButton>
#include <QAbstractListModel>
#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QCloseEvent>
#include <QColorDialog>
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
#include <QScrollArea>
#include <QScrollBar>
#include <QSlider>
#include <QSpinBox>
#include <QStackedWidget>
#include <QToolBar>
#include <QTranslator>
#include <QVariant>
#include <libcockatrice/card/database/card_database.h>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/settings/cache_settings.h>
#include <libcockatrice/settings/card_counter_settings.h>
#include <libcockatrice/settings/shortcut_treeview.h>

#define WIKI_CUSTOM_PIC_URL "https://github.com/Cockatrice/Cockatrice/wiki/Custom-Picture-Download-URLs"
#define WIKI_CUSTOM_SHORTCUTS "https://github.com/Cockatrice/Cockatrice/wiki/Custom-Keyboard-Shortcuts"
#define WIKI_TRANSLATION_FAQ "https://github.com/Cockatrice/Cockatrice/wiki/Translation-FAQ"

enum startupCardUpdateCheckBehaviorIndex
{
    startupCardUpdateCheckBehaviorIndexNone,
    startupCardUpdateCheckBehaviorIndexPrompt,
    startupCardUpdateCheckBehaviorIndexAlways
};

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
    startupUpdateCheckCheckBox.setChecked(settings.getCheckUpdatesOnStartup());

    startupCardUpdateCheckBehaviorSelector.addItem(""); // these will be set in retranslateUI
    startupCardUpdateCheckBehaviorSelector.addItem("");
    startupCardUpdateCheckBehaviorSelector.addItem("");
    if (SettingsCache::instance().getStartupCardUpdateCheckPromptForUpdate()) {
        startupCardUpdateCheckBehaviorSelector.setCurrentIndex(startupCardUpdateCheckBehaviorIndexPrompt);
    } else if (SettingsCache::instance().getStartupCardUpdateCheckAlwaysUpdate()) {
        startupCardUpdateCheckBehaviorSelector.setCurrentIndex(startupCardUpdateCheckBehaviorIndexAlways);
    } else {
        startupCardUpdateCheckBehaviorSelector.setCurrentIndex(startupCardUpdateCheckBehaviorIndexNone);
    }

    cardUpdateCheckIntervalSpinBox.setMinimum(1);
    cardUpdateCheckIntervalSpinBox.setMaximum(30);
    cardUpdateCheckIntervalSpinBox.setValue(settings.getCardUpdateCheckInterval());
    updateNotificationCheckBox.setChecked(settings.getNotifyAboutUpdates());
    newVersionOracleCheckBox.setChecked(settings.getNotifyAboutNewVersion());

    showTipsOnStartup.setChecked(settings.getShowTipsOnStartup());

    advertiseTranslationPageLabel.setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    advertiseTranslationPageLabel.setOpenExternalLinks(true);

    connect(&languageBox, qOverload<int>(&QComboBox::currentIndexChanged), this,
            &GeneralSettingsPage::languageBoxChanged);
    connect(&startupUpdateCheckCheckBox, &QCheckBox::QT_STATE_CHANGED, &settings,
            &SettingsCache::setCheckUpdatesOnStartup);
    connect(&startupCardUpdateCheckBehaviorSelector, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [](int index) {
                SettingsCache::instance().setStartupCardUpdateCheckPromptForUpdate(
                    index == startupCardUpdateCheckBehaviorIndexPrompt);
                SettingsCache::instance().setStartupCardUpdateCheckAlwaysUpdate(
                    index == startupCardUpdateCheckBehaviorIndexAlways);
            });
    connect(&cardUpdateCheckIntervalSpinBox, qOverload<int>(&QSpinBox::valueChanged), &settings,
            &SettingsCache::setCardUpdateCheckInterval);
    connect(&updateNotificationCheckBox, &QCheckBox::QT_STATE_CHANGED, &settings, &SettingsCache::setNotifyAboutUpdate);
    connect(&newVersionOracleCheckBox, &QCheckBox::QT_STATE_CHANGED, &settings,
            &SettingsCache::setNotifyAboutNewVersion);
    connect(&showTipsOnStartup, &QCheckBox::clicked, &settings, &SettingsCache::setShowTipsOnStartup);

    auto *personalGrid = new QGridLayout;
    personalGrid->addWidget(&languageLabel, 0, 0);
    personalGrid->addWidget(&languageBox, 0, 1);
    personalGrid->addWidget(&advertiseTranslationPageLabel, 1, 1, Qt::AlignRight);
    personalGrid->addWidget(&updateReleaseChannelLabel, 2, 0);
    personalGrid->addWidget(&updateReleaseChannelBox, 2, 1);
    personalGrid->addWidget(&startupUpdateCheckCheckBox, 4, 0, 1, 2);
    personalGrid->addWidget(&startupCardUpdateCheckBehaviorLabel, 5, 0);
    personalGrid->addWidget(&startupCardUpdateCheckBehaviorSelector, 5, 1);
    personalGrid->addWidget(&cardUpdateCheckIntervalLabel, 6, 0);
    personalGrid->addWidget(&cardUpdateCheckIntervalSpinBox, 6, 1);
    personalGrid->addWidget(&lastCardUpdateCheckDateLabel, 7, 1);
    personalGrid->addWidget(&updateNotificationCheckBox, 8, 0, 1, 2);
    personalGrid->addWidget(&newVersionOracleCheckBox, 9, 0, 1, 2);
    personalGrid->addWidget(&showTipsOnStartup, 10, 0, 1, 2);

    personalGroupBox = new QGroupBox;
    personalGroupBox->setLayout(personalGrid);

    deckPathEdit = new QLineEdit(settings.getDeckPath());
    deckPathEdit->setReadOnly(true);
    QPushButton *deckPathButton = new QPushButton("...");
    connect(deckPathButton, &QPushButton::clicked, this, &GeneralSettingsPage::deckPathButtonClicked);

    filtersPathEdit = new QLineEdit(settings.getFiltersPath());
    filtersPathEdit->setReadOnly(true);
    QPushButton *filtersPathButton = new QPushButton("...");
    connect(filtersPathButton, &QPushButton::clicked, this, &GeneralSettingsPage::filtersPathButtonClicked);

    replaysPathEdit = new QLineEdit(settings.getReplaysPath());
    replaysPathEdit->setReadOnly(true);
    QPushButton *replaysPathButton = new QPushButton("...");
    connect(replaysPathButton, &QPushButton::clicked, this, &GeneralSettingsPage::replaysPathButtonClicked);

    picsPathEdit = new QLineEdit(settings.getPicsPath());
    picsPathEdit->setReadOnly(true);
    QPushButton *picsPathButton = new QPushButton("...");
    connect(picsPathButton, &QPushButton::clicked, this, &GeneralSettingsPage::picsPathButtonClicked);

    cardDatabasePathEdit = new QLineEdit(settings.getCardDatabasePath());
    cardDatabasePathEdit->setReadOnly(true);
    QPushButton *cardDatabasePathButton = new QPushButton("...");
    connect(cardDatabasePathButton, &QPushButton::clicked, this, &GeneralSettingsPage::cardDatabasePathButtonClicked);

    customCardDatabasePathEdit = new QLineEdit(settings.getCustomCardDatabasePath());
    customCardDatabasePathEdit->setReadOnly(true);
    QPushButton *customCardDatabasePathButton = new QPushButton("...");
    connect(customCardDatabasePathButton, &QPushButton::clicked, this,
            &GeneralSettingsPage::customCardDatabaseButtonClicked);

    tokenDatabasePathEdit = new QLineEdit(settings.getTokenDatabasePath());
    tokenDatabasePathEdit->setReadOnly(true);
    QPushButton *tokenDatabasePathButton = new QPushButton("...");
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
    mainLayout->addWidget(personalGroupBox);
    mainLayout->addWidget(pathsGroupBox);
    mainLayout->addStretch();

    GeneralSettingsPage::retranslateUi();

    // connect the ReleaseChannel combo box only after the entries are inserted in retranslateUi
    connect(&updateReleaseChannelBox, qOverload<int>(&QComboBox::currentIndexChanged), &settings,
            &SettingsCache::setUpdateReleaseChannelIndex);
    updateReleaseChannelBox.setCurrentIndex(settings.getUpdateReleaseChannelIndex());

    setLayout(mainLayout);

    connect(&SettingsCache::instance(), &SettingsCache::langChanged, this, &GeneralSettingsPage::retranslateUi);
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
        qCWarning(DlgSettingsLog) << "Unable to load" << translationPrefix << "translation" << appNameHint << "at"
                                  << translationPath;
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

void GeneralSettingsPage::filtersPathButtonClicked()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Choose path"), filtersPathEdit->text());
    if (path.isEmpty())
        return;

    filtersPathEdit->setText(path);
    SettingsCache::instance().setFiltersPath(path);
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
    resetAllPathsButton->setText(tr("Reset all paths"));

    const auto &settings = SettingsCache::instance();

    QDate lastCheckDate = settings.getLastCardUpdateCheck();
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

AppearanceSettingsPage::AppearanceSettingsPage()
{
    SettingsCache &settings = SettingsCache::instance();

    // Theme settings
    QString themeName = SettingsCache::instance().getThemeName();

    QStringList themeDirs = themeManager->getAvailableThemes().keys();
    for (int i = 0; i < themeDirs.size(); i++) {
        themeBox.addItem(themeDirs[i]);
        if (themeDirs[i] == themeName)
            themeBox.setCurrentIndex(i);
    }

    connect(&themeBox, qOverload<int>(&QComboBox::currentIndexChanged), this, &AppearanceSettingsPage::themeBoxChanged);
    connect(&openThemeButton, &QPushButton::clicked, this, &AppearanceSettingsPage::openThemeLocation);

    for (const auto &entry : BackgroundSources::all()) {
        homeTabBackgroundSourceBox.addItem(QObject::tr(entry.trKey), QVariant::fromValue(entry.type));
    }

    QString homeTabBackgroundSource = SettingsCache::instance().getHomeTabBackgroundSource();
    int homeTabBackgroundSourceId =
        homeTabBackgroundSourceBox.findData(BackgroundSources::fromId(homeTabBackgroundSource));
    if (homeTabBackgroundSourceId != -1) {
        homeTabBackgroundSourceBox.setCurrentIndex(homeTabBackgroundSourceId);
    }

    connect(&homeTabBackgroundSourceBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
        auto type = homeTabBackgroundSourceBox.currentData().value<BackgroundSources::Type>();
        SettingsCache::instance().setHomeTabBackgroundSource(BackgroundSources::toId(type));
    });

    homeTabBackgroundShuffleFrequencySpinBox.setRange(0, 3600);
    homeTabBackgroundShuffleFrequencySpinBox.setSuffix(tr(" seconds"));
    homeTabBackgroundShuffleFrequencySpinBox.setValue(SettingsCache::instance().getHomeTabBackgroundShuffleFrequency());
    connect(&homeTabBackgroundShuffleFrequencySpinBox, qOverload<int>(&QSpinBox::valueChanged),
            &SettingsCache::instance(), &SettingsCache::setHomeTabBackgroundShuffleFrequency);

    auto *themeGrid = new QGridLayout;
    themeGrid->addWidget(&themeLabel, 0, 0);
    themeGrid->addWidget(&themeBox, 0, 1);
    themeGrid->addWidget(&openThemeButton, 1, 1);
    themeGrid->addWidget(&homeTabBackgroundSourceLabel, 2, 0);
    themeGrid->addWidget(&homeTabBackgroundSourceBox, 2, 1);
    themeGrid->addWidget(&homeTabBackgroundShuffleFrequencyLabel, 3, 0);
    themeGrid->addWidget(&homeTabBackgroundShuffleFrequencySpinBox, 3, 1);

    themeGroupBox = new QGroupBox;
    themeGroupBox->setLayout(themeGrid);

    // Menu settings
    showShortcutsCheckBox.setChecked(settings.getShowShortcuts());
    connect(&showShortcutsCheckBox, &QCheckBox::QT_STATE_CHANGED, this, &AppearanceSettingsPage::showShortcutsChanged);

    auto *menuGrid = new QGridLayout;
    menuGrid->addWidget(&showShortcutsCheckBox, 0, 0);

    menuGroupBox = new QGroupBox;
    menuGroupBox->setLayout(menuGrid);

    // Card rendering
    displayCardNamesCheckBox.setChecked(settings.getDisplayCardNames());
    connect(&displayCardNamesCheckBox, &QCheckBox::QT_STATE_CHANGED, &settings, &SettingsCache::setDisplayCardNames);

    autoRotateSidewaysLayoutCardsCheckBox.setChecked(settings.getAutoRotateSidewaysLayoutCards());
    connect(&autoRotateSidewaysLayoutCardsCheckBox, &QCheckBox::QT_STATE_CHANGED, &settings,
            &SettingsCache::setAutoRotateSidewaysLayoutCards);

    overrideAllCardArtWithPersonalPreferenceCheckBox.setChecked(settings.getOverrideAllCardArtWithPersonalPreference());
    connect(&overrideAllCardArtWithPersonalPreferenceCheckBox, &QCheckBox::QT_STATE_CHANGED, this,
            &AppearanceSettingsPage::overrideAllCardArtWithPersonalPreferenceToggled);

    bumpSetsWithCardsInDeckToTopCheckBox.setChecked(settings.getBumpSetsWithCardsInDeckToTop());
    connect(&bumpSetsWithCardsInDeckToTopCheckBox, &QCheckBox::QT_STATE_CHANGED, &settings,
            &SettingsCache::setBumpSetsWithCardsInDeckToTop);

    cardScalingCheckBox.setChecked(settings.getScaleCards());
    connect(&cardScalingCheckBox, &QCheckBox::QT_STATE_CHANGED, &settings, &SettingsCache::setCardScaling);

    roundCardCornersCheckBox.setChecked(settings.getRoundCardCorners());
    connect(&roundCardCornersCheckBox, &QAbstractButton::toggled, &settings, &SettingsCache::setRoundCardCorners);

    verticalCardOverlapPercentBox.setValue(settings.getStackCardOverlapPercent());
    verticalCardOverlapPercentBox.setRange(0, 80);
    connect(&verticalCardOverlapPercentBox, qOverload<int>(&QSpinBox::valueChanged), &settings,
            &SettingsCache::setStackCardOverlapPercent);

    cardViewInitialRowsMaxBox.setRange(1, 999);
    cardViewInitialRowsMaxBox.setValue(SettingsCache::instance().getCardViewInitialRowsMax());
    connect(&cardViewInitialRowsMaxBox, qOverload<int>(&QSpinBox::valueChanged), this,
            &AppearanceSettingsPage::cardViewInitialRowsMaxChanged);

    cardViewExpandedRowsMaxBox.setRange(1, 999);
    cardViewExpandedRowsMaxBox.setValue(SettingsCache::instance().getCardViewExpandedRowsMax());
    connect(&cardViewExpandedRowsMaxBox, qOverload<int>(&QSpinBox::valueChanged), this,
            &AppearanceSettingsPage::cardViewExpandedRowsMaxChanged);

    auto *cardsGrid = new QGridLayout;
    cardsGrid->addWidget(&displayCardNamesCheckBox, 0, 0, 1, 2);
    cardsGrid->addWidget(&autoRotateSidewaysLayoutCardsCheckBox, 1, 0, 1, 2);
    cardsGrid->addWidget(&cardScalingCheckBox, 2, 0, 1, 2);
    cardsGrid->addWidget(&roundCardCornersCheckBox, 3, 0, 1, 2);
    cardsGrid->addWidget(&overrideAllCardArtWithPersonalPreferenceCheckBox, 4, 0, 1, 2);
    cardsGrid->addWidget(&bumpSetsWithCardsInDeckToTopCheckBox, 5, 0, 1, 2);
    cardsGrid->addWidget(&verticalCardOverlapPercentLabel, 6, 0, 1, 1);
    cardsGrid->addWidget(&verticalCardOverlapPercentBox, 6, 1, 1, 1);
    cardsGrid->addWidget(&cardViewInitialRowsMaxLabel, 7, 0);
    cardsGrid->addWidget(&cardViewInitialRowsMaxBox, 7, 1);
    cardsGrid->addWidget(&cardViewExpandedRowsMaxLabel, 8, 0);
    cardsGrid->addWidget(&cardViewExpandedRowsMaxBox, 8, 1);

    cardsGroupBox = new QGroupBox;
    cardsGroupBox->setLayout(cardsGrid);

    // Card counter colors

    auto *cardCounterColorsLayout = new QGridLayout;
    cardCounterColorsLayout->setColumnStretch(1, 1);
    cardCounterColorsLayout->setColumnStretch(3, 1);
    cardCounterColorsLayout->setColumnStretch(5, 1);

    auto &cardCounterSettings = SettingsCache::instance().cardCounters();
    for (int index = 0; index < 6; ++index) {
        auto *pushButton = new QPushButton;
        pushButton->setStyleSheet(QString("background-color: %1").arg(cardCounterSettings.color(index).name()));

        connect(&SettingsCache::instance().cardCounters(), &CardCounterSettings::colorChanged, pushButton,
                [index, pushButton](int changedIndex, const QColor &color) {
                    if (index == changedIndex) {
                        pushButton->setStyleSheet(QString("background-color: %1").arg(color.name()));
                    }
                });

        connect(pushButton, &QPushButton::clicked, this, [index, this]() {
            auto &cardCounterSettings = SettingsCache::instance().cardCounters();

            auto newColor = QColorDialog::getColor(cardCounterSettings.color(index), this);
            if (!newColor.isValid())
                return;

            cardCounterSettings.setColor(index, newColor);
        });

        auto *colorName = new QLabel;
        cardCounterNames.append(colorName);

        int row = index / 3;
        int column = 2 * (index % 3);

        cardCounterColorsLayout->addWidget(pushButton, row, column);
        cardCounterColorsLayout->addWidget(colorName, row, column + 1);
    }

    auto *cardCountersLayout = new QVBoxLayout;
    cardCountersLayout->addLayout(cardCounterColorsLayout, 1);

    cardCountersGroupBox = new QGroupBox;
    cardCountersGroupBox->setLayout(cardCountersLayout);

    // Hand layout
    horizontalHandCheckBox.setChecked(settings.getHorizontalHand());
    connect(&horizontalHandCheckBox, &QCheckBox::QT_STATE_CHANGED, &settings, &SettingsCache::setHorizontalHand);

    leftJustifiedHandCheckBox.setChecked(settings.getLeftJustified());
    connect(&leftJustifiedHandCheckBox, &QCheckBox::QT_STATE_CHANGED, &settings, &SettingsCache::setLeftJustified);

    auto *handGrid = new QGridLayout;
    handGrid->addWidget(&horizontalHandCheckBox, 0, 0, 1, 2);
    handGrid->addWidget(&leftJustifiedHandCheckBox, 1, 0, 1, 2);

    handGroupBox = new QGroupBox;
    handGroupBox->setLayout(handGrid);

    // table grid layout
    invertVerticalCoordinateCheckBox.setChecked(settings.getInvertVerticalCoordinate());
    connect(&invertVerticalCoordinateCheckBox, &QCheckBox::QT_STATE_CHANGED, &settings,
            &SettingsCache::setInvertVerticalCoordinate);

    minPlayersForMultiColumnLayoutEdit.setMinimum(2);
    minPlayersForMultiColumnLayoutEdit.setValue(settings.getMinPlayersForMultiColumnLayout());
    connect(&minPlayersForMultiColumnLayoutEdit, qOverload<int>(&QSpinBox::valueChanged), &settings,
            &SettingsCache::setMinPlayersForMultiColumnLayout);
    minPlayersForMultiColumnLayoutLabel.setBuddy(&minPlayersForMultiColumnLayoutEdit);

    connect(&maxFontSizeForCardsEdit, qOverload<int>(&QSpinBox::valueChanged), &settings,
            &SettingsCache::setMaxFontSize);
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

    // putting it all together
    auto *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(themeGroupBox);
    mainLayout->addWidget(menuGroupBox);
    mainLayout->addWidget(cardsGroupBox);
    mainLayout->addWidget(cardCountersGroupBox);
    mainLayout->addWidget(handGroupBox);
    mainLayout->addWidget(tableGroupBox);
    mainLayout->addStretch();

    setLayout(mainLayout);

    connect(&SettingsCache::instance(), &SettingsCache::langChanged, this, &AppearanceSettingsPage::retranslateUi);
    retranslateUi();
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

void AppearanceSettingsPage::showShortcutsChanged(QT_STATE_CHANGED_T value)
{
    SettingsCache::instance().setShowShortcuts(value);
    qApp->setAttribute(Qt::AA_DontShowShortcutsInContextMenus, value == 0); // 0 = unchecked
}

void AppearanceSettingsPage::overrideAllCardArtWithPersonalPreferenceToggled(QT_STATE_CHANGED_T value)
{
    bool enable = static_cast<bool>(value);

    QString message;
    if (enable) {
        message = tr("Enabling this feature will disable the use of the Printing Selector.\n\n"
                     "You will not be able to manage printing preferences on a per-deck basis, "
                     "or see printings other people have selected for their decks.\n\n"
                     "You will have to use the Set Manager, available through Card Database -> Manage Sets.\n\n"
                     "Are you sure you would like to enable this feature?");
    } else {
        message =
            tr("Disabling this feature will enable the Printing Selector.\n\n"
               "You can now choose printings on a per-deck basis in the Deck Editor and configure which printing "
               "gets added to a deck by default by pinning it in the Printing Selector.\n\n"
               "You can also use the Set Manager to adjust custom sort order for printings in the Printing Selector"
               " (other sort orders like alphabetical or release date are available).\n\n"
               "Are you sure you would like to disable this feature?");
    }

    QMessageBox::StandardButton result =
        QMessageBox::question(this, tr("Confirm Change"), message, QMessageBox::Yes | QMessageBox::No);

    if (result == QMessageBox::Yes) {
        SettingsCache::instance().setOverrideAllCardArtWithPersonalPreference(value);
        // Caches are now invalid.
        CardPictureLoader::clearPixmapCache();
        CardPictureLoader::clearNetworkCache();
    } else {
        // If user cancels, revert the checkbox/state back
        QTimer::singleShot(0, this, [this, enable]() {
            overrideAllCardArtWithPersonalPreferenceCheckBox.blockSignals(true);
            overrideAllCardArtWithPersonalPreferenceCheckBox.setChecked(!enable);
            overrideAllCardArtWithPersonalPreferenceCheckBox.blockSignals(false);
        });
    }
}

/**
 * Updates the settings for cardViewInitialRowsMax.
 * Forces expanded rows max to always be >= initial rows max
 * @param value The new value
 */
void AppearanceSettingsPage::cardViewInitialRowsMaxChanged(int value)
{
    SettingsCache::instance().setCardViewInitialRowsMax(value);
    if (cardViewExpandedRowsMaxBox.value() < value) {
        cardViewExpandedRowsMaxBox.setValue(value);
    }
}

/**
 * Updates the settings for cardViewExpandedRowsMax.
 * Forces initial rows max to always be <= expanded rows max
 * @param value The new value
 */
void AppearanceSettingsPage::cardViewExpandedRowsMaxChanged(int value)
{
    SettingsCache::instance().setCardViewExpandedRowsMax(value);
    if (cardViewInitialRowsMaxBox.value() > value) {
        cardViewInitialRowsMaxBox.setValue(value);
    }
}

void AppearanceSettingsPage::retranslateUi()
{
    themeGroupBox->setTitle(tr("Theme settings"));
    themeLabel.setText(tr("Current theme:"));
    openThemeButton.setText(tr("Open themes folder"));
    homeTabBackgroundSourceLabel.setText(tr("Home tab background source:"));
    homeTabBackgroundShuffleFrequencyLabel.setText(tr("Home tab background shuffle frequency:"));
    homeTabBackgroundShuffleFrequencySpinBox.setSpecialValueText(tr("Disabled"));

    menuGroupBox->setTitle(tr("Menu settings"));
    showShortcutsCheckBox.setText(tr("Show keyboard shortcuts in right-click menus"));

    cardsGroupBox->setTitle(tr("Card rendering"));
    displayCardNamesCheckBox.setText(tr("Display card names on cards having a picture"));
    autoRotateSidewaysLayoutCardsCheckBox.setText(tr("Auto-Rotate cards with sideways layout"));
    overrideAllCardArtWithPersonalPreferenceCheckBox.setText(
        tr("Override all card art with personal set preference (Pre-ProviderID change behavior)"));
    bumpSetsWithCardsInDeckToTopCheckBox.setText(
        tr("Bump sets that the deck contains cards from to the top in the printing selector"));
    cardScalingCheckBox.setText(tr("Scale cards on mouse over"));
    roundCardCornersCheckBox.setText(tr("Use rounded card corners"));
    verticalCardOverlapPercentLabel.setText(
        tr("Minimum overlap percentage of cards on the stack and in vertical hand"));
    cardViewInitialRowsMaxLabel.setText(tr("Maximum initial height for card view window:"));
    cardViewInitialRowsMaxBox.setSuffix(tr(" rows"));
    cardViewExpandedRowsMaxLabel.setText(tr("Maximum expanded height for card view window:"));
    cardViewExpandedRowsMaxBox.setSuffix(tr(" rows"));

    cardCountersGroupBox->setTitle(tr("Card counters"));

    auto &cardCounterSettings = SettingsCache::instance().cardCounters();
    for (int index = 0; index < cardCounterNames.size(); ++index) {
        cardCounterNames[index]->setText(tr("Counter %1").arg(cardCounterSettings.displayName(index)));
    }

    handGroupBox->setTitle(tr("Hand layout"));
    horizontalHandCheckBox.setText(tr("Display hand horizontally (wastes space)"));
    leftJustifiedHandCheckBox.setText(tr("Enable left justification"));

    tableGroupBox->setTitle(tr("Table grid layout"));
    invertVerticalCoordinateCheckBox.setText(tr("Invert vertical coordinate"));
    minPlayersForMultiColumnLayoutLabel.setText(tr("Minimum player count for multi-column layout:"));
    maxFontSizeForCardsLabel.setText(tr("Maximum font size for information displayed on cards:"));
}

enum visualDeckStoragePromptForConversionIndex
{
    visualDeckStoragePromptForConversionIndexNone,
    visualDeckStoragePromptForConversionIndexPrompt,
    visualDeckStoragePromptForConversionIndexAlways
};

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

    clickPlaysAllSelectedCheckBox.setChecked(SettingsCache::instance().getClickPlaysAllSelected());
    connect(&clickPlaysAllSelectedCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance(),
            &SettingsCache::setClickPlaysAllSelected);

    playToStackCheckBox.setChecked(SettingsCache::instance().getPlayToStack());
    connect(&playToStackCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance(),
            &SettingsCache::setPlayToStack);

    closeEmptyCardViewCheckBox.setChecked(SettingsCache::instance().getCloseEmptyCardView());
    connect(&closeEmptyCardViewCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance(),
            &SettingsCache::setCloseEmptyCardView);

    focusCardViewSearchBarCheckBox.setChecked(SettingsCache::instance().getFocusCardViewSearchBar());
    connect(&focusCardViewSearchBarCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance(),
            &SettingsCache::setFocusCardViewSearchBar);

    annotateTokensCheckBox.setChecked(SettingsCache::instance().getAnnotateTokens());
    connect(&annotateTokensCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance(),
            &SettingsCache::setAnnotateTokens);

    useTearOffMenusCheckBox.setChecked(SettingsCache::instance().getUseTearOffMenus());
    connect(&useTearOffMenusCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance(),
            [](const QT_STATE_CHANGED_T state) { SettingsCache::instance().setUseTearOffMenus(state == Qt::Checked); });

    auto *generalGrid = new QGridLayout;
    generalGrid->addWidget(&doubleClickToPlayCheckBox, 0, 0);
    generalGrid->addWidget(&clickPlaysAllSelectedCheckBox, 1, 0);
    generalGrid->addWidget(&playToStackCheckBox, 2, 0);
    generalGrid->addWidget(&closeEmptyCardViewCheckBox, 3, 0);
    generalGrid->addWidget(&focusCardViewSearchBarCheckBox, 4, 0);
    generalGrid->addWidget(&annotateTokensCheckBox, 5, 0);
    generalGrid->addWidget(&useTearOffMenusCheckBox, 6, 0);

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

    visualDeckStorageInGameCheckBox.setChecked(SettingsCache::instance().getVisualDeckStorageInGame());
    connect(&visualDeckStorageInGameCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance(),
            &SettingsCache::setVisualDeckStorageInGame);

    visualDeckStorageSelectionAnimationCheckBox.setChecked(
        SettingsCache::instance().getVisualDeckStorageSelectionAnimation());
    connect(&visualDeckStorageSelectionAnimationCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance(),
            &SettingsCache::setVisualDeckStorageSelectionAnimation);

    visualDeckStoragePromptForConversionSelector.addItem(""); // these will be set in retranslateUI
    visualDeckStoragePromptForConversionSelector.addItem("");
    visualDeckStoragePromptForConversionSelector.addItem("");
    if (SettingsCache::instance().getVisualDeckStoragePromptForConversion()) {
        visualDeckStoragePromptForConversionSelector.setCurrentIndex(visualDeckStoragePromptForConversionIndexPrompt);
    } else if (SettingsCache::instance().getVisualDeckStorageAlwaysConvert()) {
        visualDeckStoragePromptForConversionSelector.setCurrentIndex(visualDeckStoragePromptForConversionIndexAlways);
    } else {
        visualDeckStoragePromptForConversionSelector.setCurrentIndex(visualDeckStoragePromptForConversionIndexNone);
    }
    connect(&visualDeckStoragePromptForConversionSelector, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [](int index) {
                SettingsCache::instance().setVisualDeckStoragePromptForConversion(
                    index == visualDeckStoragePromptForConversionIndexPrompt);
                SettingsCache::instance().setVisualDeckStorageAlwaysConvert(
                    index == visualDeckStoragePromptForConversionIndexAlways);
            });

    defaultDeckEditorTypeSelector.addItem(""); // these will be set in retranslateUI
    defaultDeckEditorTypeSelector.addItem("");
    defaultDeckEditorTypeSelector.setCurrentIndex(SettingsCache::instance().getDefaultDeckEditorType());
    connect(&defaultDeckEditorTypeSelector, QOverload<int>::of(&QComboBox::currentIndexChanged),
            &SettingsCache::instance(), &SettingsCache::setDefaultDeckEditorType);

    auto *deckEditorGrid = new QGridLayout;
    deckEditorGrid->addWidget(&openDeckInNewTabCheckBox, 0, 0);
    deckEditorGrid->addWidget(&visualDeckStorageInGameCheckBox, 1, 0);
    deckEditorGrid->addWidget(&visualDeckStorageSelectionAnimationCheckBox, 2, 0);
    deckEditorGrid->addWidget(&visualDeckStoragePromptForConversionLabel, 3, 0);
    deckEditorGrid->addWidget(&visualDeckStoragePromptForConversionSelector, 3, 1);
    deckEditorGrid->addWidget(&defaultDeckEditorTypeLabel, 4, 0);
    deckEditorGrid->addWidget(&defaultDeckEditorTypeSelector, 4, 1);

    deckEditorGroupBox = new QGroupBox;
    deckEditorGroupBox->setLayout(deckEditorGrid);

    // replay settings
    rewindBufferingMsBox.setRange(0, 9999);
    rewindBufferingMsBox.setValue(SettingsCache::instance().getRewindBufferingMs());
    connect(&rewindBufferingMsBox, qOverload<int>(&QSpinBox::valueChanged), &SettingsCache::instance(),
            &SettingsCache::setRewindBufferingMs);

    auto *replayGrid = new QGridLayout;
    replayGrid->addWidget(&rewindBufferingMsLabel, 0, 0, 1, 1);
    replayGrid->addWidget(&rewindBufferingMsBox, 0, 1, 1, 1);

    replayGroupBox = new QGroupBox;
    replayGroupBox->setLayout(replayGrid);

    // putting it all together
    auto *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(generalGroupBox);
    mainLayout->addWidget(notificationsGroupBox);
    mainLayout->addWidget(animationGroupBox);
    mainLayout->addWidget(deckEditorGroupBox);
    mainLayout->addWidget(replayGroupBox);
    mainLayout->addStretch();

    setLayout(mainLayout);

    connect(&SettingsCache::instance(), &SettingsCache::langChanged, this, &UserInterfaceSettingsPage::retranslateUi);
    retranslateUi();
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
    clickPlaysAllSelectedCheckBox.setText(tr("&Clicking plays all selected cards (instead of just the clicked card)"));
    playToStackCheckBox.setText(tr("&Play all nonlands onto the stack (not the battlefield) by default"));
    closeEmptyCardViewCheckBox.setText(tr("Close card view window when last card is removed"));
    focusCardViewSearchBarCheckBox.setText(tr("Auto focus search bar when card view window is opened"));
    annotateTokensCheckBox.setText(tr("Annotate card text on tokens"));
    useTearOffMenusCheckBox.setText(tr("Use tear-off menus, allowing right click menus to persist on screen"));
    notificationsGroupBox->setTitle(tr("Notifications settings"));
    notificationsEnabledCheckBox.setText(tr("Enable notifications in taskbar"));
    specNotificationsEnabledCheckBox.setText(tr("Notify in the taskbar for game events while you are spectating"));
    buddyConnectNotificationsEnabledCheckBox.setText(tr("Notify in the taskbar when users in your buddy list connect"));
    animationGroupBox->setTitle(tr("Animation settings"));
    tapAnimationCheckBox.setText(tr("&Tap/untap animation"));
    deckEditorGroupBox->setTitle(tr("Deck editor/storage settings"));
    openDeckInNewTabCheckBox.setText(tr("Open deck in new tab by default"));
    visualDeckStorageInGameCheckBox.setText(tr("Use visual deck storage in game lobby"));
    visualDeckStorageSelectionAnimationCheckBox.setText(tr("Use selection animation for Visual Deck Storage"));
    visualDeckStoragePromptForConversionLabel.setText(
        tr("When adding a tag in the visual deck storage to a .txt deck:"));
    visualDeckStoragePromptForConversionSelector.setItemText(visualDeckStoragePromptForConversionIndexNone,
                                                             tr("do nothing"));
    visualDeckStoragePromptForConversionSelector.setItemText(visualDeckStoragePromptForConversionIndexPrompt,
                                                             tr("ask to convert to .cod"));
    visualDeckStoragePromptForConversionSelector.setItemText(visualDeckStoragePromptForConversionIndexAlways,
                                                             tr("always convert to .cod"));
    defaultDeckEditorTypeLabel.setText(tr("Default deck editor type"));
    defaultDeckEditorTypeSelector.setItemText(TabSupervisor::ClassicDeckEditor, tr("Classic Deck Editor"));
    defaultDeckEditorTypeSelector.setItemText(TabSupervisor::VisualDeckEditor, tr("Visual Deck Editor"));
    replayGroupBox->setTitle(tr("Replay settings"));
    rewindBufferingMsLabel.setText(tr("Buffer time for backwards skip via shortcut:"));
    rewindBufferingMsBox.setSuffix(" ms");
}

DeckEditorSettingsPage::DeckEditorSettingsPage()
{
    picDownloadCheckBox.setChecked(SettingsCache::instance().getPicDownload());
    connect(&picDownloadCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance(),
            &SettingsCache::setPicDownload);

    urlLinkLabel.setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    urlLinkLabel.setOpenExternalLinks(true);

    connect(&clearDownloadedPicsButton, &QPushButton::clicked, this,
            &DeckEditorSettingsPage::clearDownloadedPicsButtonClicked);
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
    lpGeneralGrid->addLayout(networkCacheLayout, 2, 1);
    lpGeneralGrid->addLayout(networkRedirectCacheLayout, 3, 0);
    lpGeneralGrid->addLayout(pixmapCacheLayout, 3, 1);
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
    connect(&mcDownloadSpoilersCheckBox, &QCheckBox::toggled, &SettingsCache::instance(),
            &SettingsCache::setDownloadSpoilerStatus);
    connect(&mcDownloadSpoilersCheckBox, &QCheckBox::toggled, this, &DeckEditorSettingsPage::setSpoilersEnabled);
    connect(&pixmapCacheEdit, qOverload<int>(&QSpinBox::valueChanged), &SettingsCache::instance(),
            &SettingsCache::setPixmapCacheSize);
    connect(&networkCacheEdit, qOverload<int>(&QSpinBox::valueChanged), &SettingsCache::instance(),
            &SettingsCache::setNetworkCacheSizeInMB);
    connect(&networkRedirectCacheTtlEdit, qOverload<int>(&QSpinBox::valueChanged), &SettingsCache::instance(),
            &SettingsCache::setNetworkRedirectCacheTtl);

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

void DeckEditorSettingsPage::clearDownloadedPicsButtonClicked()
{
    CardPictureLoader::clearNetworkCache();

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
    clearDownloadedPicsButton.setText(tr("Delete Downloaded Images"));
    resetDownloadURLs.setText(tr("Reset Download URLs"));
    networkCacheLabel.setText(tr("Network Cache Size:"));
    networkCacheEdit.setToolTip(tr("On-disk cache for downloaded pictures"));
    networkRedirectCacheTtlLabel.setText(tr("Redirect Cache TTL:"));
    networkRedirectCacheTtlEdit.setToolTip(tr("How long cached redirects for urls are valid for."));
    pixmapCacheLabel.setText(tr("Picture Cache Size:"));
    pixmapCacheEdit.setToolTip(tr("In-memory cache for pictures not currently on screen"));
    updateNowButton->setText(tr("Update Spoilers"));
    aAdd->setText(tr("Add New URL"));
    aEdit->setText(tr("Edit URL"));
    aRemove->setText(tr("Remove URL"));
    networkRedirectCacheTtlEdit.setSuffix(" " + tr("Day(s)"));
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
    connect(mentionColor, &QLineEdit::textChanged, this, &MessagesSettingsPage::updateColor);

    messagePopups.setChecked(SettingsCache::instance().getShowMessagePopup());
    connect(&messagePopups, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance(),
            &SettingsCache::setShowMessagePopups);

    mentionPopups.setChecked(SettingsCache::instance().getShowMentionPopup());
    connect(&mentionPopups, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance(),
            &SettingsCache::setShowMentionPopups);

    roomHistory.setChecked(SettingsCache::instance().getRoomHistory());
    connect(&roomHistory, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance(), &SettingsCache::setRoomHistory);

    customAlertString = new QLineEdit();
    customAlertString->setText(SettingsCache::instance().getHighlightWords());
    connect(customAlertString, &QLineEdit::textChanged, &SettingsCache::instance(), &SettingsCache::setHighlightWords);

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
    connect(highlightColor, &QLineEdit::textChanged, this, &MessagesSettingsPage::updateHighlightColor);

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
    connect(aAdd, &QAction::triggered, this, &MessagesSettingsPage::actAdd);

    aEdit = new QAction(this);
    aEdit->setIcon(QPixmap("theme:icons/pencil"));
    connect(aEdit, &QAction::triggered, this, &MessagesSettingsPage::actEdit);

    aRemove = new QAction(this);
    aRemove->setIcon(QPixmap("theme:icons/decrement"));
    connect(aRemove, &QAction::triggered, this, &MessagesSettingsPage::actRemove);

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

    connect(&SettingsCache::instance(), &SettingsCache::langChanged, this, &MessagesSettingsPage::retranslateUi);
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
    customAlertString->setPlaceholderText(tr("Word1 Word2 Word3"));
    aAdd->setText(tr("Add New Message"));
    aEdit->setText(tr("Edit Message"));
    aRemove->setText(tr("Remove Message"));
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

    connect(&themeBox, qOverload<int>(&QComboBox::currentIndexChanged), this, &SoundSettingsPage::themeBoxChanged);
    connect(&soundTestButton, &QPushButton::clicked, soundEngine, &SoundEngine::testSound);

    masterVolumeSlider = new QSlider(Qt::Horizontal);
    masterVolumeSlider->setMinimum(0);
    masterVolumeSlider->setMaximum(100);
    masterVolumeSlider->setValue(SettingsCache::instance().getMasterVolume());
    masterVolumeSlider->setToolTip(QString::number(SettingsCache::instance().getMasterVolume()));
    connect(&SettingsCache::instance(), &SettingsCache::masterVolumeChanged, this,
            &SoundSettingsPage::masterVolumeChanged);
    connect(masterVolumeSlider, &QSlider::sliderReleased, soundEngine, &SoundEngine::testSound);
    connect(masterVolumeSlider, &QSlider::valueChanged, &SettingsCache::instance(), &SettingsCache::setMasterVolume);

    masterVolumeSpinBox = new QSpinBox();
    masterVolumeSpinBox->setMinimum(0);
    masterVolumeSpinBox->setMaximum(100);
    masterVolumeSpinBox->setValue(SettingsCache::instance().getMasterVolume());
    connect(masterVolumeSlider, &QSlider::valueChanged, masterVolumeSpinBox, &QSpinBox::setValue);
    connect(masterVolumeSpinBox, qOverload<int>(&QSpinBox::valueChanged), masterVolumeSlider, &QSlider::setValue);

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

    connect(&SettingsCache::instance(), &SettingsCache::langChanged, this, &SoundSettingsPage::retranslateUi);
    retranslateUi();
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
    // search bar
    searchEdit = new SearchLineEdit;
    searchEdit->setObjectName("searchEdit");
    searchEdit->setClearButtonEnabled(true);

    setFocusProxy(searchEdit);
    setFocusPolicy(Qt::ClickFocus);

    // table
    shortcutsTable = new ShortcutTreeView(this);

    shortcutsTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    shortcutsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    shortcutsTable->setColumnWidth(0, width() / 3 * 2);
    searchEdit->setTreeView(shortcutsTable);

    connect(searchEdit, &SearchLineEdit::textChanged, shortcutsTable, &ShortcutTreeView::updateSearchString);

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
    _mainLayout->addWidget(searchEdit);
    _mainLayout->addWidget(shortcutsTable);
    _mainLayout->addWidget(editShortcutGroupBox);
    _mainLayout->addLayout(_buttonsLayout);

    setLayout(_mainLayout);

    connect(btnResetAll, &QPushButton::clicked, this, &ShortcutSettingsPage::resetShortcuts);
    connect(btnClearAll, &QPushButton::clicked, this, &ShortcutSettingsPage::clearShortcuts);

    connect(shortcutsTable, &ShortcutTreeView::currentItemChanged, this, &ShortcutSettingsPage::currentItemChanged);

    connect(&SettingsCache::instance(), &SettingsCache::langChanged, this, &ShortcutSettingsPage::retranslateUi);
    retranslateUi();
}

void ShortcutSettingsPage::currentItemChanged(const QString &key)
{
    if (key.isEmpty()) {
        currentActionGroupName->setText("");
        currentActionName->setText("");
        editTextBox->setShortcutName("");
    } else {
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

void ShortcutSettingsPage::clearShortcuts()
{
    if (QMessageBox::question(this, tr("Clear all default shortcuts"),
                              tr("Do you really want to clear all shortcuts?")) == QMessageBox::Yes) {
        SettingsCache::instance().shortcuts().clearAllShortcuts();
    }
}

void ShortcutSettingsPage::retranslateUi()
{
    shortcutsTable->retranslateUi();

    currentActionGroupLabel->setText(tr("Section:"));
    currentActionLabel->setText(tr("Action:"));
    currentShortcutLabel->setText(tr("Shortcut:"));
    editTextBox->retranslateUi();
    faqLabel->setText(QString("<a href='%1'>%2</a>").arg(WIKI_CUSTOM_SHORTCUTS).arg(tr("How to set custom shortcuts")));
    btnResetAll->setText(tr("Restore all default shortcuts"));
    btnClearAll->setText(tr("Clear all shortcuts"));
    searchEdit->setPlaceholderText(tr("Search by shortcut name"));
}

static QScrollArea *makeScrollable(QWidget *widget)
{
    widget->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Maximum);

    auto *scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setContentsMargins(0, 0, 0, 0);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->horizontalScrollBar()->setEnabled(false);
    scrollArea->setWidget(widget);
    return scrollArea;
}

DlgSettings::DlgSettings(QWidget *parent) : QDialog(parent)
{
    auto rec = QGuiApplication::primaryScreen()->availableGeometry();
    this->setMinimumSize(qMin(700, rec.width()), qMin(700, rec.height()));

    connect(&SettingsCache::instance(), &SettingsCache::langChanged, this, &DlgSettings::updateLanguage);

    contentsWidget = new QListWidget;
    contentsWidget->setViewMode(QListView::IconMode);
    contentsWidget->setIconSize(QSize(58, 50));
    contentsWidget->setMovement(QListView::Static);
    contentsWidget->setMinimumHeight(85);
    contentsWidget->setMaximumHeight(85);
    contentsWidget->setSpacing(5);

    pagesWidget = new QStackedWidget;
    pagesWidget->addWidget(new GeneralSettingsPage);
    pagesWidget->addWidget(makeScrollable(new AppearanceSettingsPage));
    pagesWidget->addWidget(makeScrollable(new UserInterfaceSettingsPage));
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
    connect(buttonBox, &QDialogButtonBox::accepted, this, &DlgSettings::close);

    auto *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(vboxLayout);
    mainLayout->addSpacing(2);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    connect(&SettingsCache::instance(), &SettingsCache::langChanged, this, &DlgSettings::retranslateUi);
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

    connect(contentsWidget, &QListWidget::currentItemChanged, this, &DlgSettings::changePage);
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

void DlgSettings::closeEvent(QCloseEvent *event)
{
    bool showLoadError = true;
    QString loadErrorMessage = tr("Unknown Error loading card database");
    LoadStatus loadStatus = CardDatabaseManager::getInstance()->getLoadStatus();
    qCInfo(DlgSettingsLog) << "Card Database load status: " << loadStatus;
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

    contentsWidget->reset();
}
