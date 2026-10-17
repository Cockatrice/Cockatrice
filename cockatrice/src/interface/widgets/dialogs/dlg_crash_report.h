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

class CrashReporter;

/**
 * @brief Crash-report dialog: mock fallback or real Sentry send.
 *
 * Mock mode (default): the Send path writes a redacted JSON file to the temp
 * dir instead of uploading anything. Real mode (realSending, with a live
 * CrashReporter): Send first folds the redacted log and the optional user note
 * into the pending native crash event(s), then gives consent so dump, stack,
 * log and note upload as one Sentry event; when no pending crash event exists
 * it falls back to a standalone message event with the same attachments.
 * Failure offers a local save instead.
 */
class DlgCrashReport : public QDialog
{
    Q_OBJECT
public:
    explicit DlgCrashReport(QWidget *parent = nullptr, bool realSending = false, CrashReporter *_reporter = nullptr);

    static QString buildRedactedPreview();
    static QString writeMockReport(const QString &previewText, const QString &userNote = QString());

    void reject() override;

protected:
    void changeEvent(QEvent *event) override;

private:
    QLabel *infoLabel;
    QLabel *accountLabel;
    QPlainTextEdit *previewArea;
    QPlainTextEdit *noteArea;
    QCheckBox *rememberBox;
    QPushButton *sendButton;
    QPushButton *dontSendButton;
    bool realSending = false;
    CrashReporter *reporter = nullptr;

    void retranslateUi();

private slots:
    void actSend();
    void actDontSend();
};

#endif
