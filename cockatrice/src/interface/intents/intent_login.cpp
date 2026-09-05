#include "intent_login.h"

#include "../../client/settings/cache_settings.h"
#include "../widgets/dialogs/dlg_login_prompt.h"
#include "libcockatrice/settings/servers_settings.h"

#include <QDialog>

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
        emitFinished();
    } else {
        emitFailed(tr("No saved credentials for this server"));
    }
}

void IntentGetLoginCredentials::onPreconditionNotSatisfied()
{
    // No credentials saved for the target server: ask the user for them. They
    // opt into saving them so later links to the same server connect directly.
    const QString serverText = context->hostname + ":" + context->port;
    DlgLoginPrompt dialog(serverText);
    // ApplicationModal: the dialog has no parent (the intent is not a widget),
    // so WindowModal would not actually block any other window.
    dialog.setWindowModality(Qt::ApplicationModal);

    if (dialog.exec() != QDialog::Accepted) {
        emitCancelled();
        return;
    }

    context->username = dialog.username();
    context->password = dialog.password();

    if (dialog.savePassword() && !context->username.isEmpty()) {
        ServersSettings &servers = SettingsCache::instance().servers();
        servers.addNewServer(context->hostname, context->hostname, context->port, context->username, context->password,
                             true);
    }

    emitFinished();
}
