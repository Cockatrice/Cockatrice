#ifndef COCKATRICE_COMMANDER_BRACKET_DEFINITIONS_H
#define COCKATRICE_COMMANDER_BRACKET_DEFINITIONS_H

#include <QHash>
#include <QString>

struct CommanderBracketDefinition
{
    QString tag;

    QString officialName;
    QString displayName;

    QString explanation;
};

class CommanderBracketDefinitions
{
public:
    void clear();

    void addDefinition(const CommanderBracketDefinition &definition);

    QString officialName(const QString &tag) const;
    QString displayName(const QString &tag) const;
    QString explanation(const QString &tag) const;

    bool contains(const QString &tag) const;

private:
    QHash<QString, CommanderBracketDefinition> definitions;
};

#endif // COCKATRICE_COMMANDER_BRACKET_DEFINITIONS_H
