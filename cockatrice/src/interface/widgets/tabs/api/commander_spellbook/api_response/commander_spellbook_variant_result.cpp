#include "commander_spellbook_variant_result.h"

void CommanderSpellbookVariantResult::fromJson(const QJsonObject &json)
{
    id = json.value("id").toString();
    status = json.value("status").toString();

    uses = json.value("uses").toArray();
    cardRequires = json.value("requires").toArray();
    produces = json.value("produces").toArray();
    of = json.value("of").toArray();
    includes = json.value("includes").toArray();

    manaNeeded = json.value("manaNeeded").toArray();
    manaValueNeeded = json.value("manaValueNeeded").toArray();

    easyPrerequisites = json.value("easyPrerequisites").toArray();
    notablePrerequisites = json.value("notablePrerequisites").toArray();

    description = json.value("description").toString();
    notes = json.value("notes").toString();
    popularity = json.value("popularity").toDouble();

    spoiler = json.value("spoiler").toBool();
    bracketTag = CommanderSpellbookBracketTag::bracketTagFromString(json.value("bracketTag").toString());

    legalities = json.value("legalities").toObject();
    prices = json.value("prices").toObject();

    variantCount = json.value("variantCount").toInt();
}
