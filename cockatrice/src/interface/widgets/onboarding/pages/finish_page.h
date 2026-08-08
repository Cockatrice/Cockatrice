#ifndef FINISH_PAGE_H
#define FINISH_PAGE_H

#include "../first_run_wizard_page.h"

class QLabel;

class FinishPage : public FirstRunWizardPage
{
    Q_OBJECT

public:
    explicit FinishPage(QWidget *parent = nullptr);

    QString stepTitle() const override;
    void retranslateUi() override;

private:
    QLabel *bodyLabel;
};

#endif // FINISH_PAGE_H
