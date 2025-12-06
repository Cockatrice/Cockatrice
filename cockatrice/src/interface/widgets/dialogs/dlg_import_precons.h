#ifndef DLG_IMPORT_PRECONS_H
#define DLG_IMPORT_PRECONS_H

#include <QCheckBox>
#include <QFutureWatcher>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QNetworkAccessManager>
#include <QProgressBar>
#include <QRadioButton>
#include <QSettings>
#include <QTextEdit>
#include <QTreeWidget>
#include <QWidget>
#include <QWizardPage>

class DlgImportPrecons : public QWizard
{
    Q_OBJECT
public:
    explicit DlgImportPrecons(QWidget *parent = nullptr);
    void accept() override;
    void enableButtons();
    void disableButtons();
    void retranslateUi();
    void setCardSourceUrl(const QString &sourceUrl)
    {
        cardSourceUrl = sourceUrl;
    }
    const QString &getCardSourceUrl() const
    {
        return cardSourceUrl;
    }
    void setTempDir(const QString &tempDir)
    {
        tmpDir = tempDir;
    }
    const QString &getTempDir() const
    {
        return tmpDir;
    }

    QNetworkAccessManager *nam;
    QByteArray xmlData;
    bool doneWithParsing = false;

private:
    QString cardSourceUrl;
    QString tmpDir;
};

class LoadPreconsPage : public QWizardPage
{
    Q_OBJECT
public:
    explicit LoadPreconsPage(QWidget *parent = nullptr);
    void retranslateUi();

protected:
    void initializePage() override;
    bool validatePage() override;
    bool parsePreconsFromByteArray(const QByteArray &data, QString folderPath);
    void readPreconsFromByteArray(QByteArray _data);
    static QString createTmpDirectory();
    void processTarArchive(const QByteArray &tarData);
    void readPreconsFromByteArrayRef(QByteArray &_data);
    void downloadPreconsFile(const QUrl &url);

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
    void actLoadPreconsFile();
    void actRestoreDefaultUrl();
    void actDownloadProgressPreconsFile(qint64 received, qint64 total);
    void actDownloadFinishedPreconsFile();
    void importFinished();
    void zipDownloadFailed(const QString &message);
};

class SavePreconsPage : public QWizardPage
{
    Q_OBJECT
public:
    explicit SavePreconsPage(QWidget *parent = nullptr);
    void retranslateUi();

private:
    QTreeWidget *folderTreeWidget;
    QLabel *saveLabel;

protected:
    void initializePage() override;
    void populateFolderTree(QTreeWidgetItem *parent, const QString &path);
    void onItemChanged(QTreeWidgetItem *item, int column);
    void cleanupPage() override;
    bool validatePage() override;
    void copyCheckedFolders(QTreeWidgetItem *item);
    bool copyDirectory(const QString &srcPath, const QString &destPath);
};

bool readPreconsFromByteArray(const QByteArray &data);

#endif // DLG_IMPORT_PRECONS_H
