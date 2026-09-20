#ifndef COCKATRICE_HANDLE_COMMANDER_BRACKETS_H
#define COCKATRICE_HANDLE_COMMANDER_BRACKETS_H

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>

class HandleCommanderBrackets : public QObject
{
    Q_OBJECT

public:
    explicit HandleCommanderBrackets(QObject *parent = nullptr);

    void downloadBracketDefinitions();

signals:
    void sigBracketDefinitionsDownloaded();
    void sigBracketDefinitionsDownloadFailed(QNetworkReply::NetworkError error);

private slots:
    void actFinishParsingDownloadedData();

private:
    void updateBracketDefinitions(const QVariantMap &jsonMap);

    QNetworkAccessManager *nam;
    QNetworkReply *reply;
};

#endif // COCKATRICE_HANDLE_COMMANDER_BRACKETS_H
