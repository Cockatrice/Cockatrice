#ifndef COCKATRICE_PAGES_H
#define COCKATRICE_PAGES_H

#include "pagetemplates.h"

#include <QByteArray>
#include <QFuture>
#include <QFutureWatcher>
#include <QString>
#include <QTimer>
#include <QWizard>
#include <utility>

class QCheckBox;
class QGroupBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QRadioButton;
class QProgressBar;
class QNetworkAccessManager;
class QTextEdit;
class QVBoxLayout;
class OracleImporter;
class QSettings;

class IntroPage : public OracleWizardPage
{
    Q_OBJECT
public:
    explicit IntroPage(QWidget *parent = nullptr);
    void retranslateUi() override;

private:
    QStringList findQmFiles();
    QString languageName(const QString &lang);

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
    bool ok = false;                        ///< JSON scan produced set data (or plain XML was handled)
    bool plainXml = false;                  ///< input was a plain Cockatrice XML database
    QByteArray xmlData;                     ///< raw XML for the plain-XML path
    QString errorMessage;                   ///< set when the input could not be processed
    bool offerUncompressedFallback = false; ///< decompression-only failure: offer the uncompressed URL
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
    void readSetsFromByteArray(QByteArray _data);
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
    void actDownloadFinishedSetsFile();
    void updateParsingProgress(int bytesRead, int totalBytes);
    void scanProgressToStdout(int bytesRead, int totalBytes);
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
