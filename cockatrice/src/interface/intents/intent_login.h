#ifndef COCKATRICE_INTENT_LOGIN_H
#define COCKATRICE_INTENT_LOGIN_H

#include "../../client/settings/cache_settings.h"
#include "contexts/context_connect_to_server.h"
#include "intent.h"
#include "remote_client.h"

class IntentGetLoginCredentials : public Intent
{
    Q_OBJECT

public:
    IntentGetLoginCredentials(RemoteClient *_remoteClient, ContextConnectToServer *_context)
        : Intent(), remoteClient(_remoteClient), context(_context)
    {
    }

protected:
    bool checkPrecondition() const override
    {
        ServersSettings &servers = SettingsCache::instance().servers();
        return servers.hasLoginData(context->hostname, context->port);
    }

    void onPreconditionSatisfied() override
    {
        ServersSettings &servers = SettingsCache::instance().servers();
        auto index = servers.findServerIndex(context->hostname, context->port);

        if (index >= 0) {
            context->username =
                servers.getValue(QString("username%1").arg(index), "server", "server_details").toString();
            context->password =
                servers.getValue(QString("password%1").arg(index), "server", "server_details").toString();
            emit finished();
            qWarning() << "Using saved credentials";
        } else {
            qWarning() << "No saved server entry";
        }
    }

    void onPreconditionNotSatisfied() override
    {
    }

private:
    RemoteClient *remoteClient;
    ContextConnectToServer *context;
};

#endif // COCKATRICE_INTENT_LOGIN_H
