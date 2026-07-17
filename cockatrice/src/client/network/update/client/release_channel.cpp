#include "release_channel.h"

#include "version_string.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QNetworkReply>
#include <QRegularExpression>
#include <QSysInfo>
#include <QtGlobal>
#include <optional>

#if defined(Q_OS_MACOS)
#include <sys/sysctl.h>
#include <sys/types.h>
#endif

#define STABLERELEASE_URL "https://api.github.com/repos/Cockatrice/Cockatrice/releases/latest"
#define STABLEMANUALDOWNLOAD_URL "https://github.com/Cockatrice/Cockatrice/releases/latest"
#define STABLETAG_URL "https://api.github.com/repos/Cockatrice/Cockatrice/git/refs/tags/"

#define BETARELEASE_URL "https://api.github.com/repos/Cockatrice/Cockatrice/releases"
#define BETAMANUALDOWNLOAD_URL "https://github.com/Cockatrice/Cockatrice/releases/"
#define BETARELEASE_CHANGESURL "https://github.com/Cockatrice/Cockatrice/compare/%1...%2"

#define GIT_SHORT_HASH_LEN 7

ReleaseChannel::ReleaseChannel() : netMan(new QNetworkAccessManager(this)), response(nullptr), lastRelease(nullptr)
{
}

ReleaseChannel::~ReleaseChannel()
{
    netMan->deleteLater();
}

void ReleaseChannel::checkForUpdates()
{
    QString releaseChannelUrl = getReleaseChannelUrl();
    qCInfo(ReleaseChannelLog) << "Searching for updates on the channel: " << releaseChannelUrl;
    response = netMan->get(QNetworkRequest(releaseChannelUrl));
    connect(response, &QNetworkReply::finished, this, &ReleaseChannel::releaseListFinished);
}

// Find compatible assets for host platform (Linux is not supported by in-client updater)
std::optional<int> ReleaseChannel::getTargetVersionForCurrentOS(const QString &fileName)
{
    const QRegularExpression *regex = nullptr;

#if defined(Q_OS_MACOS)
    const bool isIntel = [] {
        // QSysInfo does not go through translation layers
        // We need to use sysctl to reliably detect the underlying architecture
        char arch[255];
        size_t len = sizeof(arch);
        if (sysctlbyname("machdep.cpu.brand_string", arch, &len, nullptr, 0) == 0) {
            return QString::fromUtf8(arch).contains("Intel");
        }
        return false;
    }();
    static const QRegularExpression macIntelRegex(R"(-macOS(\d+)_Intel\.[^.]+$)",
                                                  QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression macArmRegex(R"(-macOS(\d+)\.[^.]+$)", QRegularExpression::CaseInsensitiveOption);
    regex = isIntel ? &macIntelRegex : &macArmRegex;

#elif defined(Q_OS_WIN)
    static const QRegularExpression winRegex(R"(-(?:Win|Windows)(\d+)\.[^.]+$)",
                                             QRegularExpression::CaseInsensitiveOption);
    regex = &winRegex;

#else // if the OS doesn't fit one of the above #defines, then it will never match
    Q_UNUSED(fileName);
    return std::nullopt;
#endif

    // Any asset targeting an OS version smaller or equal to the current host works
    const int systemVersion = QSysInfo::productVersion().split('.').first().toInt();
    auto match = regex->match(fileName);
    if (!match.hasMatch()) {
        return std::nullopt;
    }
    const int targetVersion = match.captured(1).toInt();
    if (targetVersion > systemVersion) {
        return std::nullopt;
    }
    return targetVersion;
}

// Pick newest targeted version (highest number) amongst all compatible ones
QString ReleaseChannel::findBestDownloadUrl(const QVariantList &assets)
{
    QString bestUrl;
    int bestVersion = -1;
    for (const auto &rawAsset : assets) {
        QVariantMap asset = rawAsset.toMap();
        QString name = asset["name"].toString();
        QString url = asset["browser_download_url"].toString();
        auto version = getTargetVersionForCurrentOS(name);
        if (!version) {
            continue;
        }
        if (*version > bestVersion) {
            bestVersion = *version;
            bestUrl = url;
        }
    }
    if (!bestUrl.isEmpty()) {
        qCInfo(ReleaseChannelLog) << "Best compatible asset=" << bestUrl;
    }
    return bestUrl;
}

QString StableReleaseChannel::getManualDownloadUrl() const
{
    return QString(STABLEMANUALDOWNLOAD_URL);
}

QString StableReleaseChannel::getName() const
{
    return tr("Default");
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
        qCWarning(ReleaseChannelLog) << "No reply received from the release update server.";
        emit error(tr("No reply received from the release update server."));
        return;
    }

    QVariantMap resultMap = jsonResponse.toVariant().toMap();
    if (!(resultMap.contains("name") && resultMap.contains("html_url") && resultMap.contains("tag_name") &&
          resultMap.contains("published_at"))) {
        qCWarning(ReleaseChannelLog) << "Invalid received from the release update server:" << resultMap;
        emit error(tr("Invalid reply received from the release update server."));
        return;
    }

    if (!lastRelease) {
        lastRelease = new Release;
    }

    lastRelease->setName(resultMap["name"].toString());
    lastRelease->setDescriptionUrl(resultMap["html_url"].toString());
    lastRelease->setPublishDate(resultMap["published_at"].toDate());

    if (resultMap.contains("assets")) {
        auto url = findBestDownloadUrl(resultMap["assets"].toList());
        if (!url.isEmpty()) {
            lastRelease->setDownloadUrl(url);
        }
    }

    QString shortHash = lastRelease->getCommitHash().left(GIT_SHORT_HASH_LEN);
    QString myHash = QString(VERSION_COMMIT);
    qCInfo(ReleaseChannelLog) << "Current hash=" << myHash << "update hash=" << shortHash;

    qCInfo(ReleaseChannelLog) << "Got reply from release server, name=" << lastRelease->getName()
                              << "desc=" << lastRelease->getDescriptionUrl() << "date=" << lastRelease->getPublishDate()
                              << "url=" << lastRelease->getDownloadUrl();

    const QString &tagName = resultMap["tag_name"].toString();
    QString url = QString(STABLETAG_URL) + tagName;
    qCInfo(ReleaseChannelLog) << "Searching for commit hash corresponding to stable channel tag: " << tagName;
    response = netMan->get(QNetworkRequest(url));
    connect(response, &QNetworkReply::finished, this, &StableReleaseChannel::tagListFinished);
}

