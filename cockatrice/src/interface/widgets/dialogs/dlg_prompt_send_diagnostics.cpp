#include "dlg_prompt_send_diagnostics.h"

#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>

DlgPromptSendDiagnostics::DlgPromptSendDiagnostics(QWidget *parent) : QDialog(parent), choice(SendDiagnosticsDisabled)
{
    auto *layout = new QVBoxLayout(this);

    messageLabel = new QLabel(this);
    messageLabel->setWordWrap(true);
    layout->addWidget(messageLabel);

    auto *buttonLayout = new QHBoxLayout;

    noButton = new QPushButton(this);
    minimalButton = new QPushButton(this);
    fullButton = new QPushButton(this);

    buttonLayout->addWidget(noButton);
    buttonLayout->addWidget(minimalButton);
    buttonLayout->addWidget(fullButton);

    layout->addLayout(buttonLayout);

    // Connect buttons
    connect(noButton, &QPushButton::clicked, this, [this]() {
        choice = SendDiagnosticsDisabled;
        accept();
    });
    connect(minimalButton, &QPushButton::clicked, this, [this]() {
        choice = SendDiagnosticsBasic;
        accept();
    });
    connect(fullButton, &QPushButton::clicked, this, [this]() {
        choice = SendDiagnosticsFull;
        accept();
    });

    retranslateUi();
}

void DlgPromptSendDiagnostics::retranslateUi()
{
    setWindowTitle(tr("Help us improve Cockatrice"));

    messageLabel->setText(tr("Hi there! Cockatrice is made by a small team, and we want to make it better for you.\n\n"
                             "We’d love to collect a tiny bit of anonymous technical data about how you use the app — "
                             "nothing personal, just usage info so we know what works and what needs improvement."));

    noButton->setText(tr("No, thank you"));
    minimalButton->setText(tr("Minimal"));
    fullButton->setText(tr("Full"));
}
