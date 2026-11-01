#include "release_channel.h"

#include "version_string.h"

#include <QByteArray>
#include <QDateTime>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QMap>
#include <QMessageLogger>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QUrl>
#include <QVariant>
#include <QVariantMap>
#include <QtGlobal>
#include <libcockatrice/network/network_manager.h>
#include <optional>
#include <qnamespace.h>
#include <qprocessordetection.h>

#if defined(Q_OS_MACOS)
#include <sys/sysctl.h>
#include <sys/types.h>
#endif

#define STABLERELEASE_URL "https://api.github.com/repos/Cockatrice/Cockatrice/releases/latest"
#define STABLEMANUALDOWNLOAD_URL "https://github.com/Cockatrice/Cockatrice/releases/latest"
#define RELEASECOMMIT_URL "https://api.github.com/repos/Cockatrice/Cockatrice/commits/"

#define BETARELEASE_URL "https://api.github.com/repos/Cockatrice/Cockatrice/releases"
#define BETAMANUALDOWNLOAD_URL "https://github.com/Cockatrice/Cockatrice/releases/"
#define BETARELEASE_CHANGESURL "https://github.com/Cockatrice/Cockatrice/compare/%1...%2"

#define GIT_SHORT_HASH_LEN 7

namespace
{

#if defined(Q_OS_MACOS) || defined(Q_OS_WIN)
// QSysInfo::productVersion() is not guaranteed to start with a number, e.g. it returns
// "Server 2022" on Windows Server. Use the first numeric token as the major host version.
std::optional<int> getProductVersionMajor()
{
    const QString versionString = QSysInfo::productVersion();
    bool ok = false;
    static const QRegularExpression firstNumber(R"((\d+))");
    const auto match = firstNumber.match(versionString);
    const int version = match.hasMatch() ? match.captured(1).toInt(&ok) : 0;
    if (!ok) {
        qCWarning(ReleaseChannelLog) << "Unable to determine OS version from" << versionString;
        return std::nullopt;
    }
    return version;
}
#endif

} // namespace

ReleaseChannel::ReleaseChannel() : netMan(NetworkManagerFactory::create(this)), response(nullptr), lastRelease(nullptr)
{
}

ReleaseChannel::~ReleaseChannel()
{
    if (response) {
        disconnect(response, nullptr, this, nullptr);
        response->abort();
    }
}

void ReleaseChannel::checkForUpdates()
{
    // Startup and the update dialog can ask at the same time. Both observers
    // should receive the result of one complete release lookup.
    if (response || pendingRelease) {
        return;
    }

    QString releaseChannelUrl = getReleaseChannelUrl();
    qCInfo(ReleaseChannelLog) << "Searching for updates on the channel: " << releaseChannelUrl;
    response = netMan->get(QNetworkRequest(releaseChannelUrl));
    connect(response, &QNetworkReply::finished, this, &ReleaseChannel::releaseListFinished);
}