void StableReleaseChannel::tagListFinished()
{
    auto *reply = static_cast<QNetworkReply *>(sender());
    QJsonParseError parseError{};
    QJsonDocument jsonResponse = QJsonDocument::fromJson(reply->readAll(), &parseError);
    reply->deleteLater();
    if (parseError.error != QJsonParseError::NoError) {
        qCWarning(ReleaseChannelLog) << "No reply received from the tag update server.";
        emit error(tr("No reply received from the tag update server."));
        return;
    }

    QVariantMap resultMap = jsonResponse.toVariant().toMap();
    if (!(resultMap.contains("object") && resultMap["object"].toMap().contains("sha"))) {
        qCWarning(ReleaseChannelLog) << "Invalid received from the tag update server.";
        emit error(tr("Invalid reply received from the tag update server."));
        return;
    }

    lastRelease->setCommitHash(resultMap["object"].toMap()["sha"].toString());
    qCInfo(ReleaseChannelLog) << "Got reply from tag server, commit=" << lastRelease->getCommitHash();

    QString shortHash = lastRelease->getCommitHash().left(GIT_SHORT_HASH_LEN);
    QString myHash = QString(VERSION_COMMIT);
    qCInfo(ReleaseChannelLog) << "Current hash=" << myHash << "update hash=" << shortHash;
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
    return tr("Beta");
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
        qCWarning(ReleaseChannelLog) << "No reply received from the release update server:" << QString(jsonData);
        emit error(tr("No reply received from the release update server."));
        return;
    }

    // Make sure resultMap has all elements we'll need
    if (!resultMap.contains("assets") || !resultMap.contains("author") || !resultMap.contains("tag_name") ||
        !resultMap.contains("target_commitish") || !resultMap.contains("assets_url") ||
        !resultMap.contains("published_at")) {
        qCWarning(ReleaseChannelLog) << "Invalid received from the release update server:" << resultMap;
        emit error(tr("Invalid reply received from the release update server."));
        return;
    }

    if (lastRelease == nullptr) {
        lastRelease = new Release;
    }

    lastRelease->setCommitHash(resultMap["target_commitish"].toString());
    lastRelease->setPublishDate(resultMap["published_at"].toDate());

    QString shortHash = lastRelease->getCommitHash().left(GIT_SHORT_HASH_LEN);
    lastRelease->setName(QString("%1 (%2)").arg(resultMap["tag_name"].toString()).arg(shortHash));
    lastRelease->setDescriptionUrl(QString(BETARELEASE_CHANGESURL).arg(VERSION_COMMIT, shortHash));

    qCInfo(ReleaseChannelLog) << "Got reply from release server, size=" << resultMap.size()
                              << "name=" << lastRelease->getName() << "desc=" << lastRelease->getDescriptionUrl()
                              << "commit=" << lastRelease->getCommitHash() << "date=" << lastRelease->getPublishDate();

    QString betaBuildDownloadUrl = resultMap["assets_url"].toString();

    qCInfo(ReleaseChannelLog) << "Searching for a corresponding file on the beta channel: " << betaBuildDownloadUrl;
    response = netMan->get(QNetworkRequest(betaBuildDownloadUrl));
    connect(response, &QNetworkReply::finished, this, &BetaReleaseChannel::fileListFinished);
}

void BetaReleaseChannel::fileListFinished()
{
    auto *reply = static_cast<QNetworkReply *>(sender());
    QJsonParseError parseError{};
    QJsonDocument jsonResponse = QJsonDocument::fromJson(reply->readAll(), &parseError);
    reply->deleteLater();
    if (parseError.error != QJsonParseError::NoError) {
        qCWarning(ReleaseChannelLog) << "No reply received from the file update server.";
        emit error(tr("No reply received from the file update server."));
        return;
    }

    QVariantList resultList = jsonResponse.toVariant().toList();
    QString shortHash = lastRelease->getCommitHash().left(GIT_SHORT_HASH_LEN);
    QString myHash = QString(VERSION_COMMIT);
    qCInfo(ReleaseChannelLog) << "Current hash=" << myHash << "update hash=" << shortHash;

    bool needToUpdate = (QString::compare(shortHash, myHash, Qt::CaseInsensitive) != 0);
    bool compatibleVersion = false;

    QString downloadUrl = findBestDownloadUrl(resultList);
    if (!downloadUrl.isEmpty()) {
        compatibleVersion = true;
        lastRelease->setDownloadUrl(downloadUrl);
    }

    emit finishedCheck(needToUpdate, compatibleVersion, lastRelease);
}
