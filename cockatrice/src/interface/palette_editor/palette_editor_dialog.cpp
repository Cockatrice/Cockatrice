#include "palette_editor_dialog.h"

#include "../theme_manager.h"
#include "palette_generator.h"
#include "palette_grid_widget.h"
#include "quick_setup_panel.h"

#include <QApplication>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFrame>
#include <QGuiApplication>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QStyleHints>
#include <QTimer>

PaletteEditorDialog::PaletteEditorDialog(const QString &_themeDirPath, const QString &_themeName, QWidget *parent)
    : QDialog(parent), themeDirPath(_themeDirPath), themeName(_themeName)
{
    setMinimumSize(740, 220);
    setupUi();

    // Load both scheme configs upfront so switching is instant
    loadSchemes();

    loadedScheme = themeManager->isDarkMode(themeDirPath) ? "Dark" : "Light";

    schemeComboBox->blockSignals(true);
    schemeComboBox->setCurrentText(loadedScheme);
    schemeComboBox->blockSignals(false);

    paletteGrid->loadPalette(workingConfig[loadedScheme]);
    seedAccentFromScheme(loadedScheme);

    retranslateUi();
}

void PaletteEditorDialog::setupUi()
{
    auto *root = new QVBoxLayout(this);
    root->setSpacing(0);
    root->setContentsMargins(0, 0, 0, 0);

    // Header
    header = new QWidget;
    header->setAutoFillBackground(true);
    {
        QPalette hp = header->palette();
        hp.setColor(QPalette::Window, qApp->palette().color(QPalette::Window).darker(108));
        header->setPalette(hp);
    }
    auto *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(12, 8, 12, 8);

    titleLabel = new QLabel(this);
    titleLabel->setTextFormat(Qt::RichText);

    editingLabel = new QLabel(this);

    schemeComboBox = new QComboBox;
    schemeComboBox->addItems({"Light", "Dark"});
    schemeComboBox->setFixedWidth(90);

    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(editingLabel);
    headerLayout->addWidget(schemeComboBox);
    root->addWidget(header);

    auto makeSeparator = [&]() {
        auto *sep = new QFrame;
        sep->setFrameShape(QFrame::HLine);
        sep->setFrameShadow(QFrame::Plain);
        sep->setFixedHeight(1);
        return sep;
    };

    root->addWidget(makeSeparator());

    // Quick Setup panel
    quickSetupPanel = new QuickSetupPanel;
    quickSetupPanel->setAutoFillBackground(true);

    QPalette sp = quickSetupPanel->palette();
    sp.setColor(QPalette::Window, qApp->palette().color(QPalette::Window).darker(102));
    quickSetupPanel->setPalette(sp);

    root->addWidget(quickSetupPanel);
    root->addWidget(makeSeparator());

    // Toggle button — acts as a section header for the advanced area
    paletteGridToggleButton = new QPushButton(this);
    paletteGridToggleButton->setCheckable(true);
    paletteGridToggleButton->setChecked(false);
    paletteGridToggleButton->setFlat(true);
    paletteGridToggleButton->setStyleSheet("QPushButton { text-align: left; padding: 5px 12px; font-weight: bold; }"
                                           "QPushButton:checked { }");
    root->addWidget(paletteGridToggleButton);

    // Separator + grid start hidden; revealed by the toggle
    paletteGridSeparator = makeSeparator();
    paletteGridSeparator->setVisible(false);
    root->addWidget(paletteGridSeparator);

    paletteGrid = new PaletteGridWidget;
    paletteGrid->setVisible(false);
    root->addWidget(paletteGrid, 1);

    // Footer
    root->addWidget(makeSeparator());
    footer = new QWidget;
    footer->setAutoFillBackground(true);
    {
        QPalette fp = footer->palette();
        fp.setColor(QPalette::Window, qApp->palette().color(QPalette::Window).darker(104));
        footer->setPalette(fp);
    }
    auto *footerLayout = new QHBoxLayout(footer);
    footerLayout->setContentsMargins(12, 8, 12, 8);

    revertButton = new QPushButton(this);

    buttonBox = new QDialogButtonBox;
    resetBtn = buttonBox->addButton(tr("Reset"), QDialogButtonBox::ResetRole);
    applyBtn = buttonBox->addButton(tr("Apply"), QDialogButtonBox::ApplyRole);
    saveBtn = buttonBox->addButton(tr("Save && Apply"), QDialogButtonBox::AcceptRole);
    closeBtn = buttonBox->addButton(QDialogButtonBox::Close);

    footerLayout->addWidget(revertButton);
    footerLayout->addStretch();
    footerLayout->addWidget(buttonBox);
    root->addWidget(footer);

    // Connections
    connect(schemeComboBox, &QComboBox::currentTextChanged, this, &PaletteEditorDialog::onSchemeChanged);
    connect(quickSetupPanel, &QuickSetupPanel::generateRequested, this, &PaletteEditorDialog::onGenerateFromAccent);
    connect(revertButton, &QPushButton::clicked, this, &PaletteEditorDialog::onRevertToDefault);
    connect(resetBtn, &QPushButton::clicked, this, &PaletteEditorDialog::onReset);
    connect(applyBtn, &QPushButton::clicked, this, &PaletteEditorDialog::onApply);
    connect(saveBtn, &QPushButton::clicked, this, &PaletteEditorDialog::onSave);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);

    connect(paletteGridToggleButton, &QPushButton::toggled, this, [this](bool open) {
        paletteGridToggleButton->setText(open ? tr("▼  Edit Palette") : tr("▶  Edit Palette"));
        paletteGridSeparator->setVisible(open);
        paletteGrid->setVisible(open);

        if (open) {
            setMinimumHeight(680);
            resize(width(), 680);
        } else {
            setMinimumHeight(220);
            adjustSize(); // shrinks to fit just the visible content
        }
    });
}

