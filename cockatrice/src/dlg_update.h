#ifndef DLG_UPDATE_H
#define DLG_UPDATE_H

#include "update_downloader.h"

#include <QDialogButtonBox>
#include <QProgressDialog>
#include <QtNetwork>
class Release;

class DlgUpdate : public QDialog
{
    Q_OBJECT
public:
    DlgUpdate(QWidget *parent);

private slots:
    void finishedUpdateCheck(bool needToUpdate, bool isCompatible, Release *release);
    void gotoDownloadPage();
    void downloadUpdate();
    void cancelDownload();
    void updateCheckError(const QString &errorString);
    void downloadSuccessful(const QUrl &filepath);
    void downloadProgressMade(qint64 bytesRead, qint64 totalBytes);
    void downloadError(const QString &errorString);
    void closeDialog();

private:
    QUrl updateUrl;
    void enableUpdateButton(bool enable);
    void enableOkButton(bool enable);
    void addStopDownloadAndRemoveOthers(bool enable);
    void beginUpdateCheck();
    void setLabel(const QString &text);
    QLabel *statusLabel, *descriptionLabel;
    QProgressBar *progress;
    QPushButton *manualDownload, *gotoDownload, *ok, *stopDownload;
    QPushButton *cancel;
    UpdateDownloader *uDownloader;
    QDialogButtonBox *buttonBox;
};

#endif
