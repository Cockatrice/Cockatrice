#include "spoiler_background_updater.h"

#include "../../../../main.h"
#include "../../../settings/cache_settings.h"
#include "spoiler_file.h"

#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QFutureWatcher>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QUrl>
#include <QtConcurrentRun>
#include <exception>
#include <libcockatrice/card/database/card_database.h>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/network/network_manager.h>
#include <libcockatrice/settings/download_settings.h>
#include <libcockatrice/settings/paths_settings.h>
#include <version_string.h>

namespace
{
const QUrl SPOILERS_STATUS_URL(
    QStringLiteral("https://raw.githubusercontent.com/Cockatrice/Magic-Spoiler/files/SpoilerSeasonEnabled"));
const QUrl SPOILERS_URL(QStringLiteral("https://raw.githubusercontent.com/Cockatrice/Magic-Spoiler/files/spoiler.xml"));
// Request coordination stays on the GUI thread; reload workers never access it.
QPointer<SpoilerBackgroundUpdater> currentUpdater;
QPointer<QFutureWatcher<bool>> activeReload;
quint64 newestOperation = 0;

bool removeSpoilerFile(const QString &path)
{
    QFile file(path);
    if (!file.exists()) {
        return false;
    }
    if (!file.remove()) {
        qCWarning(SpoilerBackgroundUpdaterLog) << "Could not remove spoiler database" << path << file.errorString();
        return false;
    }
    return true;
}
} // namespace

SpoilerBackgroundUpdater::SpoilerBackgroundUpdater(QObject *parent, bool forceReload)
    : QObject(parent), enabledAtStart(SettingsCache::instance().downloads().getDownloadSpoilersStatus()),
      forceReload(forceReload), targetPath(SettingsCache::instance().paths().getSpoilerCardDatabasePath()),
      operationId(++newestOperation), manager(NetworkManagerFactory::create(this)), notificationIcon(trayIcon)
{
    connect(&SettingsCache::instance().downloads(), &DownloadSettings::downloadSpoilerStatusChanged, this,
            &SpoilerBackgroundUpdater::cancelIfSettingsChanged);
    connect(&SettingsCache::instance().paths(), &PathsSettings::cardDatabasePathChanged, this,
            &SpoilerBackgroundUpdater::cancelIfSettingsChanged);
    // Even a disabled/no-op operation completes after its caller can connect signals.
    QTimer::singleShot(0, this, &SpoilerBackgroundUpdater::begin);
}

void SpoilerBackgroundUpdater::begin()
{
    if (completed) {
        return;
    }
    if (cancelled || operationId != newestOperation ||
        (reloadOnly ? SettingsCache::instance().downloads().getDownloadSpoilersStatus()
                    : (!enabledAtStart || !SettingsCache::instance().downloads().getDownloadSpoilersStatus())) ||
        targetPath != SettingsCache::instance().paths().getSpoilerCardDatabasePath()) {
        finish();
        return;
    }
    const auto previous = currentUpdater;
    if (activeReload) {
        // The existing database API stores its final load status after its parser
        // mutex is released. Do not overlap updater-owned calls to that API.
        connect(activeReload, &QFutureWatcher<bool>::finished, this, &SpoilerBackgroundUpdater::begin,
                static_cast<Qt::ConnectionType>(Qt::QueuedConnection | Qt::SingleShotConnection));
        if (previous && previous != this) {
            previous->cancel();
        }
        return;
    }
    currentUpdater = this;
    QPointer<SpoilerBackgroundUpdater> guard(this);
    if (previous && previous != this) {
        previous->cancel();
    }
    if (!guard) {
        return;
    }
    if (!isCurrent()) {
        finish();
    } else if (reloadOnly) {
        if (QFileInfo::exists(targetPath) && !removeSpoilerFile(targetPath)) {
            finish(tr("The spoiler database could not be removed. Check its file permissions and use Reload Card "
                      "Database to retry."));
            return;
        }
        reloadDatabase(false);
    } else {
        downloadFromURL(SPOILERS_STATUS_URL, false);
    }
}

bool SpoilerBackgroundUpdater::isCurrent() const
{
    return !completed && !cancelled && currentUpdater == this && operationId == newestOperation &&
           (reloadOnly != SettingsCache::instance().downloads().getDownloadSpoilersStatus()) &&
           targetPath == SettingsCache::instance().paths().getSpoilerCardDatabasePath();
}

void SpoilerBackgroundUpdater::cancelIfSettingsChanged()
{
    if (reloadOnly == SettingsCache::instance().downloads().getDownloadSpoilersStatus() ||
        targetPath != SettingsCache::instance().paths().getSpoilerCardDatabasePath()) {
        cancel();
    }
}

void SpoilerBackgroundUpdater::cancel()
{
    if (completed) {
        return;
    }
    cancelled = true;
    const auto reply = activeReply;
    activeReply.clear();
    QPointer<SpoilerBackgroundUpdater> guard(this);
    if (reply) {
        reply->abort();
        if (reply) {
            reply->deleteLater();
        }
    }
    // A running reload owns no updater state. Allow its completion to retire its
    // watcher before a heap-owned updater is released by the caller.
    if (guard && !reloading) {
        finish();
    }
}

