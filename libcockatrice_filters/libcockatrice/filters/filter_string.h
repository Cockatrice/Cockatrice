/**
 * @file filter_string.h
 * @ingroup CardDatabaseModelFilters
 * @brief TODO: Document this.
 */

#ifndef FILTER_STRING_H
#define FILTER_STRING_H

#include "filter_tree.h"

#include <QLoggingCategory>
#include <QMap>
#include <QString>
#include <functional>
#include <libcockatrice/card/card_info.h>
#include <utility>

inline Q_LOGGING_CATEGORY(FilterStringLog, "filter_string");

typedef CardInfoPtr CardData;
typedef std::function<bool(const CardData &)> Filter;
typedef std::function<bool(const QString &)> StringMatcher;
typedef std::function<bool(int)> NumberMatcher;

namespace peg
{
template <typename Annotation> struct AstBase;
struct EmptyType;
typedef AstBase<EmptyType> Ast;
} // namespace peg

class FilterString
{
public:
    FilterString();
    explicit FilterString(const QString &exp);
    [[nodiscard]] bool check(const CardData &card) const
    {
        if (card.isNull()) {
            static CardInfoPtr blankCard = CardInfo::newInstance("");
            return result(blankCard);
        }
        return result(card);
    }

    bool valid()
    {
        return _error.isEmpty();
    }

    QString error()
    {
        return _error;
    }

private:
    QString _error;
    Filter result;
};

#endif
