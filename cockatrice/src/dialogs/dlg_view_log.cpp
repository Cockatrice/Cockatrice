#include "dlg_view_log.h"

#include "../settings/cache_settings.h"
#include "../utility/logger.h"

#include <QPlainTextEdit>
#include <QRegularExpression>
#include <QVBoxLayout>

DlgViewLog::DlgViewLog(QWidget *parent) : QDialog(parent)
{

    logArea = new QPlainTextEdit;
    logArea->setReadOnly(true);

    auto *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(logArea);

    coClearLog = new QCheckBox;
    coClearLog->setText(tr("Clear log when closing"));
    coClearLog->setChecked(SettingsCache::instance().servers().getClearDebugLogStatus(false));
    connect(coClearLog, &QCheckBox::toggled, this, &DlgViewLog::actCheckBoxChanged);
    mainLayout->addWidget(coClearLog);

    setLayout(mainLayout);

    setWindowTitle(tr("Debug Log"));
    resize(800, 500);

    loadInitialLogBuffer();
    connect(&Logger::getInstance(), &Logger::logEntryAdded, this, &DlgViewLog::logEntryAdded);
}

void DlgViewLog::actCheckBoxChanged(bool abNewValue)
{
    SettingsCache::instance().servers().setClearDebugLogStatus(abNewValue);
}

void DlgViewLog::loadInitialLogBuffer()
{
    QList<QString> logBuffer = Logger::getInstance().getLogBuffer();
    for (const QString &message : logBuffer)
        logEntryAdded(message);
}

void DlgViewLog::logEntryAdded(QString message)
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
