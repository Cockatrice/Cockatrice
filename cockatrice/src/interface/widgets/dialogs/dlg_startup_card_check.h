/**
 * @file dlg_startup_card_check.h
 * @ingroup CardDatabaseUpdateDialogs
 * @brief TODO: Document this.
 */

#ifndef DLG_STARTUP_CARD_CHECK_H
#define DLG_STARTUP_CARD_CHECK_H

#include <QButtonGroup>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QRadioButton>
#include <QVBoxLayout>

class DlgStartupCardCheck : public QDialog
{
    Q_OBJECT
public:
    explicit DlgStartupCardCheck(QWidget *parent);

    QVBoxLayout *layout;
    QLabel *instructionLabel;
    QButtonGroup *group;
    QRadioButton *foregroundBtn, *backgroundBtn, *backgroundAlwaysBtn, *dontPromptBtn, *dontRunBtn;
    QDialogButtonBox *buttonBox;
};

#endif // DLG_STARTUP_CARD_CHECK_H
