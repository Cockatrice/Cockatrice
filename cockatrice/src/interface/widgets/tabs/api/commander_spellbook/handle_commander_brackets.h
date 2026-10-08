#ifndef COCKATRICE_HANDLE_COMMANDER_BRACKETS_H
#define COCKATRICE_HANDLE_COMMANDER_BRACKETS_H

#include <QNetworkReply>
#include <QObject>
#include <QVariantMap>
#include <qtmetamacros.h>

class QNetworkAccessManager;

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
