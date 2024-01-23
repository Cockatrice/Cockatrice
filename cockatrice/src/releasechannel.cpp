#include "releasechannel.h"

#include "version_string.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QNetworkReply>
#include <QRegularExpression>
#include <QSysInfo>
#include <QtGlobal>

#define STABLERELEASE_URL "https://api.github.com/repos/Cockatrice/Cockatrice/releases/latest"
#define STABLEMANUALDOWNLOAD_URL "https://github.com/Cockatrice/Cockatrice/releases/latest"
#define STABLETAG_URL "https://api.github.com/repos/Cockatrice/Cockatrice/git/refs/tags/"

#define BETARELEASE_URL "https://api.github.com/repos/Cockatrice/Cockatrice/releases"
#define BETAMANUALDOWNLOAD_URL "https://github.com/Cockatrice/Cockatrice/releases/"
#define BETARELEASE_CHANGESURL "https://github.com/Cockatrice/Cockatrice/compare/%1...%2"

#define GIT_SHORT_HASH_LEN 7

int ReleaseChannel::sharedIndex = 0;

ReleaseChannel::ReleaseChannel() : netMan(new QNetworkAccessManager(this)), response(nullptr), lastRelease(nullptr)
{
    index = sharedIndex++;
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

// Different release channel checking functions for different operating systems
#if defined(Q_OS_MACOS)
bool ReleaseChannel::downloadMatchesCurrentOS(const QString &fileName)
{
    static QRegularExpression version_regex("macOS-(\\d+)\\.(\\d+)");
    auto match = version_regex.match(fileName);
    if (!match.hasMatch()) {
        return false;
    }

    // older(smaller) releases are compatible with a newer or the same system version
    int sys_maj = QSysInfo::productVersion().split(".")[0].toInt();
    int sys_min = QSysInfo::productVersion().split(".")[1].toInt();
    int rel_maj = match.captured(1).toInt();
    int rel_min = match.captured(2).toInt();
    return rel_maj < sys_maj || (rel_maj == sys_maj && rel_min <= sys_min);
}
#elif defined(Q_OS_WIN)
bool ReleaseChannel::downloadMatchesCurrentOS(const QString &fileName)
{
#if Q_PROCESSOR_WORDSIZE == 4
    return fileName.contains("32bit");
#elif Q_PROCESSOR_WORDSIZE == 8
    const QString &version = QSysInfo::productVersion();
    if (version.startsWith("7") || version.startsWith("8")) {
        return fileName.contains("Win7");
    } else {
        return fileName.contains("Win10");
    }
#else
    return false;
#endif
}
#else
bool ReleaseChannel::downloadMatchesCurrentOS(const QString &)
{
    // If the OS doesn't fit one of the above #defines, then it will never match
    return false;
}
#endif

QString StableReleaseChannel::getManualDownloadUrl() const
{
    return QString(STABLEMANUALDOWNLOAD_URL);
}

QString StableReleaseChannel::getName() const
{
    return tr("Stable Releases");
}

QString StableReleaseChannel::getReleaseChannelUrl() const
{
    return QString(STABLERELEASE_URL);
}

void StableReleaseChannel::releaseListFinished()
{
    auto *reply = static_cast<QNetworkReply *>(sender());
    QJsonParseError parseError{};
    QJsonDocument jsonResponse = QJsonDocument::fromJson(reply->readAll(), &parseError);
    reply->deleteLater();
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "No reply received from the release update server.";
        emit error(tr("No reply received from the release update server."));
        return;
    }

    QVariantMap resultMap = jsonResponse.toVariant().toMap();
    if (!(resultMap.contains("name") && resultMap.contains("html_url") && resultMap.contains("tag_name") &&
          resultMap.contains("published_at"))) {
        qWarning() << "Invalid received from the release update server.";
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

    qDebug() << "Got reply from release server, name=" << lastRelease->getName()
             << "desc=" << lastRelease->getDescriptionUrl() << "date=" << lastRelease->getPublishDate()
             << "url=" << lastRelease->getDownloadUrl();

    const QString &tagName = resultMap["tag_name"].toString();
    QString url = QString(STABLETAG_URL) + tagName;
    qDebug() << "Searching for commit hash corresponding to stable channel tag: " << tagName;
    response = netMan->get(QNetworkRequest(url));
    connect(response, SIGNAL(finished()), this, SLOT(tagListFinished()));
}

void StableReleaseChannel::tagListFinished()
{
    auto *reply = static_cast<QNetworkReply *>(sender());
    QJsonParseError parseError{};
    QJsonDocument jsonResponse = QJsonDocument::fromJson(reply->readAll(), &parseError);
    reply->deleteLater();
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "No reply received from the tag update server.";
        emit error(tr("No reply received from the tag update server."));
        return;
    }

    QVariantMap resultMap = jsonResponse.toVariant().toMap();
    if (!(resultMap.contains("object") && resultMap["object"].toMap().contains("sha"))) {
        qWarning() << "Invalid received from the tag update server.";
        emit error(tr("Invalid reply received from the tag update server."));
        return;
    }

    lastRelease->setCommitHash(resultMap["object"].toMap()["sha"].toString());
    qDebug() << "Got reply from tag server, commit=" << lastRelease->getCommitHash();

    QString shortHash = lastRelease->getCommitHash().left(GIT_SHORT_HASH_LEN);
    QString myHash = QString(VERSION_COMMIT);
    qDebug() << "Current hash=" << myHash << "update hash=" << shortHash;
    const bool needToUpdate = (QString::compare(shortHash, myHash, Qt::CaseInsensitive) != 0);

    emit finishedCheck(needToUpdate, lastRelease->isCompatibleVersionFound(), lastRelease);
}

