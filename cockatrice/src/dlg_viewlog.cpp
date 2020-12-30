#include "dlg_viewlog.h"

#include "logger.h"
#include "settingscache.h"

#include <QPlainTextEdit>
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
    connect(coClearLog, SIGNAL(toggled(bool)), this, SLOT(actCheckBoxChanged(bool)));
    mainLayout->addWidget(coClearLog);

    setLayout(mainLayout);

    setWindowTitle(tr("Debug Log"));
    resize(800, 500);

    loadInitialLogBuffer();
    connect(&Logger::getInstance(), SIGNAL(logEntryAdded(QString)), this, SLOT(logEntryAdded(QString)));
}

void DlgViewLog::actCheckBoxChanged(bool abNewValue)
{
    SettingsCache::instance().servers().setClearDebugLogStatus(abNewValue);
}

void DlgViewLog::loadInitialLogBuffer()
{
    QList<QString> logBuffer = Logger::getInstance().getLogBuffer();
    foreach (QString message, logBuffer)
        logEntryAdded(message);
}

void DlgViewLog::logEntryAdded(QString message)
{
    logArea->appendPlainText(message);
}

void DlgViewLog::closeEvent(QCloseEvent * /* event */)
{
    if (coClearLog->isChecked()) {
        logArea->clear();

        logArea->appendPlainText(Logger::getInstance().getClientVersion());
        logArea->appendPlainText(Logger::getInstance().getSystemArchitecture());
    }
}
