/**
 * @file dlg_view_log.h
 * @ingroup ServerLogDialogs
 */
//! \todo Document this file.

#ifndef DLG_VIEWLOG_H
#define DLG_VIEWLOG_H

#include <QDialog>
#include <QString>
#include <qtmetamacros.h>

class QPlainTextEdit;
class QCheckBox;
class QPushButton;
class QWidget;

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
    QPushButton *copyToClipboardButton;

    void loadInitialLogBuffer();
private slots:
    void appendLogEntry(const QString &message);
    void actCheckBoxChanged(bool abNewValue);
    void actCopyToClipboard();
};

#endif