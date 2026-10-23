#include "client_update_checker.h"

#include "../../../settings/cache_settings.h"
#include "release_channel.h"

ClientUpdateChecker::ClientUpdateChecker(QObject *parent) : QObject(parent)
{
}

void ClientUpdateChecker::check()
{
    auto releaseChannel = SettingsCache::instance().getUpdateReleaseChannel();

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    connect(releaseChannel, &ReleaseChannel::finishedCheck, this, &ClientUpdateChecker::actFinishedCheck,
            Qt::SingleShotConnection);
    connect(releaseChannel, &ReleaseChannel::error, this, &ClientUpdateChecker::actError, Qt::SingleShotConnection);
#else
    finishedCheckConnection =
        connect(releaseChannel, &ReleaseChannel::finishedCheck, this, &ClientUpdateChecker::actFinishedCheck);
    errorConnection = connect(releaseChannel, &ReleaseChannel::error, this, &ClientUpdateChecker::actError);
#endif

    releaseChannel->checkForUpdates();
}

void ClientUpdateChecker::actFinishedCheck(bool needToUpdate, bool isCompatible, Release *release)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 5, 0)
    disconnect(finishedCheckConnection);
    disconnect(errorConnection);
#endif
    emit finishedCheck(needToUpdate, isCompatible, release);
}

void ClientUpdateChecker::actError(const QString &errorString)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 5, 0)
    disconnect(finishedCheckConnection);
    disconnect(errorConnection);
#endif
    emit error(errorString);
}