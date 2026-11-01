#include "remote_public_decks_model.h"

#include "../../../client/settings/cache_settings.h"

#include <QMetaObject>
#include <QPointer>
#include <QTimer>
#include <QVariant>
#include <QtGlobal>
#include <algorithm>
#include <utility>
#include <google/protobuf/repeated_ptr_field.h>
#include <libcockatrice/network/client/abstract/abstract_client.h>
#include <libcockatrice/protocol/pb/command_deck_list_other_user.pb.h>
#include <libcockatrice/protocol/pb/response.pb.h>
#include <libcockatrice/protocol/pb/response_deck_list.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_deckstorage.pb.h>
#include <libcockatrice/protocol/pending_command.h>
#include <libcockatrice/settings/network_settings.h>
#include <string>

class QObject;

RemotePublicDecksModel::RemotePublicDecksModel(AbstractClient *_client, QObject *parent)
    : QAbstractListModel(parent), client(_client)
{
    // The ping sweep can drop a pending command without ever emitting finished,
    // so loading must not be a latch: time it out and clear it when the client
    // goes away, or the tab is stuck on the loading state for the session.
    loadingTimeoutTimer = new QTimer(this);
    loadingTimeoutTimer->setSingleShot(true);
    loadingTimeoutTimer->setInterval(
        static_cast<int>((static_cast<qint64>(SettingsCache::instance().network().getTimeOut()) + 1) *
                         SettingsCache::instance().network().getKeepAlive() * 1000));
    connect(loadingTimeoutTimer, &QTimer::timeout, this, &RemotePublicDecksModel::onLoadingTimeout);
    connect(client, &AbstractClient::statusChanged, this, [this](ClientStatus status) {
        if (status == StatusDisconnected || status == StatusDisconnecting) {
            // Retire the active request without discarding the last accepted
            // snapshot. A disconnect is a transport failure, not a user clear.
            const bool wasLoading = loading;
            const quint64 sequence = ++requestSequence;
            const QPointer<RemotePublicDecksModel> guard(this);
            loadingTimeoutTimer->stop();
            setLoading(false);
            if (guard && sequence == requestSequence && wasLoading) {
                emit loadFailed(tr("The connection was lost while loading public decks. Reconnect and try again."));
            }
        }
    });
    connect(client, &QObject::destroyed, this, &RemotePublicDecksModel::clear);
}

int RemotePublicDecksModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : visibleIndices.size();
}

QVariant RemotePublicDecksModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= visibleIndices.size()) {
        return QVariant();
    }
    if (role == Qt::DisplayRole || role == Qt::ToolTipRole) {
        return decks.at(visibleIndices.at(index.row())).name;
    }
    return QVariant();
}

RemotePublicDecksModel::DeckEntry RemotePublicDecksModel::entryAt(int row) const
{
    if (row < 0 || row >= visibleIndices.size()) {
        return DeckEntry{};
    }
    return decks.at(visibleIndices.at(row));
}

void RemotePublicDecksModel::setSearchText(const QString &text)
{
    const QString trimmed = text.trimmed();
    if (searchText == trimmed) {
        return;
    }
    searchText = trimmed;
    rebuildVisibleIndices();
}

void RemotePublicDecksModel::setColorFilter(VisualDeckStorageSortFilterProxyModel::FilterMode mode,
                                            const QSet<QChar> &colors)
{
    if (colorFilterMode == mode && activeColors == colors) {
        return;
    }
    colorFilterMode = mode;
    activeColors = colors;
    rebuildVisibleIndices();
}

void RemotePublicDecksModel::setTagFilter(const QSet<QString> &selected, const QSet<QString> &excluded)
{
    if (includedTags == selected && excludedTags == excluded) {
        return;
    }
    includedTags = selected;
    excludedTags = excluded;
    rebuildVisibleIndices();
}

