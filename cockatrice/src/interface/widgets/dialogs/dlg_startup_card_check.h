/**
 * @file dlg_startup_card_check.h
 * @ingroup CardDatabaseUpdateDialogs
 */
//! \todo Document this file.

#ifndef DLG_STARTUP_CARD_CHECK_H
#define DLG_STARTUP_CARD_CHECK_H

#include <QDialog>
#include <qtmetamacros.h>

class QButtonGroup;
class QDialogButtonBox;
class QLabel;
class QRadioButton;
class QVBoxLayout;
class QWidget;

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
