/**
 * @file filter_string.h
 * @ingroup CardDatabaseModelFilters
 */
//! \todo Document this file.

#ifndef FILTER_STRING_H
#define FILTER_STRING_H

#include <QCoreApplication>
#include <QLoggingCategory>
#include <QString>
#include <functional>
#include <libcockatrice/card/card_info.h>
#include <libcockatrice/card/card_localization.h>

struct CardSearchLanguage;

inline Q_LOGGING_CATEGORY(FilterStringLog, "filter_string");

typedef CardInfoPtr CardData;
typedef std::function<bool(const CardData &)> Filter;
typedef std::function<bool(const QString &)> StringMatcher;
typedef std::function<bool(int)> NumberMatcher;
typedef std::function<bool(int, int)> NumberComparer;

namespace peg
{
template <typename Annotation> struct AstBase;
struct EmptyType;

typedef AstBase<EmptyType> Ast;
} // namespace peg

class FilterString
{
    Q_DECLARE_TR_FUNCTIONS(FilterString)

public:
    FilterString();
    explicit FilterString(const QString &exp, const CardSearchLanguage &searchLanguage = {});
    [[nodiscard]] bool check(const CardData &card) const
    {
        if (card.isNull()) {
            static CardInfoPtr blankCard = CardInfo::newInstance("");
            return result(blankCard);
        }
        return result(card);
    }

    [[nodiscard]] bool valid() const
    {
        return _error.isEmpty();
    }

    [[nodiscard]] QString error() const;

private:
    // Untranslated parser message; error() adds the translated prefix, logging uses it as-is.
    QString _error;
    // Column reported by the parser, or 0 when the message carries no position.
    size_t _errorColumn = 0;
    Filter result;
};

#endif
