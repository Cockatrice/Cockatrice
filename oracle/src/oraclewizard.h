#ifndef ORACLEWIZARD_H
#define ORACLEWIZARD_H

#include <QByteArray>
#include <QString>
#include <QWizard>
#include <qtmetamacros.h>
#include <utility>

class QNetworkAccessManager;
class OracleImporter;
class QLocalServer;
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
    void handleRaiseRequest();

private:
    QByteArray tokensData;
    QString cardSourceUrl;
    QString cardSourceVersion;

    void migrateOracleSettings();
    /**
     * @brief Starts the local socket the hosting Cockatrice client uses to bring this wizard forward.
     */
    void startRaiseServer();
    /** @brief Shows, raises and activates the wizard window. */
    void raiseWizard();
    QLocalServer *raiseServer = nullptr; ///< listens for raise requests from the hosting client

protected:
    void changeEvent(QEvent *event) override;
};

#endif
