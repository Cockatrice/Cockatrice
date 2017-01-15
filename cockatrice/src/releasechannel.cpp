#include "releasechannel.h"
#include "qt-json/json.h"
#include "version_string.h"

#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QMessageBox>

#define STABLERELEASE_URL "https://api.github.com/repos/Cockatrice/Cockatrice/releases/latest"
#define STABLETAG_URL "https://api.github.com/repos/Cockatrice/Cockatrice/git/refs/tags/"
#define STABLEFILES_URL "https://api.bintray.com/packages/cockatrice/Cockatrice/Cockatrice/files"
#define STABLEDOWNLOAD_URL "https://dl.bintray.com/cockatrice/Cockatrice/"
#define STABLEMANUALDOWNLOAD_URL "https://bintray.com/cockatrice/Cockatrice/Cockatrice/_latestVersion#files"

#define DEVRELEASE_URL "https://api.github.com/repos/Cockatrice/Cockatrice/commits/master"
#define DEVFILES_URL "https://api.bintray.com/packages/cockatrice/Cockatrice/Cockatrice-git/files"
#define DEVDOWNLOAD_URL "https://dl.bintray.com/cockatrice/Cockatrice/"
#define DEVMANUALDOWNLOAD_URL "https://bintray.com/cockatrice/Cockatrice/Cockatrice-git/_latestVersion#files"
#define GIT_SHORT_HASH_LEN 7

int ReleaseChannel::sharedIndex = 0;

ReleaseChannel::ReleaseChannel()
: response(nullptr), lastRelease(nullptr)
{
    index = sharedIndex++;
    netMan = new QNetworkAccessManager(this);
}

ReleaseChannel::~ReleaseChannel()
{
    netMan->deleteLater();
}

void ReleaseChannel::checkForUpdates()
{
    QString releaseChannelUrl = getReleaseChannelUrl();
    qDebug() << "Searching for updates on the channel: " << releaseChannelUrl;
    response = netMan->get(QNetworkRequest(releaseChannelUrl));
    connect(response, SIGNAL(finished()), this, SLOT(releaseListFinished()));
}

#if defined(Q_OS_OSX)
bool ReleaseChannel::downloadMatchesCurrentOS(QVariantMap build)
{
    return build["name"].toString().endsWith(".dmg");
}

#elif defined(Q_OS_WIN)

#include <QSysInfo>

bool ReleaseChannel::downloadMatchesCurrentOS(QVariantMap build)
{
    QString wordSize = QSysInfo::buildAbi().split('-')[2];
    QString arch;
    if (wordSize == "llp64") {
        arch = "win64";
    } else if (wordSize == "ilp32") {
        arch = "win32";
    } else {
        qWarning() << "Error checking for upgrade version: wordSize is" << wordSize;
        return false;
    }

    auto fileName = build["name"].toString();
    // Checking for .zip is a workaround for the May 6th 2016 release
    auto zipName = arch + ".zip";
    auto exeName = arch + ".exe";
    return fileName.endsWith(exeName) || fileName.endsWith(zipName);
}
#else

bool ReleaseChannel::downloadMatchesCurrentOS(QVariantMap)
{
    //If the OS doesn't fit one of the above #defines, then it will never match
    return false;
}

#endif

QString StableReleaseChannel::getManualDownloadUrl() const
{
    return QString(STABLEMANUALDOWNLOAD_URL);
}

QString StableReleaseChannel::getName() const
{
    return tr("Stable releases");
}

QString StableReleaseChannel::getReleaseChannelUrl() const
{
    return QString(STABLERELEASE_URL);
}

void StableReleaseChannel::releaseListFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    bool ok;
    QString tmp = QString(reply->readAll());
    reply->deleteLater();

    QVariantMap resultMap = QtJson::Json::parse(tmp, ok).toMap();
    if (!ok) {
        qWarning() << "No reply received from the release update server:" << tmp;
        emit error(tr("No reply received from the release update server."));
        return;
    }

    if(!(resultMap.contains("name") && 
        resultMap.contains("html_url") && 
        resultMap.contains("tag_name") && 
        resultMap.contains("published_at"))) {
        qWarning() << "Invalid received from the release update server:" << tmp;
        emit error(tr("Invalid reply received from the release update server."));
        return;
    }

    if(!lastRelease)
        lastRelease = new Release;

    lastRelease->setName(resultMap["name"].toString());
    lastRelease->setDescriptionUrl(resultMap["html_url"].toString());
    lastRelease->setPublishDate(resultMap["published_at"].toDate());

    qDebug() << "Got reply from release server, size=" << tmp.size()
        << "name=" << lastRelease->getName()
        << "desc=" << lastRelease->getDescriptionUrl()
        << "date=" << lastRelease->getPublishDate();

    QString url = QString(STABLETAG_URL) + resultMap["tag_name"].toString();
    qDebug() << "Searching for a corresponding tag on the stable channel: " << url;
    response = netMan->get(QNetworkRequest(url));
    connect(response, SIGNAL(finished()), this, SLOT(tagListFinished()));
}

