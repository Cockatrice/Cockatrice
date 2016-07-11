#include "dlg_viewlog.h"
#include "logger.h"

#include <QVBoxLayout>
#include <QPlainTextEdit>

DlgViewLog::DlgViewLog(QWidget *parent)
: QDialog(parent)
{
    logArea = new QPlainTextEdit;
    logArea->setReadOnly(true);
    
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(logArea);

    setLayout(mainLayout);

    setWindowTitle(tr("Debug Log"));
    resize(800, 500);

    loadInitialLogBuffer();
    connect(&Logger::getInstance(), SIGNAL(logEntryAdded(QString)), this, SLOT(logEntryAdded(QString)));    
}

void DlgViewLog::loadInitialLogBuffer()
{
    QVector<QString> logBuffer = Logger::getInstance().getLogBuffer();
    foreach(QString message, logBuffer)
        logEntryAdded(message);
}

void DlgViewLog::logEntryAdded(QString message)
{
    logArea->appendPlainText(message);
}