void SpoilerBackgroundUpdater::downloadFromURL(const QUrl &url, bool saveResults)
{
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, QString("Cockatrice %1").arg(VERSION_STRING));
    request.setTransferTimeout(60000);
    activeReply = manager->get(request);
    if (saveResults) {
        connect(activeReply, &QNetworkReply::finished, this,
                &SpoilerBackgroundUpdater::actDownloadFinishedSpoilersFile);
    } else {
        connect(activeReply, &QNetworkReply::finished, this, &SpoilerBackgroundUpdater::actCheckIfSpoilerSeasonEnabled);
    }
}

void SpoilerBackgroundUpdater::actCheckIfSpoilerSeasonEnabled()
{
    auto *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply || reply != activeReply) {
        return;
    }
    activeReply.clear();
    reply->deleteLater();
    if (!isCurrent()) {
        finish();
        return;
    }
    if (reply->error() == QNetworkReply::NoError) {
        downloadFromURL(SPOILERS_URL, true);
    } else if (reply->error() == QNetworkReply::ContentNotFoundError) {
        const bool existed = QFileInfo::exists(targetPath);
        seasonEnded = existed;
        if (existed && !removeSpoilerFile(targetPath)) {
            finish(tr("The spoiler season has ended, but the old spoiler database could not be removed."));
            return;
        }
        if (existed || forceReload) {
            reloadDatabase(false);
        } else {
            finish();
        }
    } else {
        finish(tr("Could not check spoiler availability: %1").arg(reply->errorString()));
    }
}

void SpoilerBackgroundUpdater::actDownloadFinishedSpoilersFile()
{
    auto *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply || reply != activeReply) {
        return;
    }
    activeReply.clear();
    reply->deleteLater();
    if (!isCurrent()) {
        finish();
        return;
    }
    if (reply->error() != QNetworkReply::NoError) {
        finish(tr("Could not download spoilers: %1").arg(reply->errorString()));
        return;
    }
    spoilerData = reply->readAll();
    QString error;
    const auto result = SpoilerFile::save(targetPath, spoilerData, [this] { return isCurrent(); }, error);
    if (result == SpoilerFile::SaveResult::Failed) {
        finish(isCurrent() ? error : QString());
    } else if (result == SpoilerFile::SaveResult::Written || forceReload) {
        reloadDatabase(result == SpoilerFile::SaveResult::Written);
    } else {
        finish();
    }
}

void SpoilerBackgroundUpdater::reloadDatabase(bool updated)
{
    reloading = true;
    auto *database = CardDatabaseManager::getInstance();
    // The job outlives an individual request, so destroying that request cannot
    // let a newer updater overlap the worker that is still completing.
    auto *watcher = new QFutureWatcher<bool>(QCoreApplication::instance());
    activeReload = watcher;
    connect(watcher, &QFutureWatcher<bool>::finished, watcher, [watcher] {
        if (activeReload == watcher) {
            activeReload.clear();
        }
        watcher->deleteLater();
    });
    connect(watcher, &QFutureWatcher<bool>::finished, this, [this, watcher, database, updated] {
        bool loaded = false;
        try {
            loaded = watcher->result() && database->getLoadStatus() == Ok;
        } catch (const std::exception &error) {
            qCWarning(SpoilerBackgroundUpdaterLog) << "Spoiler database reload failed:" << error.what();
        }
        reloading = false;
        if (!isCurrent()) {
            finish();
        } else if (!loaded) {
            finish(reloadOnly
                       ? tr("Could not reload card data after disabling spoilers. Use Reload Card Database to retry.")
                       : tr("Could not reload card data after updating spoilers. Use Update Spoilers to retry the "
                            "reload."));
        } else {
            finish({}, updated);
        }
    });
    watcher->setFuture(QtConcurrent::run([database] {
        database->reloadCardDatabasesAndNotify();
        return true;
    }));
}

void SpoilerBackgroundUpdater::finish(const QString &error, bool updated)
{
    if (completed) {
        return;
    }
    completed = true;
    spoilerData = QByteArray();
    if (currentUpdater == this) {
        currentUpdater.clear();
    }
    QPointer<SpoilerBackgroundUpdater> guard(this);
    if (!error.isEmpty() && !cancelled) {
        qCWarning(SpoilerBackgroundUpdaterLog) << error;
        if (notificationIcon) {
            notificationIcon->showMessage(tr("Spoiler update failed"), error);
        }
        if (!guard) {
            return;
        }
        emit spoilerUpdateFailed(error);
    } else if (updated && !cancelled) {
        if (notificationIcon) {
            notificationIcon->showMessage(tr("Spoilers have been updated!"),
                                          tr("The card database now includes the downloaded spoilers."));
        }
        if (!guard) {
            return;
        }
        emit spoilersUpdatedSuccessfully();
    } else if (seasonEnded && !cancelled && notificationIcon) {
        notificationIcon->showMessage(tr("Spoiler season has ended"),
                                      tr("The spoiler database was removed. Please run Oracle to get released cards."));
    }
    if (guard) {
        emit spoilerCheckerDone();
    }
}

SpoilerBackgroundUpdater *SpoilerBackgroundUpdater::deleteSpoilerFile()
{
    // Deletion and the following reload share the operation lifetime. Callers can
    // observe either failure before the queued operation starts, and an explicit
    // retry still reloads when the file has already been removed.
    auto *updater = new SpoilerBackgroundUpdater(QCoreApplication::instance(), true);
    updater->reloadOnly = true;
    connect(updater, &SpoilerBackgroundUpdater::spoilerCheckerDone, updater, &QObject::deleteLater);
    return updater;
}
