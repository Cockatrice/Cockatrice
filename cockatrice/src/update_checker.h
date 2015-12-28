//
// Created by miguel on 28/12/15.
//

#ifndef COCKATRICE_UPDATECHECKER_H
#define COCKATRICE_UPDATECHECKER_H

#include <QObject>
#include <QUrl>
#include <QDate>
#include <QtNetwork>

class UpdateChecker : public QObject {
Q_OBJECT
public:
    UpdateChecker(QObject *parent);
    ~UpdateChecker();
    void check();
signals:
    void finishedCheck(bool needToUpdate, bool isCompatible, QVariantMap *build);
    void error(QString errorString);
private:
    static QVariantMap *findCompatibleBuild();
    static QDate findOldestBuild(QVariantList allBuilds);
    static QDate dateFromBuild(QVariant build);
    static QVariantMap *findCompatibleBuild(QVariantList allBuilds);
    static bool downloadMatchesCurrentOS(QVariant build);
    QVariantMap *build;
    QUrl latestFilesUrl;
    QDate buildDate;
    QNetworkAccessManager *netMan;
    QNetworkReply *response;
private slots:
    void fileListFinished();
};


#endif //COCKATRICE_UPDATECHECKER_H
