#include "commander_bracket_service.h"

#include "../../../../../client/settings/cache_settings.h"

#include <libcockatrice/settings/commander_bracket_settings.h>

CommanderBracketService &CommanderBracketService::instance()
{
    static CommanderBracketService service;
    return service;
}

CommanderBracketService::CommanderBracketService(QObject *parent) : QObject(parent)
{
    connect(&CommanderSpellbookApiAccessor::instance(), &CommanderSpellbookApiAccessor::estimateBracketFinished, this,
            &CommanderBracketService::onEstimateBracketFinished);

    connect(&CommanderSpellbookApiAccessor::instance(), &CommanderSpellbookApiAccessor::estimateBracketError, this,
            &CommanderBracketService::onEstimateBracketError);
}

quint64 CommanderBracketService::estimateBracket(const DeckList &deck, QObject *requester)
{
    return CommanderSpellbookApiAccessor::instance().estimateBracket(deck, requester);
}

void CommanderBracketService::onEstimateBracketFinished(CommanderSpellbookApiAccessor::RequestId id,
                                                        QObject *requester,
                                                        const EstimateBracketResult &result)
{
    CommanderBracketEstimate estimate;

    estimate.bracketTag = result.bracketTag;

    estimate.rawResult = result;

    auto &brackets = SettingsCache::instance().commanderBrackets();

    estimate.officialName = brackets.officialName(result.bracketTag);

    estimate.displayName = brackets.displayName(result.bracketTag);

    estimate.explanation = brackets.explanation(result.bracketTag);

    emit estimateFinished(id, requester, estimate);
}

void CommanderBracketService::onEstimateBracketError(CommanderSpellbookApiAccessor::RequestId id,
                                                     QObject *requester,
                                                     const QString &error)
{
    emit estimateError(id, requester, error);
}
