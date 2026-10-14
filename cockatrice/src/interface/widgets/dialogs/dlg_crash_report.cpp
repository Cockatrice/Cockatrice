#include "dlg_crash_report.h"

#include "../../../client/crash/crash_reporter.h"
#include "../../../client/settings/cache_settings.h"
#include "../../logger.h"

#include <QCheckBox>
#include <QDateTime>
#include <QDir>
#include <QEvent>
#include <QFile>
#include <QHBoxLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QList>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <libcockatrice/settings/debug_settings.h>

class QWidget;

DlgCrashReport::DlgCrashReport(QWidget *parent, bool _realSending, CrashReporter *_reporter)
    : QDialog(parent), realSending(_realSending), reporter(_reporter)
{
    infoLabel = new QLabel(this);
    infoLabel->setWordWrap(true);

    accountLabel = new QLabel(this);
    accountLabel->setWordWrap(true);

    previewArea = new QPlainTextEdit(this);
    previewArea->setReadOnly(true);
    previewArea->setPlainText(buildRedactedPreview());

    rememberBox = new QCheckBox(this);

    sendButton = new QPushButton(this);
    sendButton->setDefault(true);
    connect(sendButton, &QPushButton::clicked, this, &DlgCrashReport::actSend);

    dontSendButton = new QPushButton(this);
    dontSendButton->setAutoDefault(false);
    connect(dontSendButton, &QPushButton::clicked, this, &DlgCrashReport::actDontSend);

    auto *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(rememberBox);
    buttonLayout->addStretch();
    buttonLayout->addWidget(dontSendButton);
    buttonLayout->addWidget(sendButton);

    auto *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(infoLabel);
    mainLayout->addWidget(accountLabel);
    mainLayout->addWidget(previewArea);
    mainLayout->addLayout(buttonLayout);
    setLayout(mainLayout);

    resize(700, 500);
    retranslateUi();
}

void DlgCrashReport::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QDialog::changeEvent(event);
}

void DlgCrashReport::retranslateUi()
{
    setWindowTitle(tr("Send crash report"));
    if (realSending) {
        infoLabel->setText(tr("Cockatrice closed unexpectedly. The preview below shows what will be sent."));
    } else {
        infoLabel->setText(
            tr("Cockatrice closed unexpectedly. The preview below shows what a crash report would contain (mock)."));
    }
    accountLabel->setText(tr("No account needed to send a crash report."));
    rememberBox->setText(tr("Remember my choice"));
    sendButton->setText(tr("Send"));
    dontSendButton->setText(tr("Don't send"));
}

QString DlgCrashReport::buildRedactedPreview()
{
    Logger &logger = Logger::getInstance();

    QStringList header;
    header.append(logger.getClientVersion());
    header.append(logger.getSystemArchitecture());
    header.append(logger.getSystemLocale());
    header.append(logger.getClientInstallInfo());
    header.append(QStringLiteral("---"));

    QList<QString> logBuffer = logger.getLogBuffer();
    const int previewLines = 60;
    int start = logBuffer.size() > previewLines ? logBuffer.size() - previewLines : 0;

    QStringList lines = header;
    for (int i = start; i < logBuffer.size(); ++i) {
        lines.append(CrashReporter::redactCockatriceUrls(CrashReporter::sanitizeLogLine(logBuffer.at(i))));
    }
    return lines.join(QStringLiteral("\n"));
}

QString DlgCrashReport::writeMockReport(const QString &previewText)
{
    Logger &logger = Logger::getInstance();

    QJsonObject report;
    report[QStringLiteral("mock")] = true;
    report[QStringLiteral("timestamp")] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
    report[QStringLiteral("clientVersion")] = logger.getClientVersion();
    report[QStringLiteral("systemArchitecture")] = logger.getSystemArchitecture();
    report[QStringLiteral("systemLocale")] = logger.getSystemLocale();
    report[QStringLiteral("installInfo")] = logger.getClientInstallInfo();
    report[QStringLiteral("logPreview")] = previewText;

    QString fileName = QStringLiteral("crash-report-mock-%1.json")
                           .arg(QDateTime::currentDateTimeUtc().toString(QStringLiteral("yyyyMMdd-HHmmss")));
    QString filePath = QDir::tempPath() + QStringLiteral("/") + fileName;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return QString();
    }
    file.write(QJsonDocument(report).toJson());
    file.close();
    return filePath;
}

void DlgCrashReport::actSend()
{
    if (rememberBox->isChecked()) {
        SettingsCache::instance().debug().setCrashReportingMode(DebugSettings::CrashReportingMode::AlwaysSend);
    }
    const QString preview = previewArea->toPlainText();
    if (realSending && reporter != nullptr) {
        reporter->giveConsent();
        if (reporter->sendReport(preview)) {
            accept();
            return;
        }
        qCWarning(CrashReportLog) << "Failed to send crash report";
        // Outcome mapping for the failure box: Save = store locally and close
        // as accepted (the follow-up info box shows the path, so the user
        // knows it was NOT sent); Discard = revoke consent and close as
        // rejected (dropped); Cancel/X = back to the dialog, neither sent nor
        // dropped. The Remember checkbox governs only the original
        // Send/Don't-send decision, so the Discard path below revokes and
        // closes via QDialog::reject() without touching the setting (calling
        // our reject() override here would wrongly persist NeverSend over the
        // AlwaysSend stored above when Remember was checked).
        QMessageBox failureBox(QMessageBox::Warning, tr("Send crash report"),
                               tr("Sending the crash report failed. Save the report locally instead?"),
                               QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, this);
        failureBox.setDefaultButton(QMessageBox::Save);
        failureBox.setEscapeButton(QMessageBox::Cancel);
        const QMessageBox::StandardButton choice = static_cast<QMessageBox::StandardButton>(failureBox.exec());
        if (choice == QMessageBox::Save) {
            const QString savedPath = writeMockReport(preview);
            if (savedPath.isEmpty()) {
                QMessageBox::warning(this, tr("Send crash report"), tr("Could not save the crash report locally."));
            } else {
                QMessageBox::information(this, tr("Send crash report"),
                                         tr("Crash report saved locally to:\n%1").arg(savedPath));
            }
            accept();
            return;
        }
        if (choice == QMessageBox::Discard) {
            reporter->revokeConsent();
            QDialog::reject();
            return;
        }
        return;
    }
    writeMockReport(preview);
    accept();
}

void DlgCrashReport::actDontSend()
{
    reject();
}

void DlgCrashReport::reject()
{
    if (rememberBox->isChecked()) {
        SettingsCache::instance().debug().setCrashReportingMode(DebugSettings::CrashReportingMode::NeverSend);
    }
    if (realSending && reporter != nullptr) {
        reporter->revokeConsent();
    }
    QDialog::reject();
}
