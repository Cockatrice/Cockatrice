#ifndef DLG_VIEWLOG_H
#define DLG_VIEWLOG_H

#include <QCheckBox>
#include <QDialog>

class QPlainTextEdit;
class QCloseEvent;

class DlgViewLog : public QDialog
{
    Q_OBJECT
public:
    explicit DlgViewLog(QWidget *parent);

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    QPlainTextEdit *logArea;
    QCheckBox *coClearLog;

    void loadInitialLogBuffer();
private slots:
    void appendLogEntry(const QString &message);
    void actCheckBoxChanged(bool abNewValue);
};

#endif