#ifndef DLG_VIEWLOG_H
#define DLG_VIEWLOG_H

#include <QDialog>
#include <QCheckBox>

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
    QCheckBox *coClearLog;

    void loadInitialLogBuffer();
    void actCheckBoxChanged(bool abNewValue);
private slots:
    void logEntryAdded(QString message);
};

#endif