QSet<QString> RemotePublicDecksModel::allTags() const
{
    QSet<QString> all;
    for (const DeckEntry &entry : decks) {
        all.unite(QSet<QString>(entry.tags.cbegin(), entry.tags.cend()));
    }
    return all;
}

QList<int> RemotePublicDecksModel::filteredIndices(const QList<DeckEntry> &entries) const
{
    QList<int> newIndices;
    newIndices.reserve(entries.size());
    for (int row = 0; row < entries.size(); ++row) {
        const DeckEntry &entry = entries.at(row);

        if (!searchText.isEmpty() && !entry.name.contains(searchText, Qt::CaseInsensitive)) {
            continue;
        }

        if (!activeColors.isEmpty()) {
            const QString &identity = entry.colorIdentity;
            if (!colorIdentityMatches(colorFilterMode, activeColors, identity)) {
                continue;
            }
        }

        if (!includedTags.isEmpty()) {
            const QSet<QString> entryTags(entry.tags.cbegin(), entry.tags.cend());
            bool hasAll = std::all_of(includedTags.begin(), includedTags.end(),
                                      [&entryTags](const QString &tag) { return entryTags.contains(tag); });
            if (!hasAll) {
                continue;
            }
        }

        if (!excludedTags.isEmpty() && std::any_of(excludedTags.begin(), excludedTags.end(),
                                                   [&entry](const QString &tag) { return entry.tags.contains(tag); })) {
            continue;
        }

        newIndices.append(row);
    }

    return newIndices;
}

void RemotePublicDecksModel::rebuildVisibleIndices()
{
    // A filter can be changed by a reset observer. Finish the current reset
    // before publishing another one, and evaluate the latest filter values.
    if (resetting) {
        QMetaObject::invokeMethod(this, &RemotePublicDecksModel::rebuildVisibleIndices, Qt::QueuedConnection);
        return;
    }
    const QPointer<RemotePublicDecksModel> guard(this);
    resetting = true;
    beginResetModel();
    if (!guard) {
        return;
    }
    visibleIndices = filteredIndices(decks);
    resetting = false;
    endResetModel();
}

void RemotePublicDecksModel::replaceDecks(QList<DeckEntry> entries, quint64 sequence, bool completeRequest)
{
    if (sequence != requestSequence) {
        return;
    }
    if (resetting) {
        QMetaObject::invokeMethod(this, [this, entries = std::move(entries), sequence, completeRequest]() mutable {
            replaceDecks(std::move(entries), sequence, completeRequest);
        }, Qt::QueuedConnection);
        return;
    }
    const QPointer<RemotePublicDecksModel> guard(this);
    resetting = true;
    // Views must still be able to read the old rows in modelAboutToBeReset.
    beginResetModel();
    if (!guard) {
        return;
    }
    if (sequence == requestSequence) {
        decks = std::move(entries);
        visibleIndices = filteredIndices(decks);
    }
    resetting = false;
    endResetModel();
    if (!guard || sequence != requestSequence) {
        return;
    }
    if (completeRequest) {
        finishRequest(sequence);
    }
}

void RemotePublicDecksModel::refresh(const QString &userName)
{
    if (!client || client->getStatus() == StatusDisconnected ||
        client->getStatus() == StatusDisconnecting) {
        const quint64 sequence = ++requestSequence;
        const QPointer<RemotePublicDecksModel> guard(this);
        loadingTimeoutTimer->stop();
        setLoading(false);
        if (guard && sequence == requestSequence) {
            emit loadFailed(tr("The server connection is unavailable. Reconnect and try again."));
        }
        return;
    }
    const QPointer<RemotePublicDecksModel> guard(this);
    const QString requestedUser = userName;
    const quint64 sequence = ++requestSequence;
    // Arm the timeout before notifying observers. A callback may clear the
    // model or start a newer request, which owns its own timer and response.
    loadingTimeoutTimer->start();
    setLoading(true);
    if (!guard || sequence != requestSequence || !client) {
        return;
    }
    Command_DeckListOtherUser cmd;
    cmd.set_user_name(requestedUser.toStdString());
    PendingCommand *pend = client->prepareSessionCommand(cmd);
    connect(pend, &PendingCommand::finished, this,
            [this, sequence](const Response &response, const CommandContainer &) {
                if (sequence == requestSequence && loading) {
                    decksReceived(response, sequence);
                }
            });
    client->sendCommand(pend);
}

void RemotePublicDecksModel::finishRequest(quint64 sequence, const QString &error)
{
    if (sequence != requestSequence) {
        return;
    }
    const QPointer<RemotePublicDecksModel> guard(this);
    const QString failure = error;
    const quint64 completion = ++requestSequence;
    loadingTimeoutTimer->stop();
    setLoading(false);
    // loadingChanged can close the tab or immediately retry. A completed
    // request must never stop that retry's timer or display its old failure.
    if (guard && completion == requestSequence && !failure.isEmpty()) {
        emit loadFailed(failure);
    }
}

void RemotePublicDecksModel::onLoadingTimeout()
{
    if (loading) {
        finishRequest(requestSequence, tr("The server did not respond in time. Try again."));
    }
}

void RemotePublicDecksModel::clear()
{
    const QPointer<RemotePublicDecksModel> guard(this);
    const quint64 sequence = ++requestSequence;
    loadingTimeoutTimer->stop();
    replaceDecks({}, sequence);
    if (guard && sequence == requestSequence) {
        setLoading(false);
    }
}

void RemotePublicDecksModel::setLoading(bool value)
{
    if (loading == value) {
        return;
    }
    loading = value;
    emit loadingChanged(loading);
}

void RemotePublicDecksModel::decksReceived(const Response &response, quint64 sequence)
{
    // The current reply has arrived. Publishing its snapshot may wait for an
    // active reset to finish, but that delay must not become a network timeout.
    loadingTimeoutTimer->stop();
    if (response.response_code() != Response::RespOk) {
        finishRequest(sequence, tr("Failed to load the user's public decks (server response code %1).")
                                    .arg(QString::number(static_cast<int>(response.response_code()))));
        return;
    }
    if (!response.HasExtension(Response_DeckList::ext)) {
        finishRequest(sequence, tr("The server returned an invalid public deck list."));
        return;
    }

    const Response_DeckList &resp = response.GetExtension(Response_DeckList::ext);
    replaceDecks(entriesFromFolder(resp.root()), sequence, true);
}

QList<RemotePublicDecksModel::DeckEntry>
RemotePublicDecksModel::entriesFromFolder(const ServerInfo_DeckStorage_Folder &folder)
{
    QList<DeckEntry> entries;
    // Traverse without recursive calls: deeply nested server folders should
    // consume heap storage rather than exhaust the client's call stack.
    QList<const ServerInfo_DeckStorage_TreeItem *> pending;
    const auto appendChildren = [&pending](const ServerInfo_DeckStorage_Folder &parent) {
        for (int i = parent.items_size() - 1; i >= 0; --i) {
            pending.append(&parent.items(i));
        }
    };
    appendChildren(folder);
    while (!pending.isEmpty()) {
        const auto *item = pending.takeLast();
        if (item->has_folder()) {
            appendChildren(item->folder());
            continue;
        }
        if (!item->has_file()) {
            continue;
        }
        const ServerInfo_DeckStorage_File &file = item->file();
        DeckEntry entry;
        entry.id = item->id();
        entry.name = QString::fromStdString(item->name());
        entry.uploadTime = QDateTime::fromSecsSinceEpoch(file.creation_time());
        entry.bannerCardName = QString::fromStdString(file.banner_card_name());
        entry.bannerCardProvider = QString::fromStdString(file.banner_card_provider());
        entry.colorIdentity = QString::fromStdString(file.color_identity());
        for (const auto &tag : file.tags()) {
            entry.tags.append(QString::fromStdString(tag));
        }
        entries.append(std::move(entry));
    }
    return entries;
}