void StableReleaseChannel::fileListFinished()
{
    // Only implemented to satisfy interface
}

QString BetaReleaseChannel::getManualDownloadUrl() const
{
    return QString(BETAMANUALDOWNLOAD_URL);
}

QString BetaReleaseChannel::getName() const
{
    return tr("Beta Releases");
}

QString BetaReleaseChannel::getReleaseChannelUrl() const
{
    return QString(BETARELEASE_URL);
}

void BetaReleaseChannel::releaseListFinished()
{
    auto *reply = static_cast<QNetworkReply *>(sender());
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

    if (array.empty() || resultMap.empty()) {
        qWarning() << "No reply received from the release update server:" << QString(jsonData);
        emit error(tr("No reply received from the release update server."));
        return;
    }

    // Make sure resultMap has all elements we'll need
    if (!resultMap.contains("assets") || !resultMap.contains("author") || !resultMap.contains("tag_name") ||
        !resultMap.contains("target_commitish") || !resultMap.contains("assets_url") ||
        !resultMap.contains("published_at")) {
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
    lastRelease->setDescriptionUrl(QString(BETARELEASE_CHANGESURL).arg(VERSION_COMMIT, shortHash));

    qDebug() << "Got reply from release server, size=" << resultMap.size() << "name=" << lastRelease->getName()
             << "desc=" << lastRelease->getDescriptionUrl() << "commit=" << lastRelease->getCommitHash()
             << "date=" << lastRelease->getPublishDate();

    QString betaBuildDownloadUrl = resultMap["assets_url"].toString();

    qDebug() << "Searching for a corresponding file on the beta channel: " << betaBuildDownloadUrl;
    response = netMan->get(QNetworkRequest(betaBuildDownloadUrl));
    connect(response, SIGNAL(finished()), this, SLOT(fileListFinished()));
}

void BetaReleaseChannel::fileListFinished()
{
    auto *reply = static_cast<QNetworkReply *>(sender());
    QJsonParseError parseError{};
    QJsonDocument jsonResponse = QJsonDocument::fromJson(reply->readAll(), &parseError);
    reply->deleteLater();
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "No reply received from the file update server.";
        emit error(tr("No reply received from the file update server."));
        return;
    }

    QVariantList resultList = jsonResponse.toVariant().toList();
    QString shortHash = lastRelease->getCommitHash().left(GIT_SHORT_HASH_LEN);
    QString myHash = QString(VERSION_COMMIT);
    qDebug() << "Current hash=" << myHash << "update hash=" << shortHash;

    bool needToUpdate = (QString::compare(shortHash, myHash, Qt::CaseInsensitive) != 0);
    bool compatibleVersion = false;

    QStringList resultUrlList{};
    for (QVariant file : resultList) {
        QVariantMap map = file.toMap();
        resultUrlList << map["browser_download_url"].toString();
    }

    resultUrlList.sort();
    // iterate in reverse so the first item is the latest os version
    for (auto url = resultUrlList.rbegin(); url < resultUrlList.rend(); ++url) {
        if (downloadMatchesCurrentOS(*url)) {
            compatibleVersion = true;
            lastRelease->setDownloadUrl(*url);
            qDebug() << "Found compatible version url=" << *url;
            break;
        }
    }

    emit finishedCheck(needToUpdate, compatibleVersion, lastRelease);
}
