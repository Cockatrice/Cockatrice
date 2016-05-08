#ifndef DLG_UPDATE_H
#define DLG_UPDATE_H

#include <QtNetwork>
#include <QProgressDialog>

#include "update_checker.h"
#include "update_downloader.h"

class DlgUpdate : public QDialog {
Q_OBJECT
public:
    DlgUpdate(QWidget *parent);

private slots:
    void finishedUpdateCheck(bool needToUpdate, bool isCompatible, QVariantMap *build);
    void gotoDownloadPage();
    void downloadUpdate();
    void updateCheckError(QString errorString);
    void downloadSuccessful(QUrl filepath);
    void downloadProgressMade(qint64 bytesRead, qint64 totalBytes);
    void downloadError(QString errorString);
    void closeDialog();
private:
    QUrl updateUrl;
    void enableUpdateButton(bool enable);
    void enableOkButton(bool enable);
    void beginUpdateCheck();
    void setLabel(QString text);
    QLabel *text;
    QProgressBar *progress;
    QPushButton *manualDownload, *gotoDownload, *ok;
    QPushButton *cancel;
    UpdateChecker *uChecker;
    UpdateDownloader *uDownloader;
};

#endif
