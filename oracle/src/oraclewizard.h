#ifndef ORACLEWIZARD_H
#define ORACLEWIZARD_H

#include <QByteArray>
#include <QString>
#include <QWizard>
#include <qtmetamacros.h>
#include <utility>

class QNetworkAccessManager;
class OracleImporter;
class QSettings;
class QWidget;

class OracleWizard : public QWizard
{
    Q_OBJECT
public:
    explicit OracleWizard(QWidget *parent = nullptr);
    void accept() override;
    void reject() override;
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

    void runInBackground();

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

    void migrateOracleSettings();

protected:
    void changeEvent(QEvent *event) override;
};

#endif
