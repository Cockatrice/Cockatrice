#ifndef TAB_DEBUGLOG_H
#define TAB_DEBUGLOG_H

#include "tab.h"

class QPlainTextEdit;

class TabDebugLog : public Tab {
Q_OBJECT
public:
    TabDebugLog(TabSupervisor *tabSupervisor, QWidget *parent = 0);
    ~TabDebugLog();
private:
	QPlainTextEdit *logArea;

	void loadInitialLogBuffer();
public:
    void retranslateUi();
    QString getTabText() const { return tr("Debug Log"); }
private slots:
	void logEntryAdded(QString message);
public slots:
	void closeRequest();
signals:
    void debugLogClosing();
};

#endif
