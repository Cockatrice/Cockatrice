#include "filter_string.h"
#include "../../common/lib/peglib.h"

#include <QByteArray>
#include <QString>
#include <cmath>
#include <functional>

peg::parser search(R"(
Start <- QueryPartList
~ws <- [ ]+
QueryPartList <- ComplexQueryPart ( ws (ws "and ws")? ComplexQueryPart)* ws*

ComplexQueryPart <- SomewhatComplexQueryPart ws $or<[oO][rR]> ws SomewhatComplexQueryPart / SomewhatComplexQueryPart

SomewhatComplexQueryPart <- [(] QueryPartList [)] / QueryPart

QueryPart <- NotQuery / SetQuery / CMCQuery / PowerQuery / ToughnessQuery / ColorQuery / TypeQuery / OracleQuery / FieldQuery / GenericQuery

NotQuery <- ('not' ws/'-') QueryPart
SetQuery <- 'e' [:] FlexStringValue
OracleQuery <- 'o' [:] RegexString


CMCQuery <- 'cmc' ws? NumericExpression
PowerQuery <- 'pow' ws? NumericExpression
ToughnessQuery <- 'tou' ws? NumericExpression


TypeQuery <- [tT] 'ype'? [:] StringValue

Color <- < [Ww] 'hite'? / [Uu] / [Bb] 'lack'? / [Rr] 'ed'? / [Gg] 'reen'?  / [Bb] 'lue'? >
ColorEx <- Color / [mc]

ColorQuery <- [cC] 'olor'? [:] ColorEx

FieldQuery <- String [:] RegexString / String ws? NumericExpression

NonQuote <- !["].
UnescapedStringListPart <- [a-zA-Z]+
String <- UnescapedStringListPart / ["] <NonQuote*> ["]
StringValue <- String / [(] StringList [)]
StringList <- StringListString (ws? [,] ws? StringListString)*
StringListString <- UnescapedStringListPart
GenericQuery <- RegexString
RegexString <- String

FlexStringValue <- CompactStringSet / String / [(] StringList [)]
CompactStringSet <- StringListString ([,+] StringListString)+

NumericExpression <- NumericOperator ws? NumericValue
NumericOperator <- [=:] / <[><!][=]?>
NumericValue <- [0-9]+ 

)");

std::once_flag init;

static void setupParserRules() {
    auto passthru = [](const peg::SemanticValues &sv) -> Filter { return sv.size() > 0 ? sv[0].get<Filter>() : nullptr; };

    search["Start"] = passthru;
    search["QueryPartList"] = [](const peg::SemanticValues &sv) -> Filter {
        return [=](CardData x) {
            for (int i = 0; i < sv.size(); ++i) {
                if (!sv[i].get<Filter>()(x))
                    return false;
            }
            return true;
        };
    };
    search["ComplexQueryPart"] = [](const peg::SemanticValues &sv) -> Filter {
        return [=](CardData x) {
            for (int i = 0; i < sv.size(); ++i) {
                if (sv[i].get<Filter>()(x))
                    return true;
            }
            return false;
        };
    };
    search["SomewhatComplexQueryPart"] = passthru;
    search["QueryPart"] = passthru;
    search["NotQuery"] = [](const peg::SemanticValues &sv) -> Filter {
        Filter dependent = sv[0].get<Filter>();
        return [=](CardData x) -> bool { return !dependent(x); };
    };
    search["TypeQuery"] = [](const peg::SemanticValues &sv) -> Filter {
        StringMatcher matcher = sv[0].get<StringMatcher>();
        return [=](CardData x) -> bool { return matcher(x->getCardType()); };
    };
    search["SetQuery"] = [](const peg::SemanticValues &sv) -> Filter {
        StringMatcher matcher = sv[0].get<StringMatcher>();
        return [=](CardData x) -> bool {
            for (auto set : x->getSets().keys()) {
                if (matcher(set))
                    return true;
            }
            return false;
        };
    };
    search["StringValue"] = [](const peg::SemanticValues &sv) -> StringMatcher {
        auto target = sv[0].get<QString>();
        return [=](const QString &s) { return s.split(" ").contains(target, Qt::CaseInsensitive); };
    };

    search["String"] = [](const peg::SemanticValues &sv) -> QString {
        if (sv.choice() == 0) {
            return QString::fromStdString(sv.str());
        } else {
            return QString::fromStdString(sv.token(0));
        }
    };
    search["FlexStringValue"] = [](const peg::SemanticValues &sv) -> StringMatcher {
        if (sv.choice() != 1) {
            auto target = sv[0].get<QStringList>();
            return [=](const QString &s) {
                qDebug() << target << " vs " << s;
                
                for (QString str : target) {
                    if ( str == s )
                        return true;
                }
                return false;
            };
        } else {
            auto target = sv[0].get<QString>();
            return [=](const QString &s) { return s.split(" ").contains(target, Qt::CaseInsensitive); };
        }
    };
    search["CompactStringSet"] = search["StringList"] = [](const peg::SemanticValues &sv)->QStringList {
        QStringList result;
        for ( int i = 0; i < sv.size(); ++i ) {
            result.append(sv[i].get<QString>());
        }
        return result;
    };
    search["StringListString"] = [](const peg::SemanticValues &sv) -> QString {
        return QString::fromStdString(sv.str());
    };

    search["NumericExpression"] = [](const peg::SemanticValues &sv) -> NumberMatcher {
        auto arg = sv[1].get<int>();
        auto op = sv[0].get<QString>();

        if (op == ">")
            return [=](int s) { return s > arg; };
        if (op == ">=")
            return [=](int s) { return s >= arg; };
        if (op == "<")
            return [=](int s) { return s < arg; };
        if (op == "<=")
            return [=](int s) { return s <= arg; };
        if (op == "=")
            return [=](int s) { return s == arg; };
        if (op == ":")
            return [=](int s) { return s == arg; };
        if (op == "!=")
            return [=](int s) { return s != arg; };
        return [](int) { return false; };
    };

    search["NumericValue"] = [](const peg::SemanticValues &sv) -> int {
        return QString::fromStdString(sv.str()).toInt();
    };

    search["NumericOperator"] = [](const peg::SemanticValues &sv) -> QString {
        return QString::fromStdString(sv.str());
    };

    search["RegexString"] = [](const peg::SemanticValues &sv) -> StringMatcher {
        auto target = sv[0].get<QString>();
        return [=](const QString &s) { return s.QString::contains(target, Qt::CaseInsensitive); };
    };

    search["OracleQuery"] = [](const peg::SemanticValues &sv) -> Filter {
        StringMatcher matcher = sv[0].get<StringMatcher>();
        return [=](CardData x) { return matcher(x->getText()); };
    };

    search["ColorQuery"] = [](const peg::SemanticValues &sv) -> Filter {
        char target = sv[0].get<char>();
        return [=](CardData x) {
            if (target == 'm')
                return x->getColors().length() > 1;
            return x->getColors().contains(target);
        };
    };

    search["CMCQuery"] = [](const peg::SemanticValues &sv) -> Filter {
        NumberMatcher matcher = sv[0].get<NumberMatcher>();
        return [=](CardData x) -> bool { return matcher(x->getProperty("cmc").toInt()); };
    };
    search["PowerQuery"] = [](const peg::SemanticValues &sv) -> Filter {
        NumberMatcher matcher = sv[0].get<NumberMatcher>();
        return [=](CardData x) -> bool { return matcher(x->getPowTough().split("/")[0].toInt()); };
    };
    search["ToughnessQuery"] = [](const peg::SemanticValues &sv) -> Filter {
        NumberMatcher matcher = sv[0].get<NumberMatcher>();
        return [=](CardData x) -> bool {
            auto parts = x->getPowTough().split("/");
            return matcher(parts.length() == 2 ? parts[1].toInt() : 0);
        };
    };
    search["FieldQuery"] = [](const peg::SemanticValues &sv) -> Filter {
        QString field = sv[0].get<QString>();
        if (sv.choice() == 0) {
            StringMatcher matcher = sv[1].get<StringMatcher>();
            return [=](CardData x) -> bool { return matcher(x->getProperty(field)); };
        } else {
            NumberMatcher matcher = sv[1].get<NumberMatcher>();
            return [=](CardData x) -> bool { return matcher(x->getProperty(field).toInt()); };
        }
    };
    search["GenericQuery"] = [](const peg::SemanticValues &sv) -> Filter {
        StringMatcher matcher = sv[0].get<StringMatcher>();
        return [=](CardData x) { return matcher(x->getName()); };
    };

    search["Color"] = [](const peg::SemanticValues &sv) -> char { return "WUBRGU"[sv.choice()]; };
    search["ColorEx"] = [](const peg::SemanticValues &sv) -> char {
        return sv.choice() == 0 ? sv[0].get<char>() : *sv.c_str();
    };
}

FilterString::FilterString(const QString &expr)
{
    QByteArray ba = expr.toLocal8Bit();

    std::call_once(init, setupParserRules);

    _error = QString();

    if (ba.isEmpty()) {
        result = [](CardData) -> bool { return true; };
        return;
    }

    search.log = [&](size_t ln, size_t col, const std::string &msg) {
        _error = QString("%1:%2: %3").arg(ln).arg(col).arg(QString::fromStdString(msg));
    };

    if (!search.parse(ba.data(), result)) {
        std::cout << "Error!" << _error.toStdString() << std::endl;
        result = [](CardData) -> bool { return false; };
    }
}
