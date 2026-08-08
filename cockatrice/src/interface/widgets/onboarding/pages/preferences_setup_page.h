#ifndef PREFERENCES_SETUP_PAGE_H
#define PREFERENCES_SETUP_PAGE_H

#include "../first_run_wizard_page.h"

class QCheckBox;
class QGroupBox;

/** @brief A curated subset of settings for the user to adjust.
 **/
class PreferencesSetupPage : public FirstRunWizardPage
{
    Q_OBJECT

public:
    explicit PreferencesSetupPage(QWidget *parent = nullptr);

    void initializePage() override;
    bool isSkippable() const override;
    QString stepTitle() const override;
    QString stepSubtitle() const override;
    void retranslateUi() override;

private:
    QGroupBox *appearanceGroup;
    QCheckBox *styleUserListCheckBox;
    QCheckBox *cardScalingCheckBox;
    QCheckBox *roundCardCornersCheckBox;
    QCheckBox *displayCardNamesCheckBox;
    QCheckBox *autoRotateCardsCheckBox;
    QCheckBox *tapAnimationCheckBox;

    QGroupBox *notificationsGroup;
    QCheckBox *notificationsEnabledCheckBox;
    QCheckBox *soundEnabledCheckBox;

    QGroupBox *gameplayGroup;
    QCheckBox *doubleClickToPlayCheckBox;
    QCheckBox *horizontalHandCheckBox;
    QCheckBox *playToStackCheckBox;

    QGroupBox *menuGroup;
    QCheckBox *showShortcutsCheckBox;

    QGroupBox *dataGroup;
    QCheckBox *picDownloadCheckBox;
    QCheckBox *checkUpdatesOnStartupCheckBox;
    QCheckBox *showTipsOnStartupCheckBox;
};

#endif // PREFERENCES_SETUP_PAGE_H
