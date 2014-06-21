#ifndef ORACLEWIZARD_H
#define ORACLEWIZARD_H

#include <QWizard>
#include <QFutureWatcher>
#include <QFuture>

class QCheckBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QRadioButton;
class QProgressBar;
class QNetworkAccessManager;
class QTextEdit;
class QVBoxLayout;
class OracleImporter;

class OracleWizard : public QWizard
{
     Q_OBJECT
public:
     OracleWizard(QWidget *parent = 0);
     void accept();
     void enableButtons();
     void disableButtons();
public: 
     OracleImporter *importer;
};


class OracleWizardPage : public QWizardPage
{
     Q_OBJECT
public:
     OracleWizardPage(QWidget *parent = 0): QWizardPage(parent) {};
protected:
     inline OracleWizard *wizard() { return (OracleWizard*) QWizardPage::wizard(); };
};

class IntroPage : public OracleWizardPage
{
     Q_OBJECT
public:
     IntroPage(QWidget *parent = 0);
private:
     QLabel *label;
};

class LoadSetsPage : public OracleWizardPage
{
     Q_OBJECT
public:
     LoadSetsPage(QWidget *parent = 0);
protected:
     void initializePage();
     bool validatePage();
     void readSetsFromByteArray(QByteArray data);
private:
     QRadioButton *urlRadioButton;
     QRadioButton *fileRadioButton;
     QLineEdit *urlLineEdit;
     QLineEdit *fileLineEdit;
     QPushButton *fileButton;
     QLabel *progressLabel;
     QProgressBar * progressBar;

     QNetworkAccessManager *nam;
     QFutureWatcher<bool> watcher;
     QFuture<bool> future;
private slots:
     void actLoadSetsFile();
     void actDownloadProgressSetsFile(qint64 received, qint64 total);
     void actDownloadFinishedSetsFile();
     void importFinished();
};

class ChooseSetsPage : public OracleWizardPage
{
     Q_OBJECT
public:
     ChooseSetsPage(QWidget *parent = 0);
protected:
     void initializePage();
     bool validatePage();
private:
     QPushButton *checkAllButton, *uncheckAllButton;
     QVBoxLayout *checkBoxLayout;
     QList<QCheckBox *> checkBoxList;
private slots:
     void actCheckAll();
     void actUncheckAll();
     void checkBoxChanged(int state);
};

class SaveSetsPage : public OracleWizardPage
{
     Q_OBJECT
public:
     SaveSetsPage(QWidget *parent = 0);
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

#endif