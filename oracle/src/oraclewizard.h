#ifndef ORACLEWIZARD_H
#define ORACLEWIZARD_H

#include <QWizard>
#include <QFutureWatcher>
#include <QFuture>

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
     OracleWizard(QWidget *parent = 0);
     void accept();
     void enableButtons();
     void disableButtons();
     void retranslateUi();
     void setTokensData(QByteArray _tokensData) { tokensData = _tokensData; }
     bool hasTokensData() { return !tokensData.isEmpty(); }
     bool saveTokensToFile(const QString & fileName);
public: 
     OracleImporter *importer;
     QSettings * settings;
private slots:
    void updateLanguage();
private:
    QStringList findQmFiles();
    QString languageName(const QString &qmFile);
    QByteArray tokensData;
protected:
    void changeEvent(QEvent *event);
};


class OracleWizardPage : public QWizardPage
{
     Q_OBJECT
public:
     OracleWizardPage(QWidget *parent = 0): QWizardPage(parent) {};
     virtual void retranslateUi() = 0;
protected:
     inline OracleWizard *wizard() { return (OracleWizard*) QWizardPage::wizard(); };
};

class IntroPage : public OracleWizardPage
{
     Q_OBJECT
public:
    IntroPage(QWidget *parent = 0);
    void retranslateUi();
private:
    QStringList findQmFiles();
    QString languageName(const QString &qmFile);
private:
     QLabel *label, *languageLabel;
     QComboBox *languageBox;
private slots:
    void languageBoxChanged(int index);
};

class LoadSetsPage : public OracleWizardPage
{
     Q_OBJECT
public:
     LoadSetsPage(QWidget *parent = 0);
    void retranslateUi();
protected:
     void initializePage();
     bool validatePage();
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
     QProgressBar * progressBar;

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
     SaveSetsPage(QWidget *parent = 0);
    void retranslateUi();
private:
     QTextEdit *messageLog;
     QCheckBox * defaultPathCheckBox;
protected:
     void initializePage();
     void cleanupPage();
     bool validatePage();
private slots:
     void updateTotalProgress(int cardsImported, int setIndex, const QString &setName);
};

class LoadTokensPage : public OracleWizardPage
{
     Q_OBJECT
public:
     LoadTokensPage(QWidget *parent = 0);
    void retranslateUi();
protected:
     void initializePage();
     bool validatePage();
     void downloadTokensFile(QUrl url);
private:
     QLabel *urlLabel;
     QLineEdit *urlLineEdit;
     QPushButton *urlButton;
     QLabel *progressLabel;
     QProgressBar * progressBar;

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
     SaveTokensPage(QWidget *parent = 0);
    void retranslateUi();
private:
     QCheckBox * defaultPathCheckBox;
protected:
     bool validatePage();
};
#endif