/**
 * @file dlg_crash_report.h
 * @ingroup Dialogs
 */

#ifndef DLG_CRASH_REPORT_H
#define DLG_CRASH_REPORT_H

#include <QDialog>
#include <QString>
#include <qtmetamacros.h>

class QCheckBox;
class QLabel;
class QPlainTextEdit;
class QPushButton;

/**
 * @brief MOCK crash-report dialog (UI + settings only, no network).
 *
 * The Send path writes a redacted JSON file to the temp dir instead of
 * uploading anything. Real Sentry upload comes in a later phase.
 */
class DlgCrashReport : public QDialog
{
    Q_OBJECT
public:
    explicit DlgCrashReport(QWidget *parent = nullptr);

    static QString buildRedactedPreview();
    static QString writeMockReport(const QString &previewText);

protected:
    void changeEvent(QEvent *event) override;

private:
    QLabel *infoLabel;
    QLabel *accountLabel;
    QPlainTextEdit *previewArea;
    QCheckBox *rememberBox;
    QPushButton *sendButton;
    QPushButton *dontSendButton;

    void retranslateUi();

private slots:
    void actSend();
    void actDontSend();
};

#endif