// Find assets compatible with host platform (Linux is not supported by in-client updater)
std::optional<int> ReleaseChannel::getTargetVersionForCurrentOS(const QString &fileName)
{
#if defined(Q_OS_WIN) && Q_PROCESSOR_WORDSIZE == 4
    // The published Windows assets are 64-bit only
    Q_UNUSED(fileName);
    return std::nullopt;
#endif

    const QRegularExpression *regex = nullptr;

    // Only platforms shipping updater assets (macOS, Windows) need the host version
    static const std::optional<int> systemVersion =
#if defined(Q_OS_MACOS) || defined(Q_OS_WIN)
        getProductVersionMajor();
#else
        std::nullopt;
#endif

    if (!systemVersion) {
        return std::nullopt;
    }

#if defined(Q_OS_MACOS)
    static const bool isIntel = [] {
        // QSysInfo does not go through translation layers
        // We need to use sysctl to reliably detect the underlying architecture
        char arch[255];
        size_t len = sizeof(arch);
        if (sysctlbyname("machdep.cpu.brand_string", arch, &len, nullptr, 0) == 0) {
            return QString::fromUtf8(arch).contains("Intel");
        }
        return false;
    }();

    // Apple (ARM) --> Cockatrice-3.0.0-macOS15.dmg
    // Apple (x86) --> Cockatrice-3.0.0-macOS15_Intel.dmg
    static const QRegularExpression macIntelRegex(R"(-macOS(\d+)_Intel\.[^.]+$)",
                                                  QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression macRegex(R"(-macOS(\d+)\.[^.]+$)", QRegularExpression::CaseInsensitiveOption);
    regex = isIntel ? &macIntelRegex : &macRegex;

#elif defined(Q_OS_WIN)
    // Windows (x86) --> Cockatrice-3.0.0-Windows10.exe
    static const QRegularExpression winRegex(R"(-(?:Win|Windows)(\d+)\.[^.]+$)",
                                             QRegularExpression::CaseInsensitiveOption);
    regex = &winRegex;

#else // if the OS doesn't fit one of the above #defines, then it will never match
    Q_UNUSED(fileName);
    return std::nullopt;
#endif

    // Any asset targeting an OS version up to the current host version works
    auto match = regex->match(fileName);
    if (!match.hasMatch()) {
        return std::nullopt;
    }
    const int targetVersion = match.captured(1).toInt();
    if (targetVersion > *systemVersion) {
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
        const QUrl downloadUrl(url);
        if (!version || !downloadUrl.isValid() || downloadUrl.scheme() != QStringLiteral("https") ||
            downloadUrl.host().isEmpty()) {
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

bool ReleaseChannel::readResponse(QJsonDocument &document)
{
    auto *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply || reply != response) {
        return false;
    }

    response.clear();
    reply->deleteLater();
    const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (reply->error() != QNetworkReply::NoError || status < 200 || status >= 300) {
        pendingRelease.reset();
        qCWarning(ReleaseChannelLog) << "Release update request failed:" << status << reply->errorString();
        emit error(tr("Could not retrieve release information from the update server."));
        return false;
    }

    QJsonParseError parseError{};
    document = QJsonDocument::fromJson(reply->readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        invalidReply();
        return false;
    }
    return true;
}

void ReleaseChannel::invalidReply()
{
    pendingRelease.reset();
    qCWarning(ReleaseChannelLog) << "Invalid reply received from the release update server.";
    emit error(tr("Invalid reply received from the release update server."));
}

void ReleaseChannel::resolveRelease(const QJsonObject &release, bool beta)
{
    const QString tagName = release.value("tag_name").toString();
    const QUrl descriptionUrl(release.value("html_url").toString());
    const QDate publishDate = QDateTime::fromString(release.value("published_at").toString(), Qt::ISODate).date();
    if (tagName.isEmpty() || !descriptionUrl.isValid() || descriptionUrl.scheme() != QStringLiteral("https") ||
        descriptionUrl.host().isEmpty() || !publishDate.isValid() || !release.value("assets").isArray() ||
        (beta && !release.value("prerelease").isBool())) {
        invalidReply();
        return;
    }

    // Build a fresh candidate. In particular, a release without a matching
    // installer must never inherit the preceding release's download URL.
    pendingRelease = std::make_unique<Release>();
    // A stable release returned by the beta channel keeps its published title
    // and release notes; only prereleases use the tag/commit comparison view.
    pendingBeta = beta && release.value("prerelease").toBool();
    const QString name = release.value("name").toString();
    pendingRelease->setName(pendingBeta || name.isEmpty() ? tagName : name);
    pendingRelease->setDescriptionUrl(descriptionUrl.toString());
    pendingRelease->setPublishDate(publishDate);
    pendingRelease->setDownloadUrl(findBestDownloadUrl(release.value("assets").toArray().toVariantList()));

    // A release's target_commitish may be a branch, and a tag ref may point to
    // an annotated-tag object. The commits endpoint resolves either tag kind
    // to the actual commit shipped by this release.
    const QUrl commitUrl = QUrl::fromEncoded(QByteArray(RELEASECOMMIT_URL) + QUrl::toPercentEncoding(tagName));
    response = netMan->get(QNetworkRequest(commitUrl));
    connect(response, &QNetworkReply::finished, this, &ReleaseChannel::commitFinished);
}

void ReleaseChannel::commitFinished()
{
    QJsonDocument document;
    if (!readResponse(document)) {
        return;
    }
    const QString commitHash = document.object().value("sha").toString();
    static const QRegularExpression validCommitHash(QStringLiteral("^[0-9a-fA-F]{40}$"));
    if (!document.isObject() || !pendingRelease || !validCommitHash.match(commitHash).hasMatch()) {
        invalidReply();
        return;
    }

    pendingRelease->setCommitHash(commitHash);
    if (pendingBeta) {
        const QString shortHash = commitHash.left(GIT_SHORT_HASH_LEN);
        pendingRelease->setName(QString("%1 (%2)").arg(pendingRelease->getName(), shortHash));
        pendingRelease->setDescriptionUrl(QString(BETARELEASE_CHANGESURL).arg(VERSION_COMMIT, shortHash));
    }

    // Consumers can run nested event loops while displaying a release. Keep
    // completed snapshots alive until the channel is destroyed, so a later
    // check cannot mutate or invalidate an earlier result still in use.
    pendingRelease->setParent(this);
    lastRelease = pendingRelease.release();
    const QString currentHash = QString::fromLatin1(VERSION_COMMIT);
    const bool needToUpdate = currentHash.isEmpty() || !commitHash.startsWith(currentHash, Qt::CaseInsensitive);
    emit finishedCheck(needToUpdate, lastRelease->isCompatibleVersionFound(), lastRelease);
}

void StableReleaseChannel::releaseListFinished()
{
    QJsonDocument document;
    if (!readResponse(document)) {
        return;
    }
    if (!document.isObject()) {
        invalidReply();
        return;
    }
    resolveRelease(document.object(), false);
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
    QJsonDocument document;
    if (!readResponse(document)) {
        return;
    }
    if (!document.isArray()) {
        invalidReply();
        return;
    }

    // GitHub lists releases newest first. Both published and prerelease builds
    // are eligible, while drafts returned to an authenticated proxy are not.
    for (const auto &entry : document.array()) {
        if (!entry.isObject()) {
            invalidReply();
            return;
        }
        const QJsonObject release = entry.toObject();
        if (!release.value("draft").toBool()) {
            resolveRelease(release, true);
            return;
        }
    }
    invalidReply();
}