void PaletteEditorDialog::retranslateUi()
{
    setWindowTitle(tr("Palette Editor — %1").arg(themeName));
    titleLabel->setText(tr("<b>Palette Editor</b> &nbsp;·&nbsp; %1").arg(themeName));

    // Revert button only makes sense when the theme ships default palette files
    const bool hasDefault = PaletteConfig::fromDefault(themeDirPath, "Light").hasPalette() ||
                            PaletteConfig::fromDefault(themeDirPath, "Dark").hasPalette();
    revertButton->setEnabled(hasDefault);
    if (!hasDefault) {
        revertButton->setToolTip(tr("This theme ships no default palette files"));
    } else {
        revertButton->setToolTip(tr("Replace current colours with the theme author's defaults"));
    }

    schemeComboBox->setToolTip(tr("Switch between the light and dark palette files"));
    editingLabel->setText(tr("Editing:"));
    paletteGridToggleButton->setText(tr("▶  Edit Palette"));
    paletteGridToggleButton->setToolTip(tr("Show or hide the per-role colour grid for manual tweaks"));
    revertButton->setText(tr("↺  Revert to theme default"));

    resetBtn->setText(tr("Reset"));
    applyBtn->setText(tr("Apply"));
    saveBtn->setText(tr("Save && Apply"));
    resetBtn->setToolTip(tr("Discard unsaved edits and restore the last saved palette"));
    applyBtn->setToolTip(tr("Preview this palette without saving to disk"));
    saveBtn->setToolTip(tr("Write palette-%1.toml and reload the theme").arg(loadedScheme.toLower()));

    if (themeDirPath.isEmpty()) {
        saveBtn->setEnabled(false);
        saveBtn->setToolTip(tr("Cannot save: this theme has no directory on disk"));
    }
}

void PaletteEditorDialog::loadSchemes()
{
    const QStringList schemes = {"Light", "Dark"};
    for (const QString &scheme : schemes) {
        PaletteConfig cfg = PaletteConfig::fromScheme(themeDirPath, scheme);

        if (!cfg.hasPalette()) {
            cfg = PaletteConfig::fromDefault(themeDirPath, scheme);
        }

        if (!cfg.hasPalette()) {
            const QPalette appPal = qApp->palette();
            for (auto group : {QPalette::Active, QPalette::Disabled, QPalette::Inactive}) {
                for (int i = 0; i < QPalette::NColorRoles; ++i) {
                    auto role = static_cast<QPalette::ColorRole>(i);
                    if (role != QPalette::NoRole) {
                        cfg.colors[group][role] = appPal.color(group, role);
                    }
                }
            }
        }
        savedConfig[scheme] = cfg;
        workingConfig[scheme] = cfg;
    }
}

