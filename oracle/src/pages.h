#ifndef COCKATRICE_PAGES_H
#define COCKATRICE_PAGES_H

#include "pagetemplates.h"

#include <QByteArray>
#include <QFuture>
#include <QFutureWatcher>
#include <QSharedPointer>
#include <QString>
#include <QStringList>
#include <QtGlobal>
#include <qtmetamacros.h>

class QCheckBox;
class QNetworkReply;
class OracleSourceData;
class QComboBox;
class QLabel;
class QLineEdit;
class QRadioButton;
class QProgressBar;
class QTextEdit;
class QPushButton;
class QUrl;
class QWidget;

class IntroPage : public OracleWizardPage
{
    Q_OBJECT
public:
    explicit IntroPage(QWidget *parent = nullptr);
    void retranslateUi() override;

private:
    QStringList findQmFiles();

private:
    QLabel *label, *languageLabel, *versionLabel;
    QComboBox *languageBox;

private slots:
    void languageBoxChanged(int index);

protected slots:
    void initializePage() override;
};

class OutroPage : public OracleWizardPage
{
    Q_OBJECT
public:
    explicit OutroPage(QWidget * = nullptr)
    {
    }
    void retranslateUi() override;

protected:
    void initializePage() override;
};

/** @brief Result of a worker-thread sets-file load (read + decompress + dispatch). */
struct LoadSetsResult
{
    bool ok = false;                            ///< JSON scan produced set data (or plain XML was handled)
    bool plainXml = false;                      ///< input was a plain Cockatrice XML database
    QSharedPointer<OracleSourceData> xmlSource; ///< source for the plain-XML path
    QString errorMessage;                       ///< set when the input could not be processed
    bool offerUncompressedFallback = false;     ///< decompression-only failure: offer the uncompressed URL
};

class LoadSetsPage : public OracleWizardPage
{
    Q_OBJECT
public:
    explicit LoadSetsPage(QWidget *parent = nullptr);
    void retranslateUi() override;
    bool isComplete() const override;

protected:
    void initializePage() override;
    bool validatePage() override;
    void readSetsFromSource(QSharedPointer<OracleSourceData> source);
    void readSetsFromFile(const QString &fileName);
    void downloadSetsFile(const QUrl &url);
    void cancelWork() override;

private:
    QRadioButton *urlRadioButton;
    QRadioButton *fileRadioButton;
    QLineEdit *urlLineEdit;
    QLineEdit *fileLineEdit;
    QPushButton *urlButton;
    QPushButton *fileButton;
    QLabel *progressLabel;
    QProgressBar *progressBar;

    QFutureWatcher<LoadSetsResult> watcher;
    QFuture<LoadSetsResult> future;
    bool loadActive = false;

    void beginLoadSets(bool compressedFile = false);

private slots:
    void actLoadSetsFile();
    void actRestoreDefaultUrl();
    void actDownloadProgressSetsFile(qint64 received, qint64 total);
    void actDownloadFinishedSetsFile(QNetworkReply *reply,
                                     QSharedPointer<OracleSourceData> source,
                                     const QString &writeError);
    void updateParsingProgress(qint64 bytesRead, qint64 totalBytes);
    void scanProgressToStdout(qint64 bytesRead, qint64 totalBytes);
    void importFinished();
    void zipDownloadFailed(const QString &message);
};

class SaveSetsPage : public OracleWizardPage
{
    Q_OBJECT
public:
    explicit SaveSetsPage(QWidget *parent = nullptr);
    void retranslateUi() override;
    bool isComplete() const override;

private:
    QTextEdit *messageLog;
    QProgressBar *progressBar;
    QCheckBox *defaultPathCheckBox;
    QLabel *pathLabel;
    QLabel *saveLabel;

    QFutureWatcher<int> importWatcher;
    QFuture<int> importFuture;
    int totalSets = 0;
    bool importActive = false;
    bool importFailed = false;

protected:
    void initializePage() override;
    void cleanupPage() override;
    bool validatePage() override;
    void cancelWork() override;

private slots:
    void importFinished();
    void updateTotalProgress(int cardsImported, int setIndex, const QString &setName);
};

class LoadSpoilersPage : public SimpleDownloadFilePage
{
    Q_OBJECT
public:
    explicit LoadSpoilersPage(QWidget * = nullptr)
    {
    }
    void retranslateUi() override;

protected:
    QString getDefaultUrl() override;
    QString getCustomUrlSettingsKey() override;
    QString getDefaultSavePath() override;
    QString getWindowTitle() override;
    QString getFileType() override;
    QString getFilePromptName() override;
};

class LoadTokensPage : public SimpleDownloadFilePage
{
    Q_OBJECT
public:
    explicit LoadTokensPage(QWidget * = nullptr)
    {
    }
    void retranslateUi() override;

protected:
    QString getDefaultUrl() override;
    QString getCustomUrlSettingsKey() override;
    QString getDefaultSavePath() override;
    QString getWindowTitle() override;
    QString getFileType() override;
    QString getFilePromptName() override;
    void initializePage() override;
};

#endif // COCKATRICE_PAGES_H
