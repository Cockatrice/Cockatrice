#include "expression.h"
#include "./lib/peglib.h"

#include <QByteArray>
#include <QDebug>
#include <QString>
#include <QVariant>
#include <cmath>
#include <functional>

peg::parser math(R"(
    EXPRESSION   <-  P0 / IF / COMPOUND
    P0           <-  P1 (P1_OPERATOR P1)*
    P1           <-  P2 (P2_OPERATOR P2)*
    P2           <-  P3 (P3_OPERATOR P3)*
    P3           <-  LITERAL / FUNCTION / VARIABLE / '(' P0 ')'

    P1_OPERATOR  <-  < [-+] >
    P2_OPERATOR  <-  < [/*] >
    P3_OPERATOR  <-  < '^' >

    LITERAL      <-  STRING / NUMBER 
    NUMBER       <-  < '-'? [0-9]+ >
    NAME         <-  < [a-zA-Z][a-zA-Z0-9]* >
    VARIABLE     <-  < [x] >
    FUNCTION     <-  NAME '(' EXPRESSION? ( [,][ \t\r\n]* EXPRESSION )* ')'

    COMPOUND     <- '{' EXPRESSION ( [;][ \t\r\n]* EXPRESSION )* '}'

    IF           <- 'if' EXPRESSION EXPRESSION ('else' EXPRESSION)?
    STRING       <- ['] <(!['].)*> [']

    %whitespace  <-  [ \t\r\n]*
    )");

QMap<QString, std::function<QVariant(QVariantList)>> *default_functions = nullptr;

typedef double (*ddfn)(double);
static std::function<QVariant(QVariantList)> wrap(ddfn f)
{
    return [=](QVariantList a) {
        if (a.size() != 1)
            return -1.0;
        return f(a[0].toDouble());
    };
}

Expression::Expression(QVariant initial) : value(initial)
{
    if (default_functions == nullptr) {
        default_functions = new QMap<QString, std::function<QVariant(QVariantList)>>();
        default_functions->insert("sin", wrap(sin));
        default_functions->insert("cos", wrap(cos));
        default_functions->insert("tan", wrap(tan));
        default_functions->insert("sqrt", wrap(sqrt));
        default_functions->insert("log", wrap(log));
        default_functions->insert("log10", wrap(log10));
        default_functions->insert("trunc", wrap(trunc));
        default_functions->insert("abs", wrap(abs));

        default_functions->insert("floor", wrap(floor));
        default_functions->insert("ceil", wrap(ceil));
        default_functions->insert("round", wrap(round));
        default_functions->insert("trunc", wrap(trunc));
    }
    fns = QMap<QString, std::function<QVariant(QVariantList)>>(*default_functions);
}

void Expression::addFunction(QString name, std::function<QVariant(QVariantList)> fn)
{
    fns.insert(name.toLower(), fn);
}

QVariant Expression::eval(const peg::Ast &ast)
{
    const auto &nodes = ast.nodes;
    if (ast.name == "NUMBER") {
        return stod(ast.token);
    } else if (ast.name == "STRING") {
        return QString::fromStdString(ast.token);
    } else if (ast.name == "FUNCTION") {
        QString name = QString::fromStdString(nodes[0]->token).toLower();
        if (!fns.contains(name)) {
            return 0;
        }

        QVariantList values;
        for (int i = 1; i < nodes.size(); ++i) {
            values.append(eval(*nodes[i]));
        }
        return fns[name](values);
    } else if (ast.name == "VARIABLE") {
        return value;
    } else if (ast.name == "COMPOUND") {
        QVariant result;
        for (int i = 0; i < nodes.size(); ++i) {
            result = eval(*nodes[i]);
        }
        return result;
    } else if (ast.name[0] == 'P') {
        double result = eval(*nodes[0]).toDouble();
        for (int i = 1; i < nodes.size(); i += 2) {
            double arg = eval(*nodes[i + 1]).toDouble();
            char operation = nodes[i]->token[0];
            switch (operation) {
                case '+':
                    result += arg;
                    break;
                case '-':
                    result -= arg;
                    break;
                case '*':
                    result *= arg;
                    break;
                case '/':
                    result /= arg;
                    break;
                case '^':
                    result = pow(result, arg);
                    break;
                default:
                    result = 0;
                    break;
            }
        }
        return result;
    } else if ( ast.name == "IF") {
        bool test = eval(*nodes[0]).toBool();
        if ( test ) {
            return eval(*nodes[1]);
        } else if ( nodes.size() > 2 ) {
            return eval(*nodes[2]);
        } else {
            return QVariant();
        }
    } else {
        std::cerr << "Wat is " << ast.name;
        return -1;
    }
}

QVariant Expression::parse(const QString &expr)
{
    QByteArray ba = expr.toLocal8Bit();

    math.enable_ast();

    std::shared_ptr<peg::Ast> ast;
    if (math.parse(ba.data(), ast)) {
        ast = peg::AstOptimizer(true, {"FUNCTION"}).optimize(ast);
        std::cerr << ast_to_s(ast);
        return eval(*ast);
    } else {
        std::cerr << "Parse error";
    }

    return 0;
}
