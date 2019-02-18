#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <QMap>
#include <QString>
#include <QVariant>
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
    QVariant value;

    explicit Expression(QVariant initial = 0);
    QVariant parse(const QString &expr);
    void addFunction(QString name, std::function<QVariant(QVariantList)> fn);

private:
    QVariant eval(const peg::Ast &ast);
    QMap<QString, std::function<QVariant(QVariantList)>> fns;
};

#endif
