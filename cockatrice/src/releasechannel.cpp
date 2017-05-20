#include "releasechannel.h"
#include "qt-json/json.h"
#include "version_string.h"

#include <QNetworkReply>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#define STABLERELEASE_URL "https://api.github.com/repos/Cockatrice/Cockatrice/releases/latest"
#define STABLETAG_URL "https://api.github.com/repos/Cockatrice/Cockatrice/git/refs/tags/"
#define STABLEMANUALDOWNLOAD_URL "https://github.com/Cockatrice/Cockatrice/releases/latest"

#define DEVRELEASE_URL "https://api.github.com/repos/Cockatrice/Cockatrice/releases"
#define DEVMANUALDOWNLOAD_URL "https://github.com/Cockatrice/Cockatrice/releases/"
#define DEVRELEASE_DESCURL "https://github.com/Cockatrice/Cockatrice/compare/%1...%2"

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
bool ReleaseChannel::downloadMatchesCurrentOS(const QString &fileName)
{
    return fileName.endsWith(".dmg");
}

#elif defined(Q_OS_WIN)

#include <QSysInfo>

bool ReleaseChannel::downloadMatchesCurrentOS(const QString &fileName)
{
    QString wordSize = QSysInfo::buildAbi().split('-')[2];
    QString arch;
    QString devSnapshotEnd;

    if (wordSize == "llp64") {
        arch = "win64";
        devSnapshotEnd = "-x86_64_qt5";
    } else if (wordSize == "ilp32") {
        arch = "win32";
        devSnapshotEnd = "-x86_qt5";
    } else {
        qWarning() << "Error checking for upgrade version: wordSize is" << wordSize;
        return false;
    }

    auto exeName = arch + ".exe";
    auto exeDevName = devSnapshotEnd + ".exe";
    return (fileName.endsWith(exeName) || fileName.endsWith(exeDevName));
}
#else

bool ReleaseChannel::downloadMatchesCurrentOS(const QString &)
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

    if (!lastRelease)
        lastRelease = new Release;

    lastRelease->setName(resultMap["name"].toString());
    lastRelease->setDescriptionUrl(resultMap["html_url"].toString());
    lastRelease->setPublishDate(resultMap["published_at"].toDate());

    if (resultMap.contains("assets")) {
        auto rawAssets = resultMap["assets"].toList();
        // [(name, url)]
        QVector<std::pair<QString, QString>> assets;
        std::transform(rawAssets.begin(), rawAssets.end(), std::back_inserter(assets), [](QVariant _asset) {
            QVariantMap asset = _asset.toMap();
            QString name = asset["name"].toString();
            QString url = asset["browser_download_url"].toString();
            return std::make_pair(name, url);
        });

        auto _releaseAsset = std::find_if(assets.begin(), assets.end(), [](std::pair<QString, QString> nameAndUrl) {
           return downloadMatchesCurrentOS(nameAndUrl.first);
        });

        if (_releaseAsset != assets.end()) {
            std::pair<QString, QString> releaseAsset = *_releaseAsset;
            auto releaseUrl = releaseAsset.second;
            lastRelease->setDownloadUrl(releaseUrl);
        }
    }

    QString shortHash = lastRelease->getCommitHash().left(GIT_SHORT_HASH_LEN);
    QString myHash = QString(VERSION_COMMIT);
    qDebug() << "Current hash=" << myHash << "update hash=" << shortHash;

    qDebug() << "Got reply from release server, size=" << tmp.size()
        << "name=" << lastRelease->getName()
        << "desc=" << lastRelease->getDescriptionUrl()
        << "date=" << lastRelease->getPublishDate()
        << "url=" << lastRelease->getDownloadUrl();

    const QString &tagName = resultMap["tag_name"].toString();
    QString url = QString(STABLETAG_URL) + tagName;
    qDebug() << "Searching for commit hash corresponding to stable channel tag: " << tagName;
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


    QString shortHash = lastRelease->getCommitHash().left(GIT_SHORT_HASH_LEN);
    QString myHash = QString(VERSION_COMMIT);
    qDebug() << "Current hash=" << myHash << "update hash=" << shortHash;
    const bool needToUpdate = (QString::compare(shortHash, myHash, Qt::CaseInsensitive) != 0);


    emit finishedCheck(needToUpdate, lastRelease->isCompatibleVersionFound(), lastRelease);
}

void StableReleaseChannel::fileListFinished()
{
    // Only implemented to satisfy interface
    return;
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
    QByteArray jsonData = reply->readAll();
    reply->deleteLater();

    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    QJsonArray array = doc.array();

    /*
     * Get the latest release on GitHub
     * This can be either a pre-release or a published release
     * depending on timing. Both are acceptable.
     */
    QVariantMap resultMap = array.at(0).toObject().toVariantMap();

    if (array.size() == 0 || resultMap.size() == 0) {
        qWarning() << "No reply received from the release update server:" << QString(jsonData);
        emit error(tr("No reply received from the release update server."));
        return;
    }

    // Make sure resultMap has all elements we'll need
    if (!resultMap.contains("assets") ||
        !resultMap.contains("author") ||
        !resultMap.contains("tag_name") ||
        !resultMap.contains("target_commitish") ||
        !resultMap.contains("assets_url") ||
        !resultMap.contains("published_at"))
    {
        qWarning() << "Invalid received from the release update server:" << resultMap;
        emit error(tr("Invalid reply received from the release update server."));
        return;
    }

    if (lastRelease == nullptr)
        lastRelease = new Release;

    lastRelease->setCommitHash(resultMap["target_commitish"].toString());
    lastRelease->setPublishDate(resultMap["published_at"].toDate());

    QString shortHash = lastRelease->getCommitHash().left(GIT_SHORT_HASH_LEN);
    lastRelease->setName(QString("%1 (%2)").arg(resultMap["tag_name"].toString()).arg(shortHash));
    lastRelease->setDescriptionUrl(QString(DEVRELEASE_DESCURL).arg(VERSION_COMMIT, shortHash));
    
    qDebug() << "Got reply from release server, size=" << resultMap.size()
        << "name=" << lastRelease->getName()
        << "desc=" << lastRelease->getDescriptionUrl()
        << "commit=" << lastRelease->getCommitHash()
        << "date=" << lastRelease->getPublishDate();

    QString devBuildDownloadUrl = resultMap["assets_url"].toString();

    qDebug() << "Searching for a corresponding file on the dev channel: " << devBuildDownloadUrl;
    response = netMan->get(QNetworkRequest(devBuildDownloadUrl));
    connect(response, SIGNAL(finished()), this, SLOT(fileListFinished()));
}

void DevReleaseChannel::fileListFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    QByteArray jsonData = reply->readAll();
    reply->deleteLater();
    bool ok;

    QVariantList resultList = QtJson::Json::parse(jsonData, ok).toList();
    if (!ok) {
        qWarning() << "No reply received from the file update server:" << QString(jsonData);
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

        QString url = map["browser_download_url"].toString();

        if (!downloadMatchesCurrentOS(url))
            continue;

        compatibleVersion = true;
        lastRelease->setDownloadUrl(url);
        qDebug() << "Found compatible version url=" << url;
        break;
    }
    
    emit finishedCheck(needToUpdate, compatibleVersion, lastRelease);
}