void StableReleaseChannel::tagListFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    bool ok;
    QString tmp = QString(reply->readAll());
    reply->deleteLater();

    QVariantMap resultMap = QtJson::Json::parse(tmp, ok).toMap();
    if (!ok) {
        qWarning() << "No reply received from the tag update server:" << tmp;
        emit error(tr("No reply received from the tag update server."));
        return;
    }

    if(!(resultMap.contains("object") && 
        resultMap["object"].toMap().contains("sha"))) {
        qWarning() << "Invalid received from the tag update server:" << tmp;
        emit error(tr("Invalid reply received from the tag update server."));
        return;
    }

    lastRelease->setCommitHash(resultMap["object"].toMap()["sha"].toString());
    qDebug() << "Got reply from tag server, size=" << tmp.size()
        << "commit=" << lastRelease->getCommitHash();

    qDebug() << "Searching for a corresponding file on the stable channel: " << QString(STABLEFILES_URL);
    response = netMan->get(QNetworkRequest(QString(STABLEFILES_URL)));
    connect(response, SIGNAL(finished()), this, SLOT(fileListFinished()));
}

void StableReleaseChannel::fileListFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    bool ok;
    QString tmp = QString(reply->readAll());
    reply->deleteLater();

    QVariantList resultList = QtJson::Json::parse(tmp, ok).toList();
    if (!ok) {
        qWarning() << "No reply received from the file update server:" << tmp;
        emit error(tr("No reply received from the file update server."));
        return;
    }

    QString shortHash = lastRelease->getCommitHash().left(GIT_SHORT_HASH_LEN);
    QString myHash = QString(VERSION_COMMIT);
    qDebug() << "Current hash=" << myHash << "update hash=" << shortHash;

    bool needToUpdate = (QString::compare(shortHash, myHash, Qt::CaseInsensitive) != 0);
    bool compatibleVersion = false;

    foreach(QVariant file, resultList)
    {
        QVariantMap map = file.toMap();
        // TODO: map github version to bintray version
        /*
        if(!map.contains("version"))
            continue;
        if(!map["version"].toString().endsWith(shortHash))
            continue;
        */

        if(!downloadMatchesCurrentOS(map))
            continue;

        compatibleVersion = true;

        QString url = QString(STABLEDOWNLOAD_URL) + map["path"].toString();
        lastRelease->setDownloadUrl(url);
        qDebug() << "Found compatible version url=" << url;
        break;
    }
    
    emit finishedCheck(needToUpdate, compatibleVersion, lastRelease);
}

QString DevReleaseChannel::getManualDownloadUrl() const
{
    return QString(DEVMANUALDOWNLOAD_URL);
}

QString DevReleaseChannel::getName() const
{
    return tr("Development snapshots");
}

QString DevReleaseChannel::getReleaseChannelUrl() const
{
    return QString(DEVRELEASE_URL);
}

void DevReleaseChannel::releaseListFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    bool ok;
    QString tmp = QString(reply->readAll());
    reply->deleteLater();

    QVariantMap resultMap = QtJson::Json::parse(tmp, ok).toMap();
    if (!ok) {
        qWarning() << "No reply received from the release update server:" << tmp;
        emit error(tr("No reply received from the release update server."));
        return;
    }

    if(!(resultMap.contains("commit") && 
        resultMap.contains("html_url") && 
        resultMap.contains("sha") &&
        resultMap["commit"].toMap().contains("author") &&
        resultMap["commit"].toMap()["author"].toMap().contains("date"))) {
        qWarning() << "Invalid received from the release update server:" << tmp;
        emit error(tr("Invalid reply received from the release update server."));
        return;
    }

    if(!lastRelease)
        lastRelease = new Release;

    lastRelease->setName("Commit " + resultMap["sha"].toString());
    lastRelease->setDescriptionUrl(resultMap["html_url"].toString());
    lastRelease->setCommitHash(resultMap["sha"].toString());
    lastRelease->setPublishDate(resultMap["commit"].toMap()["author"].toMap()["date"].toDate());

    qDebug() << "Got reply from release server, size=" << tmp.size()
        << "name=" << lastRelease->getName()
        << "desc=" << lastRelease->getDescriptionUrl()
        << "commit=" << lastRelease->getCommitHash()
        << "date=" << lastRelease->getPublishDate();

    qDebug() << "Searching for a corresponding file on the dev channel: " << QString(DEVFILES_URL);
    response = netMan->get(QNetworkRequest(QString(DEVFILES_URL)));
    connect(response, SIGNAL(finished()), this, SLOT(fileListFinished()));
}

void DevReleaseChannel::fileListFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    bool ok;
    QString tmp = QString(reply->readAll());
    reply->deleteLater();

    QVariantList resultList = QtJson::Json::parse(tmp, ok).toList();
    if (!ok) {
        qWarning() << "No reply received from the file update server:" << tmp;
        emit error(tr("No reply received from the file update server."));
        return;
    }

    QString shortHash = lastRelease->getCommitHash().left(GIT_SHORT_HASH_LEN);
    QString myHash = QString(VERSION_COMMIT);
    qDebug() << "Current hash=" << myHash << "update hash=" << shortHash;

    bool needToUpdate = (QString::compare(shortHash, myHash, Qt::CaseInsensitive) != 0);
    bool compatibleVersion = false;

    foreach(QVariant file, resultList)
    {
        QVariantMap map = file.toMap();
        if(!map.contains("version"))
            continue;
        if(!map["version"].toString().endsWith(shortHash))
            continue;

        if(!downloadMatchesCurrentOS(map))
            continue;

        compatibleVersion = true;
        QString url = QString(DEVDOWNLOAD_URL) + map["path"].toString();
        lastRelease->setDownloadUrl(url);
        qDebug() << "Found compatible version url=" << url;
        break;
    }
    
    emit finishedCheck(needToUpdate, compatibleVersion, lastRelease);
}
