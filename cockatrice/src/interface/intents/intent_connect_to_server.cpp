#include "intent_connect_to_server.h"

#include "../../client/network/connection_controller/remote_connection_controller.h"
#include "../../client/settings/cache_settings.h"
#include "../widgets/dialogs/dlg_connect.h"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(IntentConnectLog, "intent.connect")

IntentConnectToServer::IntentConnectToServer(const ContextConnectToServer &ctx,
                                             ConnectionController *controller,
                                             QWidget *dialogParent,
                                             QObject *parent)
    : Intent(parent), ctx(ctx), controller(controller), dialogParent(dialogParent)
{
}

void IntentConnectToServer::doExecute()
{
    // 1. Try saved credentials for this hostname:port.
    if (auto creds = SettingsCache::instance().servers().findSavedCredsByHostPort(ctx.hostname, ctx.port);
        creds && !creds->password.isEmpty()) {
        qCDebug(IntentConnectLog) << "Using saved credentials for" << ctx.hostname << ":" << ctx.port;
        controller->connectToServerDirect(ctx.hostname, ctx.port, creds->playerName, creds->password);
        emitFinished(true);
        return;
    }

    // 2. No saved match (or password not saved) — open DlgConnect pre-filled.
    qCDebug(IntentConnectLog) << "No saved credentials for" << ctx.hostname << ":" << ctx.port
                              << "— opening Connect dialog";
    auto *dlg = new DlgConnect(dialogParent);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setWindowModality(Qt::ApplicationModal);
    dlg->prefillNewHost(ctx.hostname, QString::number(ctx.port));

    connect(dlg, &QDialog::accepted, this, [this, dlg]() {
        controller->connectToServerDirect(dlg->getHost(), static_cast<unsigned int>(dlg->getPort()),
                                          dlg->getPlayerName(), dlg->getPassword());
        emitFinished(true);
    });
    connect(dlg, &QDialog::rejected, this, [this]() {
        qCInfo(IntentConnectLog) << "User cancelled Connect dialog; aborting intent chain";
        emitFinished(false);
    });

    dlg->show();
}
