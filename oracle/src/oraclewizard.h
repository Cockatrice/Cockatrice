#ifndef ORACLEWIZARD_H
#define ORACLEWIZARD_H

#include <QFuture>
#include <QFutureWatcher>
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

class OracleWizard : public QWizard
{
    Q_OBJECT
public:
    explicit OracleWizard(QWidget *parent = nullptr);
    void accept() override;
    void enableButtons();
    void disableButtons();
    void retranslateUi();
    void setTokensData(QByteArray _tokensData)
    {
        tokensData = std::move(_tokensData);
    }
    bool hasTokensData()
    {
        return !tokensData.isEmpty();
    }
    bool saveTokensToFile(const QString &fileName);

public:
    OracleImporter *importer;
    QSettings *settings;

private slots:
    void updateLanguage();

private:
    QByteArray tokensData;

protected:
    void changeEvent(QEvent *event) override;
};

class OracleWizardPage : public QWizardPage
{
    Q_OBJECT
public:
    explicit OracleWizardPage(QWidget *parent = nullptr) : QWizardPage(parent){};
    virtual void retranslateUi() = 0;

protected:
    inline OracleWizard *wizard()
    {
        return (OracleWizard *)QWizardPage::wizard();
    };
};

class IntroPage : public OracleWizardPage
{
    Q_OBJECT
public:
    explicit IntroPage(QWidget *parent = nullptr);
    void retranslateUi() override;

private:
    QStringList findQmFiles();
    QString languageName(const QString &qmFile);

private:
    QLabel *label, *languageLabel, *versionLabel;
    QComboBox *languageBox;

private slots:
    void languageBoxChanged(int index);
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
    void readSetsFromByteArray(QByteArray data);
    void downloadSetsFile(QUrl url);

private:
    QRadioButton *urlRadioButton;
    QRadioButton *fileRadioButton;
    QLineEdit *urlLineEdit;
    QLineEdit *fileLineEdit;
    QPushButton *urlButton;
    QPushButton *fileButton;
    QLabel *progressLabel;
    QProgressBar *progressBar;

    QNetworkAccessManager *nam;
    QFutureWatcher<bool> watcher;
    QFuture<bool> future;

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

protected:
    void initializePage() override;
    void cleanupPage() override;
    bool validatePage() override;

private slots:
    void updateTotalProgress(int cardsImported, int setIndex, const QString &setName);
};

class LoadSpoilersPage : public OracleWizardPage
{
    Q_OBJECT
public:
    explicit LoadSpoilersPage(QWidget *parent = nullptr);
    void retranslateUi() override;

private:
    QLabel *urlLabel;
    QLineEdit *urlLineEdit;
    QPushButton *urlButton;
    QLabel *progressLabel;
    QProgressBar *progressBar;
    QNetworkAccessManager *nam;

private slots:
    void actRestoreDefaultUrl();
    void actDownloadProgressSpoilersFile(qint64 received, qint64 total);
    void actDownloadFinishedSpoilersFile();

protected:
    void initializePage() override;
    bool validatePage() override;
    void downloadSpoilersFile(QUrl url);
};

class SaveSpoilersPage : public OracleWizardPage
{
    Q_OBJECT
public:
    explicit SaveSpoilersPage(QWidget *parent = nullptr);
    void retranslateUi() override;

private:
    QCheckBox *defaultPathCheckBox;

protected:
    bool validatePage() override;
};

class LoadTokensPage : public OracleWizardPage
{
    Q_OBJECT
public:
    explicit LoadTokensPage(QWidget *parent = nullptr);
    void retranslateUi() override;

protected:
    void initializePage() override;
    bool validatePage() override;
    void downloadTokensFile(QUrl url);

private:
    QLabel *urlLabel;
    QLineEdit *urlLineEdit;
    QPushButton *urlButton;
    QLabel *progressLabel;
    QProgressBar *progressBar;
    QNetworkAccessManager *nam;

private slots:
    void actRestoreDefaultUrl();
    void actDownloadProgressTokensFile(qint64 received, qint64 total);
    void actDownloadFinishedTokensFile();
};

class SaveTokensPage : public OracleWizardPage
{
    Q_OBJECT
public:
    explicit SaveTokensPage(QWidget *parent = nullptr);
    void retranslateUi() override;

private:
    QCheckBox *defaultPathCheckBox;

protected:
    bool validatePage() override;
};

#endif