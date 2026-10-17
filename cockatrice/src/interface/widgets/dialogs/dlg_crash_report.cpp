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
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <libcockatrice/settings/personal_settings.h>

DlgCrashReport::DlgCrashReport(QWidget *parent) : QDialog(parent)
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
    infoLabel->setText(
        tr("Cockatrice closed unexpectedly. The preview below shows what a crash report would contain (mock)."));
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
        SettingsCache::instance().personal().setCrashReportingMode(PersonalSettings::CrashReportingMode::AlwaysSend);
    }
    const QString savedPath = writeMockReport(previewArea->toPlainText());
    if (savedPath.isEmpty()) {
        qWarning() << "Could not save the mock crash report locally";
    } else {
        qInfo() << "Mock crash report saved locally to" << savedPath;
    }
    accept();
}

void DlgCrashReport::actDontSend()
{
    if (rememberBox->isChecked()) {
        SettingsCache::instance().personal().setCrashReportingMode(PersonalSettings::CrashReportingMode::NeverSend);
    }
    reject();
}
