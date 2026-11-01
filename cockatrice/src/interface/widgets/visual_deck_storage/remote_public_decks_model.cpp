#include "remote_public_decks_model.h"

#include "../../../client/settings/cache_settings.h"

#include <QTimer>
#include <QVariant>
#include <QtGlobal>
#include <algorithm>
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
    // A transport can drop a pending command without emitting finished. Each
    // request therefore owns a sequence-guarded single-shot timeout below.
    connect(client, &AbstractClient::statusChanged, this, [this](ClientStatus status) {
        if (status == StatusDisconnected || status == StatusDisconnecting) {
            const bool wasLoading = loading;
            ++requestSequence;
            setLoading(false);
            if (wasLoading) {
                emit loadFailed(tr("The connection was lost while loading public decks. Try again."));
            }
        }
    });
    connect(client, &QObject::destroyed, this, [this]() {
        ++requestSequence;
        setLoading(false);
        client = nullptr;
    });
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
    searchText = text.trimmed();
    rebuildVisibleIndices();
}

void RemotePublicDecksModel::setColorFilter(VisualDeckStorageSortFilterProxyModel::FilterMode mode,
                                            const QSet<QChar> &colors)
{
    colorFilterMode = mode;
    activeColors = colors;
    rebuildVisibleIndices();
}

void RemotePublicDecksModel::setTagFilter(const QSet<QString> &selected, const QSet<QString> &excluded)
{
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

void RemotePublicDecksModel::rebuildVisibleIndices()
{
    QList<int> newIndices;
    newIndices.reserve(decks.size());
    for (int row = 0; row < decks.size(); ++row) {
        const DeckEntry &entry = decks.at(row);

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

    beginResetModel();
    visibleIndices = newIndices;
    endResetModel();
}

void RemotePublicDecksModel::refresh(const QString &userName)
{
    if (!client || client->getStatus() == StatusDisconnected ||
        client->getStatus() == StatusDisconnecting) {
        ++requestSequence;
        setLoading(false);
        emit loadFailed(tr("The server connection is unavailable. Reconnect and try again."));
        return;
    }

    // A new refresh supersedes any request still in flight. Its sequence owns
    // the timer and result; older commands may finish later but cannot replace
    // the current snapshot or change its loading state.
    const int seq = ++requestSequence;
    setLoading(true);
    const int timeoutMs = static_cast<int>(
        (static_cast<qint64>(SettingsCache::instance().network().getTimeOut()) + 1) *
        SettingsCache::instance().network().getKeepAlive() * 1000);
    QTimer::singleShot(timeoutMs, this, [this, seq]() {
        if (seq == requestSequence) {
            onLoadingTimeout();
        }
    });
    Command_DeckListOtherUser cmd;
    cmd.set_user_name(userName.toStdString());
    PendingCommand *pend = client->prepareSessionCommand(cmd);
    connect(pend, &PendingCommand::finished, this,
            [this, seq](const Response &response, const CommandContainer &commandContainer) {
                if (seq != requestSequence) {
                    return; // a newer refresh superseded this one
                }
                decksReceived(seq, response, commandContainer);
            });
    client->sendCommand(pend);
}

void RemotePublicDecksModel::onLoadingTimeout()
{
    // Invalidate the timed-out command before exposing the retry state. Some
    // transports can still deliver its completion after the timeout.
    ++requestSequence;
    setLoading(false);
    emit loadFailed(tr("The server did not respond in time. Try again."));
}

void RemotePublicDecksModel::clear()
{
    // Clearing is a new user intent: a late reply must not repopulate the model.
    ++requestSequence;
    setLoading(false);
    beginResetModel();
    decks.clear();
    visibleIndices.clear();
    endResetModel();
}

void RemotePublicDecksModel::setLoading(bool value)
{
    if (loading == value) {
        return;
    }
    loading = value;
    emit loadingChanged(loading);
}

void RemotePublicDecksModel::decksReceived(int seq, const Response &response, const CommandContainer & /*commandContainer*/)
{
    if (seq != requestSequence) {
        return;
    }
    // Invalidate this request's timeout before changing the loading state.
    ++requestSequence;
    setLoading(false);
    if (response.response_code() != Response::RespOk) {
        emit loadFailed(tr("Failed to load the user's public decks (server response code %1).")
                            .arg(QString::number(static_cast<int>(response.response_code()))));
        return;
    }

    const Response_DeckList &resp = response.GetExtension(Response_DeckList::ext);
    QList<DeckEntry> replacement;
    addFolder(resp.root(), replacement);

    QList<int> replacementIndices;
    replacementIndices.reserve(replacement.size());
    for (int row = 0; row < replacement.size(); ++row) {
        const DeckEntry &entry = replacement.at(row);
        if (!searchText.isEmpty() && !entry.name.contains(searchText, Qt::CaseInsensitive)) {
            continue;
        }
        if (!activeColors.isEmpty() &&
            !colorIdentityMatches(colorFilterMode, activeColors, entry.colorIdentity)) {
            continue;
        }
        const QSet<QString> entryTags(entry.tags.cbegin(), entry.tags.cend());
        if (!includedTags.isEmpty() &&
            !std::all_of(includedTags.cbegin(), includedTags.cend(),
                         [&entryTags](const QString &tag) { return entryTags.contains(tag); })) {
            continue;
        }
        if (!excludedTags.isEmpty() &&
            std::any_of(excludedTags.cbegin(), excludedTags.cend(),
                        [&entry](const QString &tag) { return entry.tags.contains(tag); })) {
            continue;
        }
        replacementIndices.append(row);
    }

    // Publish the complete new snapshot and its filtered index atomically.
    beginResetModel();
    decks = std::move(replacement);
    visibleIndices = std::move(replacementIndices);
    endResetModel();
}

void RemotePublicDecksModel::addFolder(const ServerInfo_DeckStorage_Folder &folder, QList<DeckEntry> &target)
{
    const int itemCount = folder.items_size();
    for (int i = 0; i < itemCount; ++i) {
        addTreeItem(folder.items(i), target);
    }
}

void RemotePublicDecksModel::addTreeItem(const ServerInfo_DeckStorage_TreeItem &item, QList<DeckEntry> &target)
{
    if (item.has_folder()) {
        addFolder(item.folder(), target);
        return;
    }

    const ServerInfo_DeckStorage_File &file = item.file();
    DeckEntry entry;
    entry.id = item.id();
    entry.name = QString::fromStdString(item.name());
    entry.uploadTime = QDateTime::fromSecsSinceEpoch(file.creation_time());
    entry.bannerCardName = QString::fromStdString(file.banner_card_name());
    entry.bannerCardProvider = QString::fromStdString(file.banner_card_provider());
    entry.colorIdentity = QString::fromStdString(file.color_identity());
    QStringList tags;
    for (const auto &tag : file.tags()) {
        tags.append(QString::fromStdString(tag));
    }
    entry.tags = tags;
    target.append(std::move(entry));
}
