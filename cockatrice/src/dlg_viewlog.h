#ifndef DLG_VIEWLOG_H
#define DLG_VIEWLOG_H

#include <QDialog>

class QPlainTextEdit;
class QCloseEvent;

class DlgViewLog : public QDialog {
Q_OBJECT
public:
    DlgViewLog(QWidget *parent);
protected:
    void closeEvent(QCloseEvent *event);
private:
	QPlainTextEdit *logArea;

	void loadInitialLogBuffer();
private slots:
	void logEntryAdded(QString message);
};

#endif
