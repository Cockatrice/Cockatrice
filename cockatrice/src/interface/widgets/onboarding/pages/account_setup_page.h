#ifndef ACCOUNT_SETUP_PAGE_H
#define ACCOUNT_SETUP_PAGE_H

#include "../first_run_wizard_page.h"

class QLabel;
class QPushButton;

/** @brief First-run account step. Does NOT embed DlgRegister's fields: they exist
 *         to be handed to ConnectionController's network registration flow, which
 *         this wizard has no visibility into. Reimplementing the fields here
 *         without that wiring would look functional and silently do nothing --
 *         worse than reuse. So: a friendly landing spot that opens the *existing*
 *         DlgRegister / connect flow via signals FirstRunWizard forwards. */
class AccountSetupPage : public FirstRunWizardPage
{
    Q_OBJECT

public:
    explicit AccountSetupPage(QWidget *parent = nullptr);

    bool isSkippable() const override;
    QString stepTitle() const override;
    QString stepSubtitle() const override;
    void retranslateUi() override;

signals:
    void registerRequested();
    void connectRequested();

private:
    QLabel *bodyLabel;
    QPushButton *registerButton;
    QPushButton *connectButton;
    QLabel *skipHintLabel;
};

#endif // ACCOUNT_SETUP_PAGE_H
