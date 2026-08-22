#ifndef WELCOME_PAGE_H
#define WELCOME_PAGE_H

#include "../first_run_wizard_page.h"

class QLabel;

class WelcomePage : public FirstRunWizardPage
{
    Q_OBJECT

public:
    explicit WelcomePage(QWidget *parent = nullptr);

    QString stepTitle() const override;
    void retranslateUi() override;

private:
    QLabel *bodyLabel;
};

#endif // WELCOME_PAGE_H
