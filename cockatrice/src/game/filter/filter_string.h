#ifndef FILTER_STRING_H
#define FILTER_STRING_H

#include "carddatabase.h"
#include "filtertree.h"

#include <QMap>
#include <QString>
#include <functional>
#include <utility>

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
    bool check(const CardData &card)
    {
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
