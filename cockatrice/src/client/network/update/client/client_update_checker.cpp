#include "client_update_checker.h"

#include "release_channel.h"

#include <libcockatrice/settings/cache_settings.h>

ClientUpdateChecker::ClientUpdateChecker(QObject *parent) : QObject(parent)
{
}

void ClientUpdateChecker::check()
{
    auto releaseChannel = SettingsCache::instance().getUpdateReleaseChannel();

    finishedCheckConnection =
        connect(releaseChannel, &ReleaseChannel::finishedCheck, this, &ClientUpdateChecker::actFinishedCheck);
    errorConnection = connect(releaseChannel, &ReleaseChannel::error, this, &ClientUpdateChecker::actError);

    releaseChannel->checkForUpdates();
}

void ClientUpdateChecker::actFinishedCheck(bool needToUpdate, bool isCompatible, Release *release)
{
    disconnect(finishedCheckConnection);
    disconnect(errorConnection);

    emit finishedCheck(needToUpdate, isCompatible, release);
}

void ClientUpdateChecker::actError(const QString &errorString)
{
    disconnect(finishedCheckConnection);
    disconnect(errorConnection);

    emit error(errorString);
}