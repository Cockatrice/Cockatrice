/**
 * @file deck_filter_string.h
 * @ingroup DeckStorageWidgets
 */
//! \todo Document this file.

#ifndef DECK_FILTER_STRING_H
#define DECK_FILTER_STRING_H

#include "../interface/deck_loader/loaded_deck.h"

#include <QLoggingCategory>
#include <QString>
#include <functional>

inline Q_LOGGING_CATEGORY(DeckFilterStringLog, "deck_filter_string");

/**
 * The data a deck search expression is evaluated against.
 *
 * This is a data view rather than a widget pointer, so the same filter
 * expression can be evaluated against a model or a live widget.
 */
struct DeckSearchData
{
    const LoadedDeck *deck = nullptr; ///< The loaded deck. Must not be null.
    QString filePath;                 ///< Absolute path of the deck file.
    QString displayName;              ///< Deck name, or the file name if the deck has no name.
    QString relativeFilePath;         ///< File path relative to the deck folder.
};

typedef std::function<bool(const DeckSearchData &data)> DeckFilter;

class DeckFilterString
{
public:
    DeckFilterString();
    explicit DeckFilterString(const QString &expr);
    bool check(const DeckSearchData &data) const
    {
        return filter(data);
    }

    [[nodiscard]] bool valid() const
    {
        return _error.isEmpty();
    }

    QString error()
    {
        return _error;
    }

private:
    QString _error;
    DeckFilter filter;
};
#endif // DECK_FILTER_STRING_H
