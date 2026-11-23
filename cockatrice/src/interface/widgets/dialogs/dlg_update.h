/**
 * @file dlg_update.h
 * @ingroup ClientUpdateDialogs
 */
//! \todo Document this file.

#ifndef DLG_UPDATE_H
#define DLG_UPDATE_H

#include "../../../client/network/update/client/release_channel.h"

#include <QDialog>
#include <QLoggingCategory>
#include <QString>
#include <QUrl>
#include <QtGlobal>
#include <qtmetamacros.h>

class QDialogButtonBox;
class QLabel;
class QProgressBar;
class QPushButton;
class QTextBrowser;
class QWidget;
class UpdateDownloader;

inline Q_LOGGING_CATEGORY(DlgUpdateLog, "dlg_update");

class DlgUpdate : public QDialog
{
    Q_OBJECT
public:
    /**
     * @param parent Window that owns the dialog and receives the installer hand-off.
     * @param knownRelease A release the startup check already fetched; skips the second request.
     * @param knownCompatible Whether that release has an automatic download for this platform.
     */
    explicit DlgUpdate(QWidget *parent, Release *knownRelease = nullptr, bool knownCompatible = false);

    void done(int result) override;

signals:
    /** Outcome of a live check, for the window's update glyph. */
    void checkFinished(bool needToUpdate, bool isCompatible, Release *release);
    /** @p release's notes are on screen, so the window can clear its unread mark. */
    void notesShown(Release *release);

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    void finishedUpdateCheck(bool needToUpdate, bool isCompatible, Release *release);
    void updateCheckError(const QString &errorString);
    void startDownload();
    void cancelDownload();
    void downloadProgressMade(qint64 bytesRead, qint64 totalBytes);
    void downloadError(const QString &errorString);
    void downloadSuccessful(const QUrl &filepath);
    void gotoDownloadPage();

private:
    void beginUpdateCheck();
    void showRelease(Release *release, bool needToUpdate, bool isCompatible);
    void setStatus(const QString &text);
    void setDownloading(bool downloading);
    void warnInstallerIsWaiting();
    void markNotesShown();
    [[nodiscard]] static QString localizedChannelName();

    Release *release = nullptr;
    bool compatible = false;
    bool downloading = false;
    /** Whether the dialog is currently presenting an offered update rather than a check. */
    bool updateOffered = false;
    /** Release whose notes this dialog already reported; empty until the first report. */
    QString shownNotesRelease;

    QLabel *titleLabel, *versionLabel, *statusLabel, *headerIcon;
    QTextBrowser *notesView;
    QProgressBar *progress;
    QPushButton *actionButton, *closeButton, *stopDownload;
    QDialogButtonBox *buttonBox;
    UpdateDownloader *uDownloader;
};

#endif
