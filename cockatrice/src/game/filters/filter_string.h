#ifndef FILTER_STRING_H
#define FILTER_STRING_H

#include "../cards/card_database.h"
#include "filter_tree.h"

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

class FilterString : QObject
{
    Q_OBJECT

public:
    FilterString();
    explicit FilterString(const QString &exp);
    bool check(const CardData &card) const
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
    void logger(size_t ln, size_t col, const std::string &msg);

    QString _error;
    Filter result;
};

#endif
