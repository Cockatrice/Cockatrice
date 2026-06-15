#include "commander_bracket_service.h"

#include "../../../../../client/settings/cache_settings.h"

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

    auto &definitions = SettingsCache::instance().commanderBracketDefs();

    estimate.officialName = definitions.officialName(result.bracketTag);

    estimate.displayName = definitions.displayName(result.bracketTag);

    estimate.explanation = definitions.explanation(result.bracketTag);

    emit estimateFinished(id, requester, estimate);
}

void CommanderBracketService::onEstimateBracketError(CommanderSpellbookApiAccessor::RequestId id,
                                                     QObject *requester,
                                                     const QString &error)
{
    emit estimateError(id, requester, error);
}