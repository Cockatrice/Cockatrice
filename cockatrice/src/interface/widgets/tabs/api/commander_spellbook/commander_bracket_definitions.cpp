#include "commander_bracket_definitions.h"

void CommanderBracketDefinitions::clear()
{
    definitions.clear();
}

void CommanderBracketDefinitions::addDefinition(const CommanderBracketDefinition &definition)
{
    definitions.insert(definition.tag, definition);
}

QString CommanderBracketDefinitions::officialName(const QString &tag) const
{
    auto it = definitions.find(tag);

    if (it == definitions.end()) {
        return tag;
    }

    return it->officialName;
}

QString CommanderBracketDefinitions::displayName(const QString &tag) const
{
    auto it = definitions.find(tag);

    if (it == definitions.end()) {
        return tag;
    }

    return it->displayName;
}

QString CommanderBracketDefinitions::explanation(const QString &tag) const
{
    auto it = definitions.find(tag);

    if (it == definitions.end()) {
        return {};
    }

    return it->explanation;
}

bool CommanderBracketDefinitions::contains(const QString &tag) const
{
    return definitions.contains(tag);
}