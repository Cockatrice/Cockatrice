#include "appearance_settings_page.h"

#include "../../../client/settings/cache_settings.h"
#include "../../client/settings/card_counter_settings.h"
#include "../../palette_editor/palette_editor_dialog.h"
#include "../dialogs/override_printing_warning.h"
#include "../interface/theme_manager.h"
#include "../interface/widgets/general/background_sources.h"

#include <QColorDialog>
#include <QDesktopServices>
#include <QGridLayout>
#include <QMessageBox>
#include <QTimer>

AppearanceSettingsPage::AppearanceSettingsPage()
{
    SettingsCache &settings = SettingsCache::instance();

    // Theme settings
    QString themeName = SettingsCache::instance().getThemeName();

    QStringList themeDirs = themeManager->getAvailableThemes().keys();
    for (int i = 0; i < themeDirs.size(); i++) {
        themeBox.addItem(themeDirs[i]);
        if (themeDirs[i] == themeName) {
            themeBox.setCurrentIndex(i);
        }
    }

    connect(&themeBox, qOverload<int>(&QComboBox::currentIndexChanged), this, &AppearanceSettingsPage::themeBoxChanged);
    connect(&openThemeButton, &QPushButton::clicked, this, &AppearanceSettingsPage::openThemeLocation);

    schemeCombo.addItem(tr("Light"), QStringLiteral("Light"));
    schemeCombo.addItem(tr("Dark"), QStringLiteral("Dark"));
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    schemeCombo.addItem(tr("System"), QStringLiteral("System"));
#endif

    // Seed from whatever the current theme already has saved
    const QString dirPath = themeManager->getAvailableThemes().value(SettingsCache::instance().getThemeName());
    const ThemeConfig cfg = ThemeConfig::fromThemeDir(dirPath);
    const QString current = cfg.colorScheme;
    const int seedIdx = schemeCombo.findData(current);
    schemeCombo.setCurrentIndex(seedIdx >= 0 ? seedIdx : 0);

    connect(&schemeCombo, &QComboBox::currentIndexChanged, this,
            [this] { themeManager->setColorScheme(schemeCombo.currentData().toString()); });

    connect(themeManager, &ThemeManager::themeChanged, this, [this, dirPath] {
        const QString newDir = themeManager->getAvailableThemes().value(SettingsCache::instance().getThemeName());
        const ThemeConfig cfg = ThemeConfig::fromThemeDir(newDir);
        const QString current = cfg.colorScheme;

        schemeCombo.blockSignals(true);
        const int idx = schemeCombo.findData(current);
        schemeCombo.setCurrentIndex(idx >= 0 ? idx : 0);
        schemeCombo.blockSignals(false);
    });

    connect(&editPaletteButton, &QPushButton::clicked, this, &AppearanceSettingsPage::editPalette);

    auto *themeGrid = new QGridLayout;
    themeGrid->addWidget(&themeLabel, 0, 0);
    themeGrid->addWidget(&themeBox, 0, 1);
    themeGrid->addWidget(&openThemeButton, 1, 1);
    themeGrid->addWidget(&schemeComboLabel, 2, 0);
    themeGrid->addWidget(&schemeCombo, 2, 1);
    themeGrid->addWidget(&editPaletteButton, 3, 1);

    themeGroupBox = new QGroupBox;
    themeGroupBox->setLayout(themeGrid);

    // Home tab settings
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
        updateHomeTabSettingsVisibility();
    });

    homeTabBackgroundShuffleFrequencySpinBox.setRange(0, 3600);
    homeTabBackgroundShuffleFrequencySpinBox.setSuffix(tr(" seconds"));
    homeTabBackgroundShuffleFrequencySpinBox.setValue(SettingsCache::instance().getHomeTabBackgroundShuffleFrequency());
    connect(&homeTabBackgroundShuffleFrequencySpinBox, qOverload<int>(&QSpinBox::valueChanged),
            &SettingsCache::instance(), &SettingsCache::setHomeTabBackgroundShuffleFrequency);

    homeTabDisplayCardNameCheckBox.setChecked(settings.getHomeTabDisplayCardName());
    connect(&homeTabDisplayCardNameCheckBox, &QCheckBox::QT_STATE_CHANGED, &settings,
            &SettingsCache::setHomeTabDisplayCardName);

    updateHomeTabSettingsVisibility();

    auto *homeTabGrid = new QGridLayout;
    homeTabGrid->addWidget(&homeTabBackgroundSourceLabel, 0, 0);
    homeTabGrid->addWidget(&homeTabBackgroundSourceBox, 0, 1);
    homeTabGrid->addWidget(&homeTabBackgroundShuffleFrequencyLabel, 1, 0);
    homeTabGrid->addWidget(&homeTabBackgroundShuffleFrequencySpinBox, 1, 1);
    homeTabGrid->addWidget(&homeTabDisplayCardNameCheckBox, 2, 0, 1, 2);

    homeTabGroupBox = new QGroupBox;
    homeTabGroupBox->setLayout(homeTabGrid);

    // Menu settings
    showShortcutsCheckBox.setChecked(settings.getShowShortcuts());
    connect(&showShortcutsCheckBox, &QCheckBox::QT_STATE_CHANGED, this, &AppearanceSettingsPage::showShortcutsChanged);

    showGameSelectorFilterToolbarCheckBox.setChecked(settings.getShowGameSelectorFilterToolbar());
    connect(&showGameSelectorFilterToolbarCheckBox, &QCheckBox::QT_STATE_CHANGED, &settings,
            &SettingsCache::setShowGameSelectorFilterToolbar);

    auto *menuGrid = new QGridLayout;
    menuGrid->addWidget(&showShortcutsCheckBox, 0, 0);
    menuGrid->addWidget(&showGameSelectorFilterToolbarCheckBox, 1, 0);

    menuGroupBox = new QGroupBox;
    menuGroupBox->setLayout(menuGrid);

    // Printings settings
    overrideAllCardArtWithPersonalPreferenceCheckBox.setChecked(settings.getOverrideAllCardArtWithPersonalPreference());
    connect(&overrideAllCardArtWithPersonalPreferenceCheckBox, &QCheckBox::QT_STATE_CHANGED, this,
            &AppearanceSettingsPage::overrideAllCardArtWithPersonalPreferenceToggled);

    bumpSetsWithCardsInDeckToTopCheckBox.setChecked(settings.getBumpSetsWithCardsInDeckToTop());
    connect(&bumpSetsWithCardsInDeckToTopCheckBox, &QCheckBox::QT_STATE_CHANGED, &settings,
            &SettingsCache::setBumpSetsWithCardsInDeckToTop);

    auto *printingsGrid = new QGridLayout;
    printingsGrid->addWidget(&overrideAllCardArtWithPersonalPreferenceCheckBox, 0, 0, 1, 2);
    printingsGrid->addWidget(&bumpSetsWithCardsInDeckToTopCheckBox, 1, 0, 1, 2);

    printingsGroupBox = new QGroupBox;
    printingsGroupBox->setLayout(printingsGrid);

    // Card rendering
    displayCardNamesCheckBox.setChecked(settings.getDisplayCardNames());
    connect(&displayCardNamesCheckBox, &QCheckBox::QT_STATE_CHANGED, &settings, &SettingsCache::setDisplayCardNames);

    autoRotateSidewaysLayoutCardsCheckBox.setChecked(settings.getAutoRotateSidewaysLayoutCards());
    connect(&autoRotateSidewaysLayoutCardsCheckBox, &QCheckBox::QT_STATE_CHANGED, &settings,
            &SettingsCache::setAutoRotateSidewaysLayoutCards);

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
    cardsGrid->addWidget(&verticalCardOverlapPercentLabel, 4, 0, 1, 1);
    cardsGrid->addWidget(&verticalCardOverlapPercentBox, 4, 1, 1, 1);
    cardsGrid->addWidget(&cardViewInitialRowsMaxLabel, 5, 0);
    cardsGrid->addWidget(&cardViewInitialRowsMaxBox, 5, 1);
    cardsGrid->addWidget(&cardViewExpandedRowsMaxLabel, 6, 0);
    cardsGrid->addWidget(&cardViewExpandedRowsMaxBox, 6, 1);

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
            if (!newColor.isValid()) {
                return;
            }

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
    mainLayout->addWidget(homeTabGroupBox);
    mainLayout->addWidget(menuGroupBox);
    mainLayout->addWidget(printingsGroupBox);
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
    if (index >= 0 && index < themeDirs.count()) {
        SettingsCache::instance().setThemeName(themeDirs.at(index));
    }
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

