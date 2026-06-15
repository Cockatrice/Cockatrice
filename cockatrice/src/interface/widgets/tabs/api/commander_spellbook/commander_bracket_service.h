#ifndef COCKATRICE_COMMANDER_BRACKET_SERVICE_H
#define COCKATRICE_COMMANDER_BRACKET_SERVICE_H

#include "commander_spellbook_api_accessor.h"
#include "libcockatrice/deck_list/deck_list.h"

#include <QObject>

struct CommanderBracketEstimate
{
    QString bracketTag;

    QString officialName;
    QString displayName;
    QString explanation;

    EstimateBracketResult rawResult;
};

class CommanderBracketService : public QObject
{
    Q_OBJECT

public:
    static CommanderBracketService &instance();

    quint64 estimateBracket(const DeckList &deck, QObject *requester);

signals:
    void estimateFinished(quint64 requestId, QObject *requester, const CommanderBracketEstimate &estimate);

    void estimateError(quint64 requestId, QObject *requester, const QString &error);

private slots:
    void onEstimateBracketFinished(CommanderSpellbookApiAccessor::RequestId id,
                                   QObject *requester,
                                   const EstimateBracketResult &result);

    void onEstimateBracketError(CommanderSpellbookApiAccessor::RequestId id, QObject *requester, const QString &error);

private:
    explicit CommanderBracketService(QObject *parent = nullptr);
};

#endif