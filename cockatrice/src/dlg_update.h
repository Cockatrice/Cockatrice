#ifndef DLG_UPDATE_H
#define DLG_UPDATE_H

#include <QtNetwork>
#include <QProgressDialog>
#include <QDialogButtonBox>

#include "update_downloader.h"
class Release;

class DlgUpdate : public QDialog {
Q_OBJECT
public:
    DlgUpdate(QWidget *parent);

private slots:
    void finishedUpdateCheck(bool needToUpdate, bool isCompatible, Release *release);
    void gotoDownloadPage();
    void downloadUpdate();
    void cancelDownload();
    void updateCheckError(QString errorString);
    void downloadSuccessful(QUrl filepath);
    void downloadProgressMade(qint64 bytesRead, qint64 totalBytes);
    void downloadError(QString errorString);
    void closeDialog();
private:
    QUrl updateUrl;
    void enableUpdateButton(bool enable);
    void enableOkButton(bool enable);
    void addStopDownloadAndRemoveOthers(bool enable);
    void beginUpdateCheck();
    void setLabel(QString text);
    QLabel *statusLabel, *descriptionLabel;
    QProgressBar *progress;
    QPushButton *manualDownload, *gotoDownload, *ok, *stopDownload;
    QPushButton *cancel;
    UpdateDownloader *uDownloader;
    QDialogButtonBox *buttonBox;
};

#endif
