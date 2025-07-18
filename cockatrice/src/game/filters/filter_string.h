#ifndef FILTER_STRING_H
#define FILTER_STRING_H

#include "../cards/card_info.h"
#include "filter_tree.h"

#include <QLoggingCategory>
#include <QMap>
#include <QString>
#include <functional>
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

class FilterString : QObject
{
    Q_OBJECT

public:
    FilterString();
    explicit FilterString(const QString &exp);
    bool check(const CardData &card) const
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
    void logger(size_t ln, size_t col, const std::string &msg);

    QString _error;
    Filter result;
};

#endif