void PaletteEditorDialog::seedAccentFromScheme(const QString &scheme)
{
    QColor seed = workingConfig.value(scheme).colors.value(QPalette::Active).value(QPalette::Highlight);
    if (seed.isValid()) {
        quickSetupPanel->setAccentColor(seed);
    }
}

void PaletteEditorDialog::onSchemeChanged(const QString &scheme)
{
    // Snapshot unsaved edits for the scheme we're leaving
    if (!loadedScheme.isEmpty()) {
        workingConfig[loadedScheme] = paletteGrid->currentPaletteConfig();
    }

    loadedScheme = scheme;
    paletteGrid->loadPalette(workingConfig.value(scheme));
    seedAccentFromScheme(scheme);
    onApply();
}

void PaletteEditorDialog::onGenerateFromAccent(const QColor &accent, int intensity)
{
    PaletteConfig cfg = PaletteGenerator::fromAccent(accent, intensity, loadedScheme);
    workingConfig[loadedScheme] = cfg;
    paletteGrid->loadPalette(cfg);
}

void PaletteEditorDialog::onApply()
{
    themeManager->previewPalette(paletteGrid->currentPaletteConfig(), loadedScheme);
}

void PaletteEditorDialog::onSave()
{
    if (loadedScheme.isEmpty()) {
        return;
    }

    PaletteConfig cfg = paletteGrid->currentPaletteConfig();

    if (!ThemeManager::savePaletteConfig(themeDirPath, loadedScheme, cfg)) {
        QMessageBox::warning(this, tr("Save failed"),
                             tr("Could not write %1 to:\n%2").arg(PaletteConfig::fileName(loadedScheme), themeDirPath));
        return;
    }

    ThemeConfig globalCfg = ThemeConfig::fromThemeDir(themeDirPath);
    globalCfg.colorScheme = loadedScheme;
    globalCfg.save(themeDirPath);

    savedConfig[loadedScheme] = cfg;
    workingConfig[loadedScheme] = cfg;
    themeManager->reloadCurrentTheme();
    accept();
}

void PaletteEditorDialog::onReset()
{
    workingConfig[loadedScheme] = savedConfig[loadedScheme];
    paletteGrid->loadPalette(savedConfig[loadedScheme]);
}

void PaletteEditorDialog::onRevertToDefault()
{
    PaletteConfig def = PaletteConfig::fromDefault(themeDirPath, loadedScheme);
    if (!def.hasPalette()) {
        QMessageBox::information(this, tr("No default found"),
                                 tr("No default palette file found for the \"%1\" scheme.").arg(loadedScheme));
        return;
    }
    workingConfig[loadedScheme] = def;
    paletteGrid->loadPalette(def);
}

void PaletteEditorDialog::changeEvent(QEvent *e)
{
    if (e->type() == QEvent::PaletteChange) {
        QTimer::singleShot(0, this, &PaletteEditorDialog::refreshChromePalettes);
    }

    QDialog::changeEvent(e);
}

void PaletteEditorDialog::refreshChromePalettes()
{
    const QPalette base = qApp->palette();

    if (header) {
        QPalette hp = header->palette();
        hp.setColor(QPalette::Window, base.color(QPalette::Window).darker(108));
        header->setPalette(hp);
        header->update();
    }
    if (footer) {
        QPalette fp = footer->palette();
        fp.setColor(QPalette::Window, base.color(QPalette::Window).darker(104));
        footer->setPalette(fp);
        footer->update();
    }
    if (quickSetupPanel) {
        QPalette sp = quickSetupPanel->palette();
        sp.setColor(QPalette::Window, base.color(QPalette::Window).darker(102));
        quickSetupPanel->setPalette(sp);
        quickSetupPanel->update();
    }
}
