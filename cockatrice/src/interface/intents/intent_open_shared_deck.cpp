#include "intent_open_shared_deck.h"

#include "../deck_loader/deck_loader.h"
#include "../widgets/dialogs/dlg_shared_decks_preview.h"
#include "../widgets/tabs/tab_supervisor.h"
#include "intent_connect_to_server.h"

#include <QMessageBox>
#include <QTimer>
#include <libcockatrice/card/database/card_database_querier.h>
#include <libcockatrice/protocol/pb/command_deck_share_download.pb.h>
#include <libcockatrice/protocol/pb/command_deck_share_list.pb.h>
#include <libcockatrice/protocol/pb/response.pb.h>
#include <libcockatrice/protocol/pb/response_deck_share_download.pb.h>
#include <libcockatrice/protocol/pb/response_deck_share_list.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_deck_share_item.pb.h>
#include <libcockatrice/protocol/pending_command.h>

IntentOpenSharedDeck::IntentOpenSharedDeck(TabSupervisor *_tabSupervisor,
                                           RemoteClient *_remoteClient,
                                           const CardDatabaseQuerier *_querier,
                                           std::unique_ptr<ContextOpenDeck> _context)
    : Intent(), tabSupervisor(_tabSupervisor), remoteClient(_remoteClient), querier(_querier),
      context(_context.release())
{
    downloadTimer = new QTimer(this);
    downloadTimer->setSingleShot(true);
    downloadTimer->setInterval(15000);
    connect(downloadTimer, &QTimer::timeout, this, &IntentOpenSharedDeck::onDownloadTimeout);
}

bool IntentOpenSharedDeck::checkPrecondition() const
{
    if (remoteClient->getStatus() != ClientStatus::StatusLoggedIn) {
        return false;
    }
    // serverName() reflects the server the client was configured to connect to,
    // which may differ from the actual TCP peer (e.g. when connecting through a
    // proxy), so only the hostname is compared here.
    return remoteClient->serverName().compare(context->serverContext.hostname, Qt::CaseInsensitive) == 0;
}

void IntentOpenSharedDeck::onPreconditionSatisfied()
{
    // Resolve the share token to its items first; a share can contain more than
    // one deck, and each item is downloaded by id.
    Command_DeckShareList cmd;
    cmd.set_token(context->shareToken.toStdString());

    PendingCommand *pend = AbstractClient::prepareSessionCommand(cmd);
    connect(pend, &PendingCommand::finished, this, &IntentOpenSharedDeck::listShareFinished);
    remoteClient->sendCommand(pend);
}

void IntentOpenSharedDeck::onPreconditionNotSatisfied()
{
    runDependency(new IntentConnectToServer(remoteClient, &context->serverContext));
}

void IntentOpenSharedDeck::listShareFinished(const Response &response, const CommandContainer & /* commandContainer */)
{
    if (response.response_code() != Response::RespOk) {
        emitFailed(tr("The shared deck could not be found or has expired"));
        return;
    }

    const Response_DeckShareList &resp = response.GetExtension(Response_DeckShareList::ext);
    if (resp.items_size() == 0) {
        emitFailed(tr("The shared deck is empty"));
        return;
    }

    QList<ServerInfo_DeckShareItem> items;
    items.reserve(resp.items_size());
    for (const ServerInfo_DeckShareItem &item : resp.items()) {
        items.append(item);
        itemNames.insert(item.id(), QString::fromStdString(item.name()));
    }

    const QString serverText = context->serverContext.hostname + ":" + context->serverContext.port;

    // Ask the user which decks to open before downloading anything.
    previewDialog = new DlgSharedDecksPreview(tabSupervisor, querier, QString::fromStdString(resp.name()),
                                              resp.expires_at(), serverText, items);
    connect(previewDialog, &DlgSharedDecksPreview::openRequested, this, &IntentOpenSharedDeck::startDownloads);
    connect(previewDialog, &DlgSharedDecksPreview::cancelled, this, &IntentOpenSharedDeck::emitCancelled);
    connect(previewDialog, &DlgSharedDecksPreview::cancelled, previewDialog, &QWidget::deleteLater);
    previewDialog->show();
    previewDialog->raise();
    previewDialog->activateWindow();
}

void IntentOpenSharedDeck::startDownloads(const QList<int> &itemIds)
{
    pendingItemIds = itemIds;
    totalItems = itemIds.size();
    completedItems = 0;
    loadedDecks.clear();
    downloadNextItem();
}

void IntentOpenSharedDeck::downloadNextItem()
{
    if (pendingItemIds.isEmpty()) {
        finishAll();
        return;
    }

    currentItemId = pendingItemIds.takeFirst();
    downloadTimer->start();

    Command_DeckShareDownload cmd;
    cmd.set_token(context->shareToken.toStdString());
    cmd.set_item_id(currentItemId);

    PendingCommand *pend = AbstractClient::prepareSessionCommand(cmd);
    connect(pend, &PendingCommand::finished, this, &IntentOpenSharedDeck::downloadShareFinished);
    remoteClient->sendCommand(pend);
}

void IntentOpenSharedDeck::downloadShareFinished(const Response &response,
                                                 const CommandContainer & /* commandContainer */)
{
    downloadTimer->stop();

    QString failureReason;
    if (response.response_code() != Response::RespOk) {
        failureReason = tr("Failed to download the shared deck");
    } else {
        const Response_DeckShareDownload &resp = response.GetExtension(Response_DeckShareDownload::ext);
        const QString deckString = QString::fromStdString(resp.deck());
        if (deckString.isEmpty()) {
            failureReason = tr("The shared deck is empty");
        } else {
            std::optional<LoadedDeck> deckOpt =
                DeckLoader::loadFromRemote(deckString, LoadedDeck::LoadInfo::NON_REMOTE_ID);
            if (!deckOpt) {
                failureReason = tr("The shared deck could not be loaded");
            } else {
                loadedDecks.append(deckOpt.value());
                ++completedItems;
                previewDialog->setDownloadProgress(completedItems, totalItems,
                                                   itemNames.value(currentItemId, tr("Unknown deck")));
                downloadNextItem();
                return;
            }
        }
    }

    onItemFailure(failureReason);
}

void IntentOpenSharedDeck::onItemFailure(const QString &reason)
{
    downloadTimer->stop();

    if (loadedDecks.isEmpty()) {
        previewDialog->deleteLater();
        emitFailed(reason);
        return;
    }

    const int downloadedCount = loadedDecks.size();
    const QMessageBox::StandardButton answer = QMessageBox::question(
        previewDialog, tr("Open shared decks"),
        tr("Could not download the deck \"%1\".\n\n%n deck(s) were already downloaded. Open them?", "", downloadedCount)
            .arg(itemNames.value(currentItemId, tr("Unknown deck"))),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

    if (answer == QMessageBox::Yes) {
        finishAll();
    } else {
        previewDialog->deleteLater();
        emitCancelled();
    }
}

void IntentOpenSharedDeck::onDownloadTimeout()
{
    onItemFailure(tr("Timed out while downloading the shared deck"));
}

void IntentOpenSharedDeck::finishAll()
{
    previewDialog->deleteLater();
    for (const LoadedDeck &deck : loadedDecks) {
        tabSupervisor->openDeckInNewTab(deck);
    }
    emitFinished();
}