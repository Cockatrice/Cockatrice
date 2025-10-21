#include "filter_string.h"

#include <QByteArray>
#include <QDebug>
#include <QRegularExpression>
#include <QString>
#include <functional>
#include <libcockatrice/utility/peglib.h>

static peg::parser search(R"(
Start <- QueryPartList
~ws <- [ ]+
QueryPartList <- ComplexQueryPart ( ws ("AND" ws)? ComplexQueryPart)* ws*

ComplexQueryPart <- SomewhatComplexQueryPart ws "OR" ws ComplexQueryPart / SomewhatComplexQueryPart

SomewhatComplexQueryPart <- [(] QueryPartList [)] / QueryPart

QueryPart <- NotQuery / SetQuery / RarityQuery / CMCQuery / FormatQuery / PowerQuery / ToughnessQuery / ColorQuery / TypeQuery / OracleQuery / FieldQuery / GenericQuery

NotQuery <- ('NOT' ws/'-') SomewhatComplexQueryPart
SetQuery <- ('e'/'set') [:] FlexStringValue
OracleQuery <- 'o' [:] MatcherString


CMCQuery <- ('cmc'/'mv') ws? NumericExpression
PowerQuery <- [Pp] 'ow' 'er'? ws? NumericExpression
ToughnessQuery <- [Tt] 'ou' 'ghness'? ws? NumericExpression

RarityQuery <- [rR] ':' Rarity
Rarity <- [Cc] 'ommon'? / [Uu] 'ncommon'? / [Rr] 'are'? / [Mm] 'ythic'? / [Ss] 'pecial'? / [a-zA-Z] [a-z]*

FormatQuery <- 'f' ':' Format / Legality ':' Format
Format <- [a-zA-Z] [a-z]*
Legality <- [Ll] 'egal'? / [Bb] 'anned'? / [Rr] 'estricted'


TypeQuery <- [tT] 'ype'? [:] StringValue

Color <- < [Ww] 'hite'? / [Uu] / [Bb] 'lack'? / [Rr] 'ed'? / [Gg] 'reen'?  / [Bb] 'lue'? >
ColorEx <- Color / [mc]

ColorQuery <- [cC] 'olor'? <[iI]?> <[:!]> ColorEx*

FieldQuery <- String [:] MatcherString / String ws? NumericExpression

NonDoubleQuoteUnlessEscaped <- '\\\"'. / !["].
NonSingleQuoteUnlessEscaped <- "\\\'". / !['].
UnescapedStringListPart <- !['":<>=! ].
SingleApostropheString <- (UnescapedStringListPart+ ws*)* ['] (UnescapedStringListPart+ ws*)*
String <- SingleApostropheString / UnescapedStringListPart+ / ["] <NonDoubleQuoteUnlessEscaped*> ["] / ['] <NonSingleQuoteUnlessEscaped*> [']
StringValue <- String / [(] StringList [)]
StringList <- StringListString (ws? [,] ws? StringListString)*
StringListString <- UnescapedStringListPart+
GenericQuery <- MatcherString

# A String that can either be a normal string or a regex search string
MatcherString <- RegexMatcher / NormalMatcher

NormalMatcher <- String
RegexMatcher <- '/' RegexMatcherString '/'
RegexMatcherString <- ('\\/' / !'/' .)+

FlexStringValue <- CompactStringSet / String / [(] StringList [)]
CompactStringSet <- StringListString ([,+] StringListString)+

NumericExpression <- NumericOperator ws? NumericValue
NumericOperator <- [=:] / <[><!][=]?>
NumericValue <- [0-9]+
)");

static std::once_flag init;

static void setupParserRules()
{
    auto passthru = [](const peg::SemanticValues &sv) -> Filter {
        return !sv.empty() ? std::any_cast<Filter>(sv[0]) : nullptr;
    };

    search["Start"] = passthru;
    search["QueryPartList"] = [](const peg::SemanticValues &sv) -> Filter {
        return [=](const CardData &x) {
            auto matchesFilter = [&x](const std::any &query) { return std::any_cast<Filter>(query)(x); };
            return std::all_of(sv.begin(), sv.end(), matchesFilter);
        };
    };
    search["ComplexQueryPart"] = [](const peg::SemanticValues &sv) -> Filter {
        return [=](const CardData &x) {
            auto matchesFilter = [&x](const std::any &query) { return std::any_cast<Filter>(query)(x); };
            return std::any_of(sv.begin(), sv.end(), matchesFilter);
        };
    };
    search["SomewhatComplexQueryPart"] = passthru;
    search["QueryPart"] = passthru;
    search["NotQuery"] = [](const peg::SemanticValues &sv) -> Filter {
        const auto dependent = std::any_cast<Filter>(sv[0]);
        return [=](const CardData &x) -> bool { return !dependent(x); };
    };
    search["TypeQuery"] = [](const peg::SemanticValues &sv) -> Filter {
        const auto matcher = std::any_cast<StringMatcher>(sv[0]);
        return [=](const CardData &x) -> bool { return matcher(x->getCardType()); };
    };
    search["SetQuery"] = [](const peg::SemanticValues &sv) -> Filter {
        auto matcher = std::any_cast<StringMatcher>(sv[0]);
        return [=](const CardData &x) -> bool {
            QList<QString> sets = x->getSets().keys();

            auto matchesSet = [&matcher](const QString &set) { return matcher(set); };
            return std::any_of(sets.begin(), sets.end(), matchesSet);
        };
    };
    search["Rarity"] = [](const peg::SemanticValues &sv) -> QString {
        switch (tolower(std::string(sv.sv())[0])) {
            case 'c':
                return "common";
            case 'u':
                return "uncommon";
            case 'r':
                return "rare";
            case 'm':
                return "mythic";
            case 's':
                return "special";
            default:
                return QString::fromStdString(std::string(sv.sv()));
        }
    };
    search["RarityQuery"] = [](const peg::SemanticValues &sv) -> Filter {
        const auto rarity = std::any_cast<QString>(sv[0]);
        return [=](const CardData &x) -> bool {
            QList<PrintingInfo> infos;
            for (const auto &printings : x->getSets()) {
                for (const auto &printing : printings) {
                    infos.append(printing);
                }
            }

            auto matchesRarity = [&rarity](const PrintingInfo &info) { return rarity == info.getProperty("rarity"); };
            return std::any_of(infos.begin(), infos.end(), matchesRarity);
        };
    };

    search["FormatQuery"] = [](const peg::SemanticValues &sv) -> Filter {
        if (sv.choice() == 0) {
            const auto format = std::any_cast<QString>(sv[0]);
            return
                [=](const CardData &x) -> bool { return x->getProperty(QString("format-%1").arg(format)) == "legal"; };
        }

        const auto format = std::any_cast<QString>(sv[1]);
        const auto legality = std::any_cast<QString>(sv[0]);
        return [=](const CardData &x) -> bool { return x->getProperty(QString("format-%1").arg(format)) == legality; };
    };
    search["Legality"] = [](const peg::SemanticValues &sv) -> QString {
        switch (tolower(std::string(sv.sv())[0])) {
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
        if (sv.size() == 1) {
            switch (tolower(std::string(sv.sv())[0])) {
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
        }

        return QString::fromStdString(std::string(sv.sv())).toLower();
    };

    search["StringValue"] = [](const peg::SemanticValues &sv) -> StringMatcher {
        // Helper function for word boundary matching
        auto createWordBoundaryMatcher = [](const QString &target) {
            QString pattern = QString("\\b%1\\b").arg(QRegularExpression::escape(target));
            QRegularExpression regex(pattern, QRegularExpression::CaseInsensitiveOption);
            return [regex](const QString &s) { return regex.match(s).hasMatch(); };
        };

        if (sv.choice() == 0) {
            const auto target = std::any_cast<QString>(sv[0]);
            return createWordBoundaryMatcher(target);
        }

        const auto target = std::any_cast<QStringList>(sv[0]);
        return [=](const QString &s) {
            auto containsString = [&s, &createWordBoundaryMatcher](const QString &str) {
                return createWordBoundaryMatcher(str)(s);
            };
            return std::any_of(target.begin(), target.end(), containsString);
        };
    };

    search["String"] = [](const peg::SemanticValues &sv) -> QString {
        if (sv.choice() == 0) {
            return QString::fromStdString(std::string(sv.sv()));
        }

        return QString::fromStdString(std::string(sv.token(0)));
    };
    search["FlexStringValue"] = [](const peg::SemanticValues &sv) -> StringMatcher {
        if (sv.choice() != 1) {
            const auto target = std::any_cast<QStringList>(sv[0]);
            return [=](const QString &s) {
                auto containsString = [&s](const QString &str) {
                    return s.split(" ").contains(str, Qt::CaseInsensitive);
                };
                return std::any_of(target.begin(), target.end(), containsString);
            };
        }

        const auto target = std::any_cast<QString>(sv[0]);
        return [=](const QString &s) { return s.split(" ").contains(target, Qt::CaseInsensitive); };
    };
    search["CompactStringSet"] = [](const peg::SemanticValues &sv) -> QStringList {
        QStringList result;
        for (const auto &i : sv) {
            result.append(std::any_cast<QString>(i));
        }
        return result;
    };
    search["StringList"] = [](const peg::SemanticValues &sv) -> QStringList {
        QStringList result;
        for (const auto &i : sv) {
            result.append(std::any_cast<QString>(i));
        }
        return result;
    };
    search["StringListString"] = [](const peg::SemanticValues &sv) -> QString {
        return QString::fromStdString(std::string(sv.sv()));
    };

    search["NumericExpression"] = [](const peg::SemanticValues &sv) -> NumberMatcher {
        const auto arg = std::any_cast<int>(sv[1]);
        const auto op = std::any_cast<QString>(sv[0]);

        if (op == ">")
            return [=](const int s) { return s > arg; };
        if (op == ">=")
            return [=](const int s) { return s >= arg; };
        if (op == "<")
            return [=](const int s) { return s < arg; };
        if (op == "<=")
            return [=](const int s) { return s <= arg; };
        if (op == "=")
            return [=](const int s) { return s == arg; };
        if (op == ":")
            return [=](const int s) { return s == arg; };
        if (op == "!=")
            return [=](const int s) { return s != arg; };
        return [](int) { return false; };
    };

    search["NumericValue"] = [](const peg::SemanticValues &sv) -> int {
        return QString::fromStdString(std::string(sv.sv())).toInt();
    };

    search["NumericOperator"] = [](const peg::SemanticValues &sv) -> QString {
        return QString::fromStdString(std::string(sv.sv()));
    };

    search["NormalMatcher"] = [](const peg::SemanticValues &sv) -> StringMatcher {
        auto target = std::any_cast<QString>(sv[0]);
        auto sanitizedTarget = QString(target);
        sanitizedTarget.replace("\\\"", "\"");
        sanitizedTarget.replace("\\'", "'");
        return [=](const QString &s) { return s.contains(sanitizedTarget, Qt::CaseInsensitive); };
    };

    search["RegexMatcher"] = [](const peg::SemanticValues &sv) -> StringMatcher {
        auto target = std::any_cast<QString>(sv[0]);
        auto regex = QRegularExpression(target, QRegularExpression::CaseInsensitiveOption);
        return [=](const QString &s) { return regex.match(s).hasMatch(); };
    };

    search["RegexMatcherString"] = [](const peg::SemanticValues &sv) -> QString {
        return QString::fromStdString(sv.token_to_string());
    };

    search["OracleQuery"] = [](const peg::SemanticValues &sv) -> Filter {
        const auto matcher = std::any_cast<StringMatcher>(sv[0]);
        return [=](const CardData &x) { return matcher(x->getText()); };
    };

    search["ColorQuery"] = [](const peg::SemanticValues &sv) -> Filter {
        QString parts;
        for (const auto &i : sv) {
            parts += std::any_cast<char>(i);
        }
        const bool identity = sv.tokens[0].empty() || sv.tokens[0][0] != 'i';
        if (sv.tokens[1][0] == ':') {
            return [=](const CardData &x) {
                QString match = identity ? x->getColors() : x->getProperty("coloridentity");
                if (parts.contains("m") && match.length() < 2) {
                    return false;
                }
                if (parts == "m") {
                    return true;
                }

                if (parts.contains("c") && match.length() == 0)
                    return true;

                auto containsColor = [&parts](const QString &s) { return parts.contains(s); };
                return std::any_of(match.begin(), match.end(), containsColor);
            };
        }

        return [=](const CardData &x) {
            QString match = identity ? x->getColors() : x->getProperty("colorIdentity");
            if (parts.contains("m") && match.length() < 2) {
                return false;
            }

            if (parts.contains("c") && match.length() != 0) {
                return false;
            }

            for (const auto &part : parts) {
                if (!match.contains(part)) {
                    return false;
                }
            }

            auto containsColor = [&parts](const QString &s) { return parts.contains(s); };
            return std::all_of(match.begin(), match.end(), containsColor);
        };
    };

    search["CMCQuery"] = [](const peg::SemanticValues &sv) -> Filter {
        const auto matcher = std::any_cast<NumberMatcher>(sv[0]);
        return [=](const CardData &x) -> bool { return matcher(x->getProperty("cmc").toInt()); };
    };
    search["PowerQuery"] = [](const peg::SemanticValues &sv) -> Filter {
        const auto matcher = std::any_cast<NumberMatcher>(sv[0]);
        return [=](const CardData &x) -> bool { return matcher(x->getPowTough().split("/")[0].toInt()); };
    };
    search["ToughnessQuery"] = [](const peg::SemanticValues &sv) -> Filter {
        const auto matcher = std::any_cast<NumberMatcher>(sv[0]);
        return [=](const CardData &x) -> bool {
            auto parts = x->getPowTough().split("/");
            return matcher(parts.length() == 2 ? parts[1].toInt() : 0);
        };
    };
    search["FieldQuery"] = [](const peg::SemanticValues &sv) -> Filter {
        const auto field = std::any_cast<QString>(sv[0]);
        if (sv.choice() == 0) {
            const auto matcher = std::any_cast<StringMatcher>(sv[1]);
            return [=](const CardData &x) -> bool { return x->hasProperty(field) && matcher(x->getProperty(field)); };
        }

        const auto matcher = std::any_cast<NumberMatcher>(sv[1]);
        return
            [=](const CardData &x) -> bool { return x->hasProperty(field) && matcher(x->getProperty(field).toInt()); };
    };
    search["GenericQuery"] = [](const peg::SemanticValues &sv) -> Filter {
        const auto matcher = std::any_cast<StringMatcher>(sv[0]);
        return [=](const CardData &x) { return matcher(x->getName()); };
    };

    search["Color"] = [](const peg::SemanticValues &sv) -> char { return "WUBRGU"[sv.choice()]; };
    search["ColorEx"] = [](const peg::SemanticValues &sv) -> char {
        return sv.choice() == 0 ? std::any_cast<char>(sv[0]) : *std::string(sv.sv()).c_str();
    };
}

FilterString::FilterString()
{
    result = [](const CardData &) -> bool { return false; };
    _error = "Not initialized";
}

FilterString::FilterString(const QString &expr)
{
    QByteArray ba = expr.simplified().toUtf8();

    std::call_once(init, setupParserRules);

    _error = QString();

    if (ba.isEmpty()) {
        result = [](const CardData &) -> bool { return true; };
        return;
    }

    search.set_logger([&](size_t /*ln*/, size_t col, const std::string &msg) {
        _error = QString("Error at position %1: %2").arg(col).arg(QString::fromStdString(msg));
    });

    if (!search.parse(ba.data(), result)) {
        qCInfo(FilterStringLog).nospace() << "FilterString error for " << expr << "; " << qPrintable(_error);
        result = [](const CardData &) -> bool { return false; };
    }
}
