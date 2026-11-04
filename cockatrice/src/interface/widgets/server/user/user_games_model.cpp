#include "user_games_model.h"

#include <QVariant>
#include <libcockatrice/network/client/abstract/abstract_client.h>
#include <libcockatrice/protocol/pb/response.pb.h>
#include <libcockatrice/protocol/pb/response_get_games_of_user.pb.h>
#include <libcockatrice/protocol/pb/session_commands.pb.h>
#include <libcockatrice/protocol/pending_command.h>
#include <utility>

UserGamesModel::UserGamesModel(AbstractClient *_client, int requestTimeoutMs, QObject *parent)
    : QAbstractListModel(parent), client(_client)
{
    timeout.setSingleShot(true);
    timeout.setInterval(requestTimeoutMs);
    connect(&timeout, &QTimer::timeout, this, [this] {
        invalidateRequest();
        setState(State::Error);
    });
    connected = client->getStatus() == StatusLoggedIn;
    connect(client, &AbstractClient::statusChanged, this, [this](ClientStatus status) {
        // AbstractClient::setStatus emits this signal while holding the (non-recursive)
        // status mutex, which getStatus() would try to lock again. Track the emitted
        // value here, and read this cache from canRefresh() for the same reason.
        connected = status == StatusLoggedIn;
        if (!connected) {
            invalidateRequest();
            replaceGames({});
            setState(online ? State::Disconnected : State::Offline);
        } else if (online) {
            refresh();
        }
    });
    connect(client, &QObject::destroyed, this, [this] {
        client = nullptr;
        connected = false;
        invalidateRequest();
        replaceGames({});
        setState(online ? State::Disconnected : State::Offline);
    });
}

int UserGamesModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : games.size();
}

QVariant UserGamesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.model() != this || index.column() != 0 || index.row() < 0 ||
        index.row() >= games.size() || role != GameRole) {
        return {};
    }
    return QVariant::fromValue(games.at(index.row()));
}

void UserGamesModel::setUser(const QString &name, bool isOnline)
{
    userName = name;
    online = isOnline;
    refresh();
}

UserGamesModel::State UserGamesModel::getState() const
{
    return state;
}

bool UserGamesModel::canRefresh() const
{
    return online && !userName.isEmpty() && client && connected && state != State::Loading;
}

void UserGamesModel::invalidateRequest()
{
    ++requestGeneration;
    // The client owns pending commands. Detach our interest without deleting
    // an object that may already have moved to the client's thread.
    disconnect(responseConnection);
    responseConnection = {};
    timeout.stop();
}

void UserGamesModel::replaceGames(QList<ServerInfo_Game> replacement)
{
    beginResetModel();
    games = std::move(replacement);
    endResetModel();
}

void UserGamesModel::setState(State newState)
{
    state = newState;
    emit stateChanged();
}

void UserGamesModel::refresh()
{
    invalidateRequest();
    const quint64 request = requestGeneration;
    replaceGames({});
    if (!online || userName.isEmpty()) {
        setState(State::Offline);
        return;
    }
    if (!client || !connected) {
        setState(State::Disconnected);
        return;
    }

    setState(State::Loading);
    // State listeners may select another user or disconnect the client.
    if (request != requestGeneration) {
        return;
    }

    Command_GetGamesOfUser command;
    command.set_user_name(userName.toStdString());
    PendingCommand *pending = client->prepareSessionCommand(command);
    responseConnection = connect(pending, &PendingCommand::finished, this,
                                 [this, request](const Response &response) { finishRequest(response, request); });
    timeout.start();
    client->sendCommand(pending);
}

void UserGamesModel::finishRequest(const Response &response, quint64 request)
{
    if (request != requestGeneration) {
        return;
    }
    invalidateRequest();
    const quint64 completion = requestGeneration;
    if (response.response_code() != Response::RespOk || !response.HasExtension(Response_GetGamesOfUser::ext)) {
        setState(State::Error);
        return;
    }

    const auto &result = response.GetExtension(Response_GetGamesOfUser::ext);
    QList<ServerInfo_Game> replacement;
    replacement.reserve(result.game_list_size());
    for (const auto &game : result.game_list()) {
        replacement.append(game);
    }
    replaceGames(std::move(replacement));
    if (completion != requestGeneration) {
        return;
    }
    setState(games.isEmpty() ? State::Empty : State::Ready);
}
