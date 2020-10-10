#ifndef PAGETEMPLATES_H
#define PAGETEMPLATES_H

#include <QWizardPage>

class OracleWizard;
class QCheckBox;
class QLabel;
class QLineEdit;
class QProgressBar;

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

class SimpleDownloadFilePage : public OracleWizardPage
{
    Q_OBJECT
public:
    explicit SimpleDownloadFilePage(QWidget *parent = nullptr);

protected:
    void initializePage() override;
    bool validatePage() override;
    void downloadFile(QUrl url);
    virtual QString getDefaultUrl() = 0;
    virtual QString getCustomUrlSettingsKey() = 0;
    virtual QString getDefaultSavePath() = 0;
    virtual QString getWindowTitle() = 0;
    virtual QString getFileType() = 0;
    bool saveToFile();
    bool internalSaveToFile(const QString &fileName);

protected:
    QByteArray downloadData;
    QLabel *urlLabel;
    QLabel *pathLabel;
    QLineEdit *urlLineEdit;
    QPushButton *urlButton;
    QLabel *progressLabel;
    QProgressBar *progressBar;
    QCheckBox *defaultPathCheckBox;

signals:
    void parsedDataReady();
private slots:
    void actRestoreDefaultUrl();
    void actDownloadProgress(qint64 received, qint64 total);
    void actDownloadFinished();
};

#endif // PAGETEMPLATES_H
