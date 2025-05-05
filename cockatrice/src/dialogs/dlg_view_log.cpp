#include "dlg_view_log.h"

#include "../settings/cache_settings.h"
#include "../utility/logger.h"

#include <QClipboard>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QVBoxLayout>

DlgViewLog::DlgViewLog(QWidget *parent) : QDialog(parent)
{
    logArea = new QPlainTextEdit;
    logArea->setReadOnly(true);

    auto *mainLayout = new QVBoxLayout;
    mainLayout->setSpacing(3);
    mainLayout->setContentsMargins(20, 20, 20, 6);

    mainLayout->addWidget(logArea);

    auto *bottomLayout = new QHBoxLayout;

    coClearLog = new QCheckBox;
    coClearLog->setText(tr("Clear log when closing"));
    coClearLog->setChecked(SettingsCache::instance().servers().getClearDebugLogStatus(false));
    connect(coClearLog, &QCheckBox::toggled, this, &DlgViewLog::actCheckBoxChanged);

    copyToClipboardButton = new QPushButton;
    copyToClipboardButton->setText(tr("Copy to clipboard"));
    copyToClipboardButton->setAutoDefault(false);
    connect(copyToClipboardButton, &QPushButton::clicked, this, &DlgViewLog::actCopyToClipboard);

    bottomLayout->addWidget(coClearLog);
    bottomLayout->addStretch();
    bottomLayout->addWidget(copyToClipboardButton);

    mainLayout->addLayout(bottomLayout);

    setLayout(mainLayout);

    setWindowTitle(tr("Debug Log"));
    resize(800, 500);

    loadInitialLogBuffer();
    connect(&Logger::getInstance(), &Logger::logEntryAdded, this, &DlgViewLog::appendLogEntry);
}

void DlgViewLog::actCheckBoxChanged(bool abNewValue)
{
    SettingsCache::instance().servers().setClearDebugLogStatus(abNewValue);
}

void DlgViewLog::actCopyToClipboard()
{
    QApplication::clipboard()->setText(logArea->toPlainText());
}

void DlgViewLog::loadInitialLogBuffer()
{
    QList<QString> logBuffer = Logger::getInstance().getLogBuffer();
    for (const QString &message : logBuffer)
        appendLogEntry(message);
}

void DlgViewLog::appendLogEntry(const QString &message)
{
    static auto colorEscapeCodePattern = QRegularExpression("\033\\[\\d+m");

    QString sanitizedMessage = message;
    sanitizedMessage.replace(colorEscapeCodePattern, "");

    logArea->appendPlainText(sanitizedMessage);
}

void DlgViewLog::closeEvent(QCloseEvent * /* event */)
{
    if (coClearLog->isChecked()) {
        logArea->clear();

        logArea->appendPlainText(Logger::getInstance().getClientVersion());
        logArea->appendPlainText(Logger::getInstance().getSystemArchitecture());
    }
}
