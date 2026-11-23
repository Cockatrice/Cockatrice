#ifndef PARSEHELPERS_H
#define PARSEHELPERS_H

#include <QString>
#include <optional>

/** @brief Read a finite deck-copy allowance naming this card in canonical English rules text. */
std::optional<int> parseDeckCopyLimit(const QString &name, const QString &text);

bool parseCipt(const QString &name, const QString &text);

#endif // PARSEHELPERS_H
