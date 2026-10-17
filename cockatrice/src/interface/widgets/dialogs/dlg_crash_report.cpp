#include "dlg_crash_report.h"

#include "../../../client/crash/crash_reporter.h"
#include "../../../client/settings/cache_settings.h"
#include "../../logger.h"

#include <QCheckBox>
#include <QDateTime>
#include <QDebug>
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
#include <libcockatrice/settings/personal_settings.h>

DlgCrashReport::DlgCrashReport(QWidget *parent, bool _realSending, CrashReporter *_reporter)
    : QDialog(parent), realSending(_realSending), reporter(_reporter)
{
    infoLabel = new QLabel(this);
    infoLabel->setWordWrap(true);

    accountLabel = new QLabel(this);
    accountLabel->setWordWrap(true);

    // Constructed before previewArea: Qt derives the default Tab chain from construction
    // order, and note sits visually above the log excerpt, so this keeps Tab moving downward.
    noteArea = new QPlainTextEdit(this);
    // Optional free text ("what I was doing when it crashed") — a few lines are
    // enough for a description, leave the rest of the space to the log excerpt.
    noteArea->setMaximumHeight(70);

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
    mainLayout->addWidget(noteArea);
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
        // The preview below prefers the crashed session's persisted log tail
        // (Logger::getPreviousSessionTail), falling back to this restart's in-memory buffer only
        // when no previous session was saved. On send it is attached as crash-log.txt to the
        // pending native crash event (dump + stack + log + note in one report); when no native
        // crash event exists, it rides the fallback message event instead.
        infoLabel->setText(tr("Oops, we crashed — and now we need your help to make Cockatrice better!\n\n"
                              "The good thing is, helping is easy and convenient. Sending uploads what the "
                              "previous run left behind when it crashed: the crash dump with the stack trace, "
                              "the app version and operating system info, the redacted log excerpt shown below, "
                              "and anything you write in the note box. If no log from the crashed session was "
                              "saved, this restart's early logs are shown instead.\n\n"
                              "Reports are what let us fix crashes like this one. Check \u201cRemember my choice\u201d "
                              "and Send, and future crash reports are sent automatically without interruption."));
    } else {
        infoLabel->setText(tr("(Mock preview — nothing will be uploaded.)\n\n"
                              "Cockatrice closed unexpectedly. A real report would contain what the previous run "
                              "left behind when it crashed: the crash dump with the stack trace, the app version "
                              "and operating system info, the redacted log excerpt shown below, and anything you "
                              "write in the note box. If no log from the crashed session was saved, this restart's "
                              "early logs are shown instead.\n\n"
                              "A mock file is written locally for inspection."));
    }
    accountLabel->setText(tr("No account needed, and no contact info is collected. Login links "
                             "(cockatrice://...) are scrubbed before sending."));
    noteArea->setPlaceholderText(tr("(Optional) Describe what you were doing when it crashed"));
    // Direction-neutral on purpose: the same box also remembers a "Don't send" decision
    // (reject() persists NeverSend when it is checked), so it must not promise sending.
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

    QList<QString> logLines = logger.getPreviousSessionTail();
    if (logLines.isEmpty()) {
        logLines = logger.getLogBuffer();
    }
    const int previewLines = 60;
    int start = logLines.size() > previewLines ? logLines.size() - previewLines : 0;

    QStringList lines = header;
    for (int i = start; i < logLines.size(); ++i) {
        lines.append(CrashReporter::redactCockatriceUrls(CrashReporter::sanitizeLogLine(logLines.at(i))));
    }
    return lines.join(QStringLiteral("\n"));
}

QString DlgCrashReport::writeMockReport(const QString &previewText, const QString &userNote)
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
    report[QStringLiteral("userNote")] = userNote;

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
        SettingsCache::instance().personal().setCrashReportingMode(PersonalSettings::CrashReportingMode::AlwaysSend);
    }
    const QString preview = previewArea->toPlainText();
    const QString userNote = noteArea->toPlainText().trimmed();
    if (realSending && reporter != nullptr) {
        // Fold log and note into the pending native crash event(s) BEFORE consent:
        // giveConsent() releases the held envelopes for upload, so dump, stack,
        // log and note arrive as one Sentry event. Without a pending crash event
        // (unclean exit that produced no native crash) this returns 0 and
        // sendReport() falls back to a standalone message event with the same
        // attachments.
        const int attached = reporter->attachToPendingCrashEnvelopes(preview, userNote);
        reporter->giveConsent();
        if (attached > 0 || reporter->sendReport(preview, userNote)) {
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
            const QString savedPath = writeMockReport(preview, userNote);
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
    const QString savedPath = writeMockReport(preview, userNote);
    if (savedPath.isEmpty()) {
        qCWarning(CrashReportLog) << "Could not save the mock crash report locally";
    } else {
        qCInfo(CrashReportLog) << "Mock crash report saved locally to" << savedPath;
    }
    accept();
}

void DlgCrashReport::actDontSend()
{
    reject();
}

void DlgCrashReport::reject()
{
    if (rememberBox->isChecked()) {
        SettingsCache::instance().personal().setCrashReportingMode(PersonalSettings::CrashReportingMode::NeverSend);
    }
    if (realSending && reporter != nullptr) {
        reporter->revokeConsent();
    }
    QDialog::reject();
}
