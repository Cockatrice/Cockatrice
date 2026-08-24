#ifndef PREFERENCES_SETUP_PAGE_H
#define PREFERENCES_SETUP_PAGE_H

#include "../first_run_wizard_page.h"

class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;

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
    QGroupBox *gameplayGroup;
    QCheckBox *doubleClickToPlayCheckBox;
    QCheckBox *horizontalHandCheckBox;
    QCheckBox *playToStackCheckBox;

    QGroupBox *notificationsGroup;
    QCheckBox *notificationsEnabledCheckBox;
    QCheckBox *soundEnabledCheckBox;

    QGroupBox *startupGroup;
    QLabel *startupTabLabel;
    QComboBox *startupTabSelector;
};

#endif // PREFERENCES_SETUP_PAGE_H