void AppearanceSettingsPage::editPalette()
{
    PaletteEditorDialog dlg(themeManager->getCurrentThemePath(), SettingsCache::instance().getThemeName(), this);
    dlg.exec();
}

void AppearanceSettingsPage::updateHomeTabSettingsVisibility()
{
    bool visible =
        SettingsCache::instance().getHomeTabBackgroundSource() != BackgroundSources::toId(BackgroundSources::Theme);

    homeTabBackgroundShuffleFrequencyLabel.setVisible(visible);
    homeTabBackgroundShuffleFrequencySpinBox.setVisible(visible);
    homeTabDisplayCardNameCheckBox.setVisible(visible);
}

void AppearanceSettingsPage::showShortcutsChanged(QT_STATE_CHANGED_T value)
{
    SettingsCache::instance().setShowShortcuts(value);
    qApp->setAttribute(Qt::AA_DontShowShortcutsInContextMenus, value == 0); // 0 = unchecked
}

void AppearanceSettingsPage::overrideAllCardArtWithPersonalPreferenceToggled(QT_STATE_CHANGED_T value)
{
    bool enable = static_cast<bool>(value);

    bool accepted = OverridePrintingWarning::execMessageBox(this, enable);

    if (!accepted) {
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
    schemeComboLabel.setText(tr("Active theme palette:"));
    editPaletteButton.setText(tr("Edit theme palette"));

    homeTabGroupBox->setTitle(tr("Home tab settings"));
    homeTabBackgroundSourceLabel.setText(tr("Home tab background source:"));
    homeTabBackgroundShuffleFrequencyLabel.setText(tr("Home tab background shuffle frequency:"));
    homeTabBackgroundShuffleFrequencySpinBox.setSpecialValueText(tr("Disabled"));
    homeTabDisplayCardNameCheckBox.setText(tr("Display card name of background in bottom right"));

    menuGroupBox->setTitle(tr("Menu settings"));
    showShortcutsCheckBox.setText(tr("Show keyboard shortcuts in right-click menus"));
    showGameSelectorFilterToolbarCheckBox.setText(tr("Show game filter toolbar above list in room tab"));

    printingsGroupBox->setTitle(tr("Card printings"));
    overrideAllCardArtWithPersonalPreferenceCheckBox.setText(
        tr("Override all card art with personal set preference (Pre-ProviderID change behavior)"));
    bumpSetsWithCardsInDeckToTopCheckBox.setText(
        tr("Bump sets that the deck contains cards from to the top in the printing selector"));

    cardsGroupBox->setTitle(tr("Card rendering"));
    displayCardNamesCheckBox.setText(tr("Display card names on cards having a picture"));
    autoRotateSidewaysLayoutCardsCheckBox.setText(tr("Auto-Rotate cards with sideways layout"));
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