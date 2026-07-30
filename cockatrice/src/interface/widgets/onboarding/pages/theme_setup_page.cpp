#include "theme_setup_page.h"

#include "../../client/settings/cache_settings.h"
#include "../../interface/palette_editor/palette_generator.h"
#include "../../interface/palette_editor/quick_setup_panel.h"
#include "../../interface/theme_manager.h"
#include "../../interface/widgets/general/background_sources.h"

#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>
#include <libcockatrice/settings/personal_settings.h>

ThemeSetupPage::ThemeSetupPage(QWidget *parent) : FirstRunWizardPage(parent)
{
    themeCombo = new QComboBox(this);
    schemeCombo = new QComboBox(this);
    schemeCombo->addItem(tr("Light"), QStringLiteral("Light"));
    schemeCombo->addItem(tr("Dark"), QStringLiteral("Dark"));
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    schemeCombo->addItem(tr("Match system"), QStringLiteral("System"));
#endif

    quickSetupPanel = new QuickSetupPanel(this);

    connect(themeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ThemeSetupPage::onThemeChanged);
    connect(schemeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ThemeSetupPage::onSchemeChanged);
    connect(quickSetupPanel, &QuickSetupPanel::valueChanged, this, &ThemeSetupPage::onGenerateFromAccent);

    homeTabBackgroundCombo = new QComboBox(this);
    for (const auto &entry : BackgroundSources::all()) {
        homeTabBackgroundCombo->addItem(QObject::tr(entry.trKey), QVariant::fromValue(entry.type));
    }
    connect(homeTabBackgroundCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &ThemeSetupPage::onHomeTabBackgroundChanged);

    // Keep the scheme combo honest when the *theme* changes underneath it
    // (switching theme reloads that theme's own stored colorScheme), and
    // opportunistically seed a palette for themes that ship none at all.
    // Mirrors AppearanceSettingsPage's identical listener for the combo-sync
    // half of this.
    connect(themeManager, &ThemeManager::themeChanged, this, [this] {
        const QString newDir = themeManager->getAvailableThemes().value(SettingsCache::instance().getThemeName());
        const ThemeConfig cfg = ThemeConfig::fromThemeDir(newDir);
        const QString current = cfg.colorScheme;

        schemeCombo->blockSignals(true);
        const int idx = schemeCombo->findData(current);
        schemeCombo->setCurrentIndex(idx >= 0 ? idx : 0);
        schemeCombo->blockSignals(false);

        maybeAutoGeneratePalette();
    });

    auto *form = new QFormLayout;
    form->addRow(tr("Theme:"), themeCombo);
    form->addRow(tr("Appearance:"), schemeCombo);
    form->addRow(tr("Home screen background:"), homeTabBackgroundCombo);

    accentGroup = new QGroupBox(this);
    auto *accentLayout = new QVBoxLayout(accentGroup);
    accentLayout->addWidget(quickSetupPanel);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(accentGroup);
    layout->addStretch();

    retranslateUi();
}

void ThemeSetupPage::initializePage()
{
    themeCombo->blockSignals(true);
    themeCombo->clear();
    const QString currentTheme = SettingsCache::instance().getThemeName();
    for (const QString &name : themeManager->getAvailableThemes().keys()) {
        themeCombo->addItem(name);
    }
    const int idx = themeCombo->findText(currentTheme);
    themeCombo->setCurrentIndex(idx >= 0 ? idx : 0);
    themeCombo->blockSignals(false);

    const QString dirPath = themeManager->getAvailableThemes().value(SettingsCache::instance().getThemeName());
    const ThemeConfig cfg = ThemeConfig::fromThemeDir(dirPath);
    schemeCombo->blockSignals(true);
    const int schemeIdx = schemeCombo->findData(cfg.colorScheme);
    schemeCombo->setCurrentIndex(schemeIdx >= 0 ? schemeIdx : 0);
    schemeCombo->blockSignals(false);

    homeTabBackgroundCombo->blockSignals(true);
    QString homeTabSource = SettingsCache::instance().personal().getHomeTabBackgroundSource();
    int homeTabIdx = homeTabBackgroundCombo->findData(BackgroundSources::fromId(homeTabSource));
    homeTabBackgroundCombo->setCurrentIndex(homeTabIdx >= 0 ? homeTabIdx : 0);
    homeTabBackgroundCombo->blockSignals(false);

    paletteDirty = false;
    maybeAutoGeneratePalette();
}

QString ThemeSetupPage::currentScheme() const
{
    return schemeCombo->currentData().toString();
}

QString ThemeSetupPage::resolvedScheme() const
{
    const QString scheme = currentScheme();
    if (scheme.isEmpty() || scheme == QStringLiteral("System")) {
        return themeManager->isDarkMode(themeManager->getCurrentThemePath()) ? "Dark" : "Light";
    }
    return scheme;
}

void ThemeSetupPage::onThemeChanged(int index)
{
    if (index < 0) {
        return;
    }
    paletteDirty = false;
    SettingsCache::instance().setThemeName(themeCombo->itemText(index));
    // Scheme-combo sync and auto-generation both happen via the
    // ThemeManager::themeChanged listener above, triggered by setThemeName.
}

void ThemeSetupPage::onSchemeChanged()
{
    themeManager->setColorScheme(currentScheme());
}

void ThemeSetupPage::onHomeTabBackgroundChanged(int index)
{
    if (index < 0) {
        return;
    }
    auto type = homeTabBackgroundCombo->currentData().value<BackgroundSources::Type>();
    SettingsCache::instance().personal().setHomeTabBackgroundSource(BackgroundSources::toId(type));
}

void ThemeSetupPage::onGenerateFromAccent(const QColor &accent, int intensity)
{
    PaletteConfig cfg = PaletteGenerator::fromAccent(accent, intensity, resolvedScheme());
    themeManager->previewPalette(cfg, resolvedScheme());
    paletteDirty = true;
}

void ThemeSetupPage::maybeAutoGeneratePalette()
{
    const QString dirPath = themeManager->getAvailableThemes().value(SettingsCache::instance().getThemeName());
    const QString scheme = resolvedScheme();

    if (PaletteConfig::fromScheme(dirPath, scheme).hasPalette() ||
        PaletteConfig::fromDefault(dirPath, scheme).hasPalette()) {
        return; // theme already has something real to show -- leave it alone
    }

    // Nothing saved, nothing shipped. Rather than showing flat native Qt
    // colours during the very first thing a new user sees, seed one from
    // whatever accent QuickSetupPanel currently holds (its own built-in
    // default the first time through), and mark it dirty so it's written to
    // disk if the user moves on without touching the accent controls.
    PaletteConfig generated =
        PaletteGenerator::fromAccent(quickSetupPanel->accentColor(), quickSetupPanel->intensity(), scheme);
    themeManager->previewPalette(generated, scheme);
    paletteDirty = true;
}

bool ThemeSetupPage::validatePage()
{
    if (paletteDirty) {
        const QString dirPath = themeManager->getAvailableThemes().value(SettingsCache::instance().getThemeName());
        const QString scheme = resolvedScheme();
        PaletteConfig cfg =
            PaletteGenerator::fromAccent(quickSetupPanel->accentColor(), quickSetupPanel->intensity(), scheme);
        ThemeManager::commitPalette(dirPath, scheme, cfg);
        themeManager->reloadCurrentTheme();
    }
    return true;
}

bool ThemeSetupPage::isSkippable() const
{
    return true;
}

QString ThemeSetupPage::stepTitle() const
{
    return tr("Pick a Look");
}

QString ThemeSetupPage::stepSubtitle() const
{
    return tr("You can fine-tune every colour later from Settings → Appearance.");
}

void ThemeSetupPage::retranslateUi()
{
    accentGroup->setTitle(tr("Accent colour (optional)"));
}