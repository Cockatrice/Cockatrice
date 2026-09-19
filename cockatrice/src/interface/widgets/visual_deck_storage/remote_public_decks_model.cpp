#include "remote_public_decks_model.h"

#include "../../../client/settings/cache_settings.h"

#include <QTimer>
#include <algorithm>
#include <libcockatrice/network/client/abstract/abstract_client.h>
#include <libcockatrice/protocol/pb/command_deck_list_other_user.pb.h>
#include <libcockatrice/protocol/pb/response.pb.h>
#include <libcockatrice/protocol/pb/response_deck_list.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_deckstorage.pb.h>
#include <libcockatrice/protocol/pending_command.h>
#include <libcockatrice/settings/network_settings.h>

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
        if (status == StatusDisconnected) {
            loadingTimeoutTimer->stop();
            setLoading(false);
        }
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
    if (loading) {
        return;
    }
    setLoading(true);
    loadingTimeoutTimer->start();
    Command_DeckListOtherUser cmd;
    cmd.set_user_name(userName.toStdString());
    PendingCommand *pend = client->prepareSessionCommand(cmd);
    connect(pend, &PendingCommand::finished, this, &RemotePublicDecksModel::decksReceived);
    client->sendCommand(pend);
}

void RemotePublicDecksModel::onLoadingTimeout()
{
    setLoading(false);
    emit loadFailed(tr("The server did not respond in time. Try again."));
}

void RemotePublicDecksModel::clear()
{
    decks.clear();
    rebuildVisibleIndices();
}

void RemotePublicDecksModel::setLoading(bool value)
{
    if (loading == value) {
        return;
    }
    loading = value;
    emit loadingChanged(loading);
}

void RemotePublicDecksModel::decksReceived(const Response &response, const CommandContainer & /*commandContainer*/)
{
    setLoading(false);
    loadingTimeoutTimer->stop();
    if (response.response_code() != Response::RespOk) {
        emit loadFailed(tr("Failed to load the user's public decks (server response code %1).")
                            .arg(QString::number(static_cast<int>(response.response_code()))));
        return;
    }

    const Response_DeckList &resp = response.GetExtension(Response_DeckList::ext);
    decks.clear();
    addFolder(resp.root());
    rebuildVisibleIndices();
}

void RemotePublicDecksModel::addFolder(const ServerInfo_DeckStorage_Folder &folder)
{
    const int itemCount = folder.items_size();
    for (int i = 0; i < itemCount; ++i) {
        addTreeItem(folder.items(i));
    }
}

void RemotePublicDecksModel::addTreeItem(const ServerInfo_DeckStorage_TreeItem &item)
{
    if (item.has_folder()) {
        addFolder(item.folder());
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
    decks.append(entry);
}
