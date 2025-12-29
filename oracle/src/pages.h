#ifndef COCKATRICE_PAGES_H
#define COCKATRICE_PAGES_H

#include "pagetemplates.h"

#include <QFuture>
#include <QFutureWatcher>
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

class LoadSetsPage : public OracleWizardPage
{
    Q_OBJECT
public:
    explicit LoadSetsPage(QWidget *parent = nullptr);
    void retranslateUi() override;

protected:
    void initializePage() override;
    bool validatePage() override;
    void readSetsFromByteArray(QByteArray _data);
    void readSetsFromByteArrayRef(QByteArray &_data);
    void downloadSetsFile(const QUrl &url);

private:
    QRadioButton *urlRadioButton;
    QRadioButton *fileRadioButton;
    QLineEdit *urlLineEdit;
    QLineEdit *fileLineEdit;
    QPushButton *urlButton;
    QPushButton *fileButton;
    QLabel *progressLabel;
    QProgressBar *progressBar;

    QFutureWatcher<bool> watcher;
    QFuture<bool> future;
    QByteArray jsonData;

private slots:
    void actLoadSetsFile();
    void actRestoreDefaultUrl();
    void actDownloadProgressSetsFile(qint64 received, qint64 total);
    void actDownloadFinishedSetsFile();
    void importFinished();
    void zipDownloadFailed(const QString &message);
};

class SaveSetsPage : public OracleWizardPage
{
    Q_OBJECT
public:
    explicit SaveSetsPage(QWidget *parent = nullptr);
    void retranslateUi() override;

private:
    QTextEdit *messageLog;
    QCheckBox *defaultPathCheckBox;
    QLabel *pathLabel;
    QLabel *saveLabel;

protected:
    void initializePage() override;
    void cleanupPage() override;
    bool validatePage() override;

private slots:
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
