#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <QMap>
#include <QString>
#include <functional>

namespace peg
{
template <typename Annotation> struct AstBase;
struct EmptyType;
typedef AstBase<EmptyType> Ast;
} // namespace peg

class Expression
{
public:
    double value;

    explicit Expression(double initial = 0);
    double parse(const QString &expr);

private:
    double eval(const peg::Ast &ast);
    QMap<QString, std::function<double(double)>> fns;
};

#endif
