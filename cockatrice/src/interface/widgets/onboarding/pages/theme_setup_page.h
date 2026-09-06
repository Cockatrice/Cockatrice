#ifndef THEME_SETUP_PAGE_H
#define THEME_SETUP_PAGE_H

#include "../first_run_wizard_page.h"

class QComboBox;
class QGroupBox;
class QuickSetupPanel;

/** @brief First-run theme step. Reuses the same building blocks as Appearance
 *         settings and the Palette Editor (ThemeManager, PaletteConfig,
 *         PaletteGenerator, and the QuickSetupPanel widget itself) rather than
 *         reimplementing palette generation or preview here.
 *
 * Behavior specific to this page (deliberately not pushed down into
 * ThemeManager, to avoid changing app-wide behaviour for existing installs):
 *  - Opening the page never changes the running palette; previews and
 *    auto-generation only happen when the user actually changes a control.
 *  - If a theme+scheme the user selects has no saved palette and no shipped
 *    default, one is generated from the QuickSetupPanel's current accent so
 *    the preview doesn't fall back to a flat, unstyled look. */
class ThemeSetupPage : public FirstRunWizardPage
{
    Q_OBJECT

public:
    explicit ThemeSetupPage(QWidget *parent = nullptr);

    void initializePage() override;
    bool validatePage() override;
    bool isSkippable() const override;
    QString stepTitle() const override;
    QString stepSubtitle() const override;
    void retranslateUi() override;

private slots:
    void onThemeChanged(int index);
    void onSchemeChanged();
    void onGenerateFromAccent(const QColor &accent, int intensity);
    void onHomeTabBackgroundChanged(int index);

private:
    QString currentScheme() const;
    QString resolvedScheme() const; // "System" -> actual Light/Dark
    void maybeAutoGeneratePalette();
    QString writableThemeDir() const;

    QComboBox *themeCombo;
    QComboBox *schemeCombo;
    QGroupBox *accentGroup;
    QuickSetupPanel *quickSetupPanel;

    QComboBox *homeTabBackgroundCombo;

    bool paletteDirty = false;
};

#endif // THEME_SETUP_PAGE_H
