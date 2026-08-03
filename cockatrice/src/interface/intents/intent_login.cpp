#include "intent_login.h"

#include "../../client/settings/cache_settings.h"
#include "libcockatrice/settings/servers_settings.h"

IntentGetLoginCredentials::IntentGetLoginCredentials(ContextConnectToServer *_context) : Intent(), context(_context)
{
}

bool IntentGetLoginCredentials::checkPrecondition() const
{
    ServersSettings &servers = SettingsCache::instance().servers();
    return servers.hasLoginData(context->hostname, context->port);
}

void IntentGetLoginCredentials::onPreconditionSatisfied()
{
    ServersSettings &servers = SettingsCache::instance().servers();
    const int index = servers.findServerIndex(context->hostname, context->port);

    if (index >= 0) {
        context->username = servers.getValue(QString("username%1").arg(index), "server", "server_details").toString();
        context->password = servers.getValue(QString("password%1").arg(index), "server", "server_details").toString();
        emit finished();
    } else {
        emit failed(tr("No saved credentials for this server"));
    }
}

void IntentGetLoginCredentials::onPreconditionNotSatisfied()
{
    emit failed(tr("No saved credentials for this server"));
}
