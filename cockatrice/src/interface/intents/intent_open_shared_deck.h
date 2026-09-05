#ifndef COCKATRICE_INTENT_OPEN_SHARED_DECK_H
#define COCKATRICE_INTENT_OPEN_SHARED_DECK_H

#include "contexts/context_open_deck.h"
#include "intent.h"
#include "remote_client.h"

#include <QList>
#include <QMap>
#include <QScopedPointer>
#include <memory>

class TabSupervisor;
struct LoadedDeck;
class CardDatabaseQuerier;
class DlgSharedDecksPreview;
class QTimer;

class IntentOpenSharedDeck : public Intent
{
    Q_OBJECT

public:
    IntentOpenSharedDeck(TabSupervisor *_tabSupervisor,
                         RemoteClient *_remoteClient,
                         const CardDatabaseQuerier *_querier,
                         std::unique_ptr<ContextOpenDeck> _context);

protected:
    bool checkPrecondition() const override;
    void onPreconditionSatisfied() override;
    void onPreconditionNotSatisfied() override;

private slots:
    void listShareFinished(const Response &response, const CommandContainer &commandContainer);
    void downloadShareFinished(const Response &response, const CommandContainer &commandContainer);
    void onDownloadTimeout();

private:
    void startDownloads(const QList<int> &itemIds);
    void downloadNextItem();
    void onItemFailure(const QString &reason);
    void finishAll();

    TabSupervisor *tabSupervisor;
    RemoteClient *remoteClient;
    const CardDatabaseQuerier *querier;
    QScopedPointer<ContextOpenDeck> context;
    DlgSharedDecksPreview *previewDialog = nullptr;
    QTimer *downloadTimer;
    QMap<int, QString> itemNames;
    QList<int> pendingItemIds;
    QList<LoadedDeck> loadedDecks;
    int currentItemId = 0;
    int totalItems = 0;
    int completedItems = 0;
};

#endif // COCKATRICE_INTENT_OPEN_SHARED_DECK_H