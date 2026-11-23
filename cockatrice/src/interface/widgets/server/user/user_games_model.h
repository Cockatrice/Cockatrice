#ifndef COCKATRICE_USER_GAMES_MODEL_H
#define COCKATRICE_USER_GAMES_MODEL_H

#include <QAbstractListModel>
#include <QList>
#include <QPointer>
#include <QString>
#include <QTimer>
#include <libcockatrice/protocol/pb/serverinfo_game.pb.h>

class AbstractClient;
class Response;

/** Current user's games and the lifetime of the request that supplies them. */
class UserGamesModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum class State
    {
        Offline,
        Disconnected,
        Loading,
        Ready,
        Empty,
        Error
    };
    static constexpr int DefaultRequestTimeoutMs = 15000;
    static constexpr int GameRole = Qt::UserRole + 10;

    explicit UserGamesModel(AbstractClient *client,
                            int requestTimeoutMs = DefaultRequestTimeoutMs,
                            QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void setUser(const QString &name, bool isOnline);
    void refresh();
    State getState() const;
    bool canRefresh() const;

signals:
    void stateChanged();

private:
    void invalidateRequest();
    void replaceGames(QList<ServerInfo_Game> replacement);
    void finishRequest(const Response &response, quint64 request);
    void setState(State newState);

    QPointer<AbstractClient> client;
    QString userName;
    bool online = false;
    bool connected = false;
    State state = State::Offline;
    QList<ServerInfo_Game> games;
    quint64 requestGeneration = 0;
    QMetaObject::Connection responseConnection;
    QTimer timeout;
};

#endif // COCKATRICE_USER_GAMES_MODEL_H
