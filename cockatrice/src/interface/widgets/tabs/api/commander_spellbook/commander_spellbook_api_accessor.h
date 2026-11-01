#ifndef COCKATRICE_COMMANDER_SPELLBOOK_API_ACCESSOR_H
#define COCKATRICE_COMMANDER_SPELLBOOK_API_ACCESSOR_H

#include <QHash>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QtGlobal>
#include <qtmetamacros.h>

class DeckList;
class QNetworkReply;
class QNetworkAccessManager;
struct EstimateBracketResult;

class CommanderSpellbookApiAccessor final : public QObject
{
    Q_OBJECT

public:
    static CommanderSpellbookApiAccessor &instance();

    using RequestId = quint64;

    RequestId estimateBracket(const DeckList &deck, QObject *requester);
    void cancelEstimate(RequestId id);

signals:
    void estimateBracketFinished(RequestId id, QObject *requester, const EstimateBracketResult &result);

    void estimateBracketError(RequestId id, QObject *requester, const QString &errorMessage);

private:
    explicit CommanderSpellbookApiAccessor(QObject *parent = nullptr);
    Q_DISABLE_COPY_MOVE(CommanderSpellbookApiAccessor)

    void onEstimateReplyFinished(QNetworkReply *reply, RequestId id, QObject *requester);

    QNetworkAccessManager *network;
    QHash<RequestId, QPointer<QNetworkReply>> pendingRequests;
    RequestId nextRequestId = 1;
};

#endif // COCKATRICE_COMMANDER_SPELLBOOK_API_ACCESSOR_H
