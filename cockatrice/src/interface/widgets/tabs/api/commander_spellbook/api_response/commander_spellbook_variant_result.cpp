#include "commander_spellbook_variant_result.h"

CommanderSpellbookVariantResult CommanderSpellbookVariantResult::fromJson(const QJsonObject &json)
{
    CommanderSpellbookVariantResult result;

    result.id = json.value("id").toString();
    result.status = json.value("status").toString();

    result.uses = json.value("uses").toArray();
    result.cardRequires = json.value("requires").toArray();
    result.produces = json.value("produces").toArray();
    result.of = json.value("of").toArray();
    result.includes = json.value("includes").toArray();

    result.manaNeeded = json.value("manaNeeded").toArray();
    result.manaValueNeeded = json.value("manaValueNeeded").toArray();

    result.easyPrerequisites = json.value("easyPrerequisites").toArray();
    result.notablePrerequisites = json.value("notablePrerequisites").toArray();

    result.description = json.value("description").toString();
    result.notes = json.value("notes").toString();
    result.popularity = json.value("popularity").toDouble();

    result.spoiler = json.value("spoiler").toBool();
    result.bracketTag = json.value("bracketTag").toString();

    result.legalities = json.value("legalities").toObject();
    result.prices = json.value("prices").toObject();

    result.variantCount = json.value("variantCount").toInt();

    return result;
}
