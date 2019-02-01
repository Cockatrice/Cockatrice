#include "expression.h"
#include "./lib/peglib.h"

#include <QByteArray>
#include <QString>
#include <cmath>
#include <functional>

peg::parser math(R"(
    EXPRESSION   <-  P0
    P0           <-  P1 (P1_OPERATOR P1)*
    P1           <-  P2 (P2_OPERATOR P2)*
    P2           <-  P3 (P3_OPERATOR P3)*
    P3           <-  NUMBER / FUNCTION / VARIABLE / '(' P0 ')'

    P1_OPERATOR  <-  < [-+] >
    P2_OPERATOR  <-  < [/*] >
    P3_OPERATOR  <-  < '^' >

    NUMBER       <-  < '-'? [0-9]+ >
    NAME         <-  < [a-z][a-z0-9]* >
    VARIABLE     <-  < [x] >
    FUNCTION     <-  NAME '(' EXPRESSION ( [,\n] EXPRESSION )* ')'

    %whitespace  <-  [ \t\r]*
    )");

QMap<QString, std::function<double(double)>> *default_functions = nullptr;

Expression::Expression(double initial) : value(initial)
{
    if (default_functions == nullptr) {
        default_functions = new QMap<QString, std::function<double(double)>>();
        default_functions->insert("sin", [](double a) { return sin(a); });
        default_functions->insert("cos", [](double a) { return cos(a); });
        default_functions->insert("tan", [](double a) { return tan(a); });
        default_functions->insert("sqrt", [](double a) { return sqrt(a); });
        default_functions->insert("log", [](double a) { return log(a); });
        default_functions->insert("log10", [](double a) { return log(a); });
        default_functions->insert("trunc", [](double a) { return trunc(a); });
        default_functions->insert("abs", [](double a) { return abs(a); });

        default_functions->insert("floor", [](double a) { return floor(a); });
        default_functions->insert("ceil", [](double a) { return ceil(a); });
        default_functions->insert("round", [](double a) { return round(a); });
        default_functions->insert("trunc", [](double a) { return trunc(a); });
    }
    fns = QMap<QString, std::function<double(double)>>(*default_functions);
}

double Expression::eval(const peg::Ast &ast)
{
    const auto &nodes = ast.nodes;
    if (ast.name == "NUMBER") {
        return stod(ast.token);
    } else if (ast.name == "FUNCTION") {
        QString name = QString::fromStdString(nodes[0]->token);
        if (!fns.contains(name))
            return 0;
        return fns[name](eval(*nodes[1]));
    } else if (ast.name == "VARIABLE") {
        return value;
    } else if (ast.name[0] == 'P') {
        double result = eval(*nodes[0]);
        for (int i = 1; i < nodes.size(); i += 2) {
            double arg = eval(*nodes[i + 1]);
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
    } else {
        return -1;
    }
}

double Expression::parse(const QString &expr)
{
    QByteArray ba = expr.toLocal8Bit();

    math.enable_ast();

    std::shared_ptr<peg::Ast> ast;
    if (math.parse(ba.data(), ast)) {
        ast = peg::AstOptimizer(true).optimize(ast);
        return eval(*ast);
    }

    return 0;
}
