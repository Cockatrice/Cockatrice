#ifndef MANABOX_JSON_DECK_PARSER_H
#define MANABOX_JSON_DECK_PARSER_H

#include "interface_json_deck_parser.h"

/** @brief Imports the public ManaBox cloud deck response, including exact printings and boards. */
class ManaBoxJsonParser : public IJsonDeckParser
{
public:
    DeckList parse(const QJsonObject &obj) override;
};

#endif // MANABOX_JSON_DECK_PARSER_H
