#ifndef FILTER_STRING_H
#define FILTER_STRING_H

#include "carddatabase.h"
#include "filtertree.h"

#include <QMap>
#include <QString>
#include <functional>

typedef CardInfoPtr CardData;
typedef std::function<bool(CardData)> Filter;
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
    explicit FilterString(const QString &exp);
    bool check(CardData card)
    {
        return result(card);
    }

private:
    Filter result;
};

#endif
