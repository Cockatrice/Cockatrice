#ifndef COCKATRICE_COMMANDER_SPELLBOOK_VARIANT_RESULT_H
#define COCKATRICE_COMMANDER_SPELLBOOK_VARIANT_RESULT_H
#include "commander_spellbook_bracket_tag.h"

#include <QJsonArray>
#include <QJsonObject>

class CommanderSpellbookVariantResult
{
public:
    void fromJson(const QJsonObject &json);

    QString id;
    QString status;

    QJsonArray uses;
    QJsonArray cardRequires;
    QJsonArray produces;
    QJsonArray of;
    QJsonArray includes;

    QJsonArray manaNeeded;
    QJsonArray manaValueNeeded;

    QJsonArray easyPrerequisites;
    QJsonArray notablePrerequisites;

    QString description;
    QString notes;
    double popularity = 0.0;

    bool spoiler = false;
    CommanderSpellbookBracketTag::BracketTag bracketTag = CommanderSpellbookBracketTag::BracketTag::Unknown;

    QJsonObject legalities;
    QJsonObject prices;

    int variantCount = 0;
};

#endif // COCKATRICE_COMMANDER_SPELLBOOK_VARIANT_RESULT_H
