#include "tab_debuglog.h"
#include "logger.h"

#include <QVBoxLayout>
#include <QPlainTextEdit>

TabDebugLog::TabDebugLog(TabSupervisor *tabSupervisor, QWidget *parent)
: Tab(tabSupervisor, parent)
{
    logArea = new QPlainTextEdit;
    logArea->setReadOnly(true);
    
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(logArea);

    QWidget *mainWidget = new QWidget(this);
    mainWidget->setLayout(mainLayout);
    setCentralWidget(mainWidget);

    loadInitialLogBuffer();
    connect(&Logger::getInstance(), SIGNAL(logEntryAdded(QString)), this, SLOT(logEntryAdded(QString)));    
}

TabDebugLog::~TabDebugLog()
{
    emit debugLogClosing();
}

void TabDebugLog::closeRequest()
{
    deleteLater();
}

void TabDebugLog::retranslateUi()
{
}

void TabDebugLog::loadInitialLogBuffer()
{
    QVector<QString> logBuffer = Logger::getInstance().getLogBuffer();
    foreach(QString message, logBuffer)
        logEntryAdded(message);
}

void TabDebugLog::logEntryAdded(QString message)
{
    logArea->appendPlainText(message);
}
