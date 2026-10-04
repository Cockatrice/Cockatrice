#ifndef COCKATRICE_COMMANDER_SPELLBOOK_API_ACCESSOR_H
#define COCKATRICE_COMMANDER_SPELLBOOK_API_ACCESSOR_H

#include <QNetworkAccessManager>
#include <QObject>
#include <QString>
#include <QtClassHelperMacros>
#include <qtmetamacros.h>
#include <qtypes.h>

class DeckList;
class QNetworkReply;
struct EstimateBracketResult;

class CommanderSpellbookApiAccessor final : public QObject
{
    Q_OBJECT

public:
    static CommanderSpellbookApiAccessor &instance();

    using RequestId = quint64;

    RequestId estimateBracket(const DeckList &deck, QObject *requester);

signals:
    void estimateBracketFinished(RequestId id, QObject *requester, const EstimateBracketResult &result);

    void estimateBracketError(RequestId id, QObject *requester, const QString &errorMessage);

private:
    explicit CommanderSpellbookApiAccessor(QObject *parent = nullptr);
    Q_DISABLE_COPY_MOVE(CommanderSpellbookApiAccessor)

    void onEstimateReplyFinished(QNetworkReply *reply);

    QNetworkAccessManager network;
    RequestId nextRequestId = 1;
};

#endif // COCKATRICE_COMMANDER_SPELLBOOK_API_ACCESSOR_H
