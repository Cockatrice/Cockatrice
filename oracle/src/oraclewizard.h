#ifndef ORACLEWIZARD_H
#define ORACLEWIZARD_H

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
    void setCardSourceUrl(const QString &sourceUrl)
    {
        cardSourceUrl = sourceUrl;
    }
    void setCardSourceVersion(const QString &sourceVersion)
    {
        cardSourceVersion = sourceVersion;
    }
    const QString &getCardSourceUrl() const
    {
        return cardSourceUrl;
    }
    const QString &getCardSourceVersion() const
    {
        return cardSourceVersion;
    }
    bool saveTokensToFile(const QString &fileName);

    void runInBackground()
    {
        backgroundMode = true;
        hide();
        currentPage()->initializePage();
    }

public:
    OracleImporter *importer;
    QSettings *settings;
    QNetworkAccessManager *nam;
    bool downloadedPlainXml = false;
    QByteArray xmlData;
    bool backgroundMode = false;

private slots:
    void updateLanguage();

private:
    QByteArray tokensData;
    QString cardSourceUrl;
    QString cardSourceVersion;

protected:
    void changeEvent(QEvent *event) override;
};

#endif
