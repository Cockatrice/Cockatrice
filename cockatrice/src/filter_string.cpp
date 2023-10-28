#include "filter_string.h"

#include "../../common/lib/peglib.h"

#include <QByteArray>
#include <QDebug>
#include <QString>
#include <functional>

peg::parser search(R"(
Start <- QueryPartList
~ws <- [ ]+
QueryPartList <- ComplexQueryPart ( ws ("and" ws)? ComplexQueryPart)* ws*

ComplexQueryPart <- SomewhatComplexQueryPart ws $or<[oO][rR]> ws ComplexQueryPart / SomewhatComplexQueryPart

SomewhatComplexQueryPart <- [(] QueryPartList [)] / QueryPart

QueryPart <- NotQuery / SetQuery / RarityQuery / CMCQuery / FormatQuery / PowerQuery / ToughnessQuery / ColorQuery / TypeQuery / OracleQuery / FieldQuery / GenericQuery

NotQuery <- ('not' ws/'-') SomewhatComplexQueryPart
SetQuery <- ('e'/'set') [:] FlexStringValue
OracleQuery <- 'o' [:] RegexString


CMCQuery <- ('cmc'/'mv') ws? NumericExpression
PowerQuery <- [Pp] 'ow' 'er'? ws? NumericExpression
ToughnessQuery <- [Tt] 'ou' 'ghness'? ws? NumericExpression
RarityQuery <- [rR] ':' RegexString

FormatQuery <- 'f' ':' Format / Legality ':' Format
Format <- [a-zA-Z] [a-z]*
Legality <- [Ll] 'egal'? / [Bb] 'anned'? / [Rr] 'estricted'


TypeQuery <- [tT] 'ype'? [:] StringValue

Color <- < [Ww] 'hite'? / [Uu] / [Bb] 'lack'? / [Rr] 'ed'? / [Gg] 'reen'?  / [Bb] 'lue'? >
ColorEx <- Color / [mc]

ColorQuery <- [cC] 'olor'? <[iI]?> <[:!]> ColorEx*

FieldQuery <- String [:] RegexString / String ws? NumericExpression

NonQuote <- !["].
UnescapedStringListPart <- ![":<>=! ].
String <- UnescapedStringListPart+ / ["] <NonQuote*> ["]
StringValue <- String / [(] StringList [)]
StringList <- StringListString (ws? [,] ws? StringListString)*
StringListString <- UnescapedStringListPart+
GenericQuery <- RegexString
RegexString <- String

FlexStringValue <- CompactStringSet / String / [(] StringList [)]
CompactStringSet <- StringListString ([,+] StringListString)+

NumericExpression <- NumericOperator ws? NumericValue
NumericOperator <- [=:] / <[><!][=]?>
NumericValue <- [0-9]+ 

)");

std::once_flag init;

static void setupParserRules()
{
    auto passthru = [](const peg::SemanticValues &sv) -> Filter { return !sv.empty() ? sv[0].get<Filter>() : nullptr; };

    search["Start"] = passthru;
    search["QueryPartList"] = [](const peg::SemanticValues &sv) -> Filter {
        return [=](CardData x) {
            for (int i = 0; i < static_cast<int>(sv.size()); ++i) {
                if (!sv[i].get<Filter>()(x))
                    return false;
            }
            return true;
        };
    };
    search["ComplexQueryPart"] = [](const peg::SemanticValues &sv) -> Filter {
        return [=](CardData x) {
            for (int i = 0; i < static_cast<int>(sv.size()); ++i) {
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
            for (const auto &set : x->getSets().keys()) {
                if (matcher(set))
                    return true;
            }
            return false;
        };
    };
    search["RarityQuery"] = [](const peg::SemanticValues &sv) -> Filter {
        StringMatcher matcher = sv[0].get<StringMatcher>();
        return [=](CardData x) -> bool {
            for (const auto &set : x->getSets().values()) {
                if (matcher(set.getProperty("rarity")))
                    return true;
            }
            return false;
        };
    };
    search["FormatQuery"] = [](const peg::SemanticValues &sv) -> Filter {
        if (sv.choice() == 0) {
            QString format = sv[0].get<QString>();
            return [=](CardData x) -> bool { return x->getProperty(QString("format-%1").arg(format)) == "legal"; };
        } else {
            QString format = sv[1].get<QString>();
            QString legality = sv[0].get<QString>();
            return [=](CardData x) -> bool { return x->getProperty(QString("format-%1").arg(format)) == legality; };
        }
    };
    search["Legality"] = [](const peg::SemanticValues &sv) -> QString {
        switch (tolower(sv.str()[0])) {
            case 'l':
                return "legal";
            case 'b':
                return "banned";
            case 'r':
                return "restricted";
            default:
                return "";
        }
    };

    search["Format"] = [](const peg::SemanticValues &sv) -> QString {
        if (sv.length() == 1) {
            switch (tolower(sv.str()[0])) {
                case 'm':
                    return "modern";
                case 's':
                    return "standard";
                case 'v':
                    return "vintage";
                case 'l':
                    return "legacy";
                case 'c':
                    return "commander";
                case 'p':
                    return "pioneer";
                default:
                    return "";
            }
        } else {
            return QString::fromStdString(sv.str()).toLower();
        }
    };
    search["StringValue"] = [](const peg::SemanticValues &sv) -> StringMatcher {
        if (sv.choice() == 0) {
            auto target = sv[0].get<QString>();
            return [=](const QString &s) { return s.split(" ").contains(target, Qt::CaseInsensitive); };
        } else {
            auto target = sv[0].get<QStringList>();
            return [=](const QString &s) {
                for (const QString &str : target) {
                    if (s.split(" ").contains(str, Qt::CaseInsensitive)) {
                        return true;
                    }
                }
                return false;
            };
        }
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
                for (const QString &str : target) {
                    if (s.split(" ").contains(str, Qt::CaseInsensitive)) {
                        return true;
                    }
                }
                return false;
            };
        } else {
            auto target = sv[0].get<QString>();
            return [=](const QString &s) { return s.split(" ").contains(target, Qt::CaseInsensitive); };
        }
    };
    search["CompactStringSet"] = search["StringList"] = [](const peg::SemanticValues &sv) -> QStringList {
        QStringList result;
        for (int i = 0; i < static_cast<int>(sv.size()); ++i) {
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
        QString parts;
        for (int i = 0; i < static_cast<int>(sv.size()); ++i) {
            parts += sv[i].get<char>();
        }
        bool idenity = sv.tokens[0].first[0] != 'i';
        if (sv.tokens[1].first[0] == ':') {
            return [=](CardData x) {
                QString match = idenity ? x->getColors() : x->getProperty("coloridentity");
                if (parts.contains("m") && match.length() < 2) {
                    return false;
                } else if (parts == "m") {
                    return true;
                }

                if (parts.contains("c") && match.length() == 0)
                    return true;

                for (const auto &i : match) {
                    if (parts.contains(i))
                        return true;
                }
                return false;
            };
        } else {
            return [=](CardData x) {
                QString match = idenity ? x->getColors() : x->getProperty("colorIdentity");
                if (parts.contains("m") && match.length() < 2)
                    return false;

                if (parts.contains("c") && match.length() != 0)
                    return false;

                for (const auto &part : parts) {
                    if (!match.contains(part))
                        return false;
                }

                for (const auto &i : match) {
                    if (!parts.contains(i))
                        return false;
                }
                return true;
            };
        }
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
            return [=](CardData x) -> bool { return x->hasProperty(field) ? matcher(x->getProperty(field)) : false; };
        } else {
            NumberMatcher matcher = sv[1].get<NumberMatcher>();
            return [=](CardData x) -> bool {
                return x->hasProperty(field) ? matcher(x->getProperty(field).toInt()) : false;
            };
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

FilterString::FilterString()
{
    result = [](CardData) -> bool { return false; };
    _error = "Not initialized";
}

FilterString::FilterString(const QString &expr)
{
    QByteArray ba = expr.simplified().toUtf8();

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
        qDebug() << "Filter string error" << _error;
        result = [](CardData) -> bool { return false; };
    }
}
