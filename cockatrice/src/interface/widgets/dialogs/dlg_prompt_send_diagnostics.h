#ifndef COCKATRICE_DLG_PROMPT_SEND_DIAGNOSTICS_H
#define COCKATRICE_DLG_PROMPT_SEND_DIAGNOSTICS_H

#include "dlg_settings.h"

#include <QDialog>
#include <QLabel>
#include <libcockatrice/interfaces/interface_network_settings_provider.h>

class QPushButton;

class DlgPromptSendDiagnostics : public QDialog
{
    Q_OBJECT
public:
    explicit DlgPromptSendDiagnostics(QWidget *parent = nullptr);

    SendDiagnosticsMode selectedChoice() const
    {
        return choice;
    }

    void retranslateUi();

private:
    SendDiagnosticsMode choice;

    QLabel *messageLabel;
    QPushButton *noButton;
    QPushButton *minimalButton;
    QPushButton *fullButton;
};

#endif // COCKATRICE_DLG_PROMPT_SEND_DIAGNOSTICS_H
