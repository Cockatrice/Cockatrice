#include "filter_string.h"

#include "libcockatrice/card/card_info.h"
#include "libcockatrice/card/card_localization.h"
#include "libcockatrice/card/database/card_database.h"
#include "libcockatrice/card/printing/printing_info.h"
#include "libcockatrice/card/set/card_set.h"
#include "libcockatrice/card/set/card_set_list.h"

#include <QByteArray>
#include <QChar>
#include <QDate>
#include <QDebug>
#include <QList>
#include <QMessageLogger>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QSharedPointer>
#include <QString>
#include <QStringList>
#include <QStringView>
#include <algorithm>
#include <any>
#include <ctype.h>
#include <functional>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/card/game_specific_terms.h>
#include <libcockatrice/utility/peglib.h>
#include <mutex>
#include <qnamespace.h>
#include <stddef.h>
#include <string>
#include <string_view>
#include <vector>

static peg::parser search(R"(
Start <- QueryPartList
~ws <- [ ]+
QueryPartList <- ComplexQueryPart ( ws ("AND" ws)? ComplexQueryPart)* ws*

ComplexQueryPart <- SomewhatComplexQueryPart ws "OR" ws ComplexQueryPart / SomewhatComplexQueryPart

SomewhatComplexQueryPart <- [(] QueryPartList [)] / QueryPart

QueryPart <- NotQuery / SetQuery / RarityQuery / CMCQuery / FormatQuery / PowerQuery / ToughnessQuery / ColorQuery / TagQuery / TypeQuery / OracleQuery / FieldQuery / GenericQuery

NotQuery <- ('NOT' ws/'-') SomewhatComplexQueryPart
SetQuery <- ('e'/'set') ws? SetQueryValue
SetQueryValue <- ([=:] ws? FlexStringValue) / (<[!][=]?> ws? FlexStringValue) / SetExpression
OracleQuery <- 'o' ws? [:] ws? MatcherString


CMCQuery <- ('cmc'/'mv') ws? NumericExpression
PowerQuery <- [Pp] 'ow' 'er'? ws? NumericExpression
ToughnessQuery <- [Tt] 'ou' 'ghness'? ws? NumericExpression

RarityQuery <- [rR] ws? ':' ws? Rarity
Rarity <- [Cc] 'ommon'? / [Uu] 'ncommon'? / [Rr] 'are'? / [Mm] 'ythic'? / [Ss] 'pecial'? / [a-zA-Z] [a-z]*

FormatQuery <- 'f' ws? ':' ws? Format / Legality ws? ':' ws? Format
Format <- [a-zA-Z] [a-z]*
Legality <- [Ll] 'egal'? / [Bb] 'anned'? / [Rr] 'estricted'


TypeQuery <- [tT] 'ype'? ws? [:] ws? StringValue

TagQuery <- [tT]('ags'/'ag') ws? [:] ws? String

Color <- < [Ww] 'hite'? / [Uu] / [Bb] 'lack'? / [Rr] 'ed'? / [Gg] 'reen'?  / [Bb] 'lue'? >
ColorEx <- Color / [mc]

ColorQuery <- [cC] 'olor'? <[iI]?> ws? <[:!]> ws? ColorEx*

FieldQuery <- String ws? [:] ws? MatcherString / String ws? NumericExpression

NonDoubleQuoteUnlessEscaped <- '\\\"'. / !["].
NonSingleQuoteUnlessEscaped <- "\\\'". / !['].
UnescapedStringListPart <- !['":<>()=! ].
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

SetExpression <- NumericOperator ws? String

NumericExpression <- NumericOperator ws? NumericValue
NumericOperator <- [=:] / <[><!][=]?>
NumericValue <- [0-9]+
)");

static std::once_flag init;

// The peglib parser rules (and therefore their rule actions) are set up once per
// process, so a rule action cannot capture per-instance state. The card language
// plain-text name and text queries search in is therefore handed to the GenericQuery
// and OracleQuery rule actions through this thread-local context, which is live only
// while a FilterString is being parsed. The rule actions copy it into the filter
// closures they produce, so card evaluation never reads process-global state.
thread_local CardSearchLanguage searchLanguageContext;

namespace
{
bool matchesInSearchLanguage(const QString &english,
                             const QString &localized,
                             const CardSearchLanguage &searchLanguage,
                             const StringMatcher &matcher)
{
    if (searchLanguage.mode == SearchLanguageMode::English) {
        return matcher(english);
    }

    if (searchLanguage.mode == SearchLanguageMode::Both) {
        if (!searchLanguage.isEnglishOnly() && matcher(localized)) {
            return true;
        }
        return matcher(english);
    }

    return searchLanguage.isEnglishOnly() ? matcher(english) : matcher(localized);
}
} // namespace

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
    search["TagQuery"] = [](const peg::SemanticValues &sv) -> Filter {
        // Tags are stored space-separated, so matching whole tokens keeps
        // `tags:ram` from matching `ramp`. Combine tags with AND:
        // `tags:draw tags:ramp`.
        const auto tag = std::any_cast<QString>(sv[0]).trimmed();
        return [=](const CardData &x) -> bool {
            const QString stored = x->getProperty(Mtg::Tags);
            if (stored.isEmpty() || tag.isEmpty()) {
                return false;
            }
            return stored.split(" ", Qt::SkipEmptyParts).contains(tag, Qt::CaseInsensitive);
        };
    };
    search["SetQueryValue"] = [](const peg::SemanticValues &sv) -> Filter {
        if (sv.choice() == 0) {
            auto matcher = std::any_cast<StringMatcher>(sv[0]);
            return [=](const CardData &x) -> bool {
                const auto &sets = x->getSets();
                return std::any_of(sets.keyValueBegin(), sets.keyValueEnd(),
                                   [&matcher](const auto &set) { return matcher(set.first); });
            };
        }
        if (sv.choice() == 1) {
            auto matcher = std::any_cast<StringMatcher>(sv[0]);
            return [=](const CardData &x) -> bool {
                const auto &sets = x->getSets();
                return std::none_of(sets.keyValueBegin(), sets.keyValueEnd(),
                                    [&matcher](const auto &set) { return matcher(set.first); });
            };
        }

        auto matcher = std::any_cast<NumberMatcher>(sv[0]);
        return [=](const CardData &x) -> bool {
            const auto &sets = x->getSets();
            return std::any_of(sets.constBegin(), sets.constEnd(), [&matcher](const auto &printings) {
                return std::any_of(printings.begin(), printings.end(), [&matcher](const PrintingInfo &printing) {
                    const CardSetPtr set = printing.getSet();
                    return set && set->getEnabled() && matcher(set->getReleaseDate().toJulianDay());
                });
            });
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
            for (const auto &printings : x->getSets()) {
                for (const auto &printing : printings) {
                    if (rarity == printing.getProperty("rarity")) {
                        return true;
                    }
                }
            }
            return false;
        };
    };

    search["FormatQuery"] = [](const peg::SemanticValues &sv) -> Filter {
        if (sv.choice() == 0) {
            const auto format = std::any_cast<QString>(sv[0]);
            return [=](const CardData &x) -> bool { return x->getLegalityProp(format) == "legal"; };
        }

        const auto format = std::any_cast<QString>(sv[1]);
        const auto legality = std::any_cast<QString>(sv[0]);
        return [=](const CardData &x) -> bool { return x->getLegalityProp(format) == legality; };
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

    search["NumericOperator"] = [](const peg::SemanticValues &sv) -> NumberComparer {
        const auto op = QString::fromStdString(std::string(sv.sv()));

        if (op == ">") {
            return [=](const int s, const int arg) { return s > arg; };
        }
        if (op == ">=") {
            return [=](const int s, const int arg) { return s >= arg; };
        }
        if (op == "<") {
            return [=](const int s, const int arg) { return s < arg; };
        }
        if (op == "<=") {
            return [=](const int s, const int arg) { return s <= arg; };
        }
        if (op == "=") {
            return [=](const int s, const int arg) { return s == arg; };
        }
        if (op == ":") {
            return [=](const int s, const int arg) { return s == arg; };
        }
        if (op == "!=") {
            return [=](const int s, const int arg) { return s != arg; };
        }
        return [](int, int) { return false; };
    };

    search["NumericValue"] = [](const peg::SemanticValues &sv) -> int {
        return QString::fromStdString(std::string(sv.sv())).toInt();
    };

    search["NumericExpression"] = [](const peg::SemanticValues &sv) -> NumberMatcher {
        const auto comparer = std::any_cast<NumberComparer>(sv[0]);
        const auto arg = std::any_cast<int>(sv[1]);
        return [=](int s) { return comparer(s, arg); };
    };

    search["SetExpression"] = [](const peg::SemanticValues &sv) -> NumberMatcher {
        const auto comparer = std::any_cast<NumberComparer>(sv[0]);
        const auto setCode = std::any_cast<QString>(sv[1]);
        const auto allSets = CardDatabaseManager::getInstance()->getSetList();
        for (auto &set : allSets) {
            if (set->getShortName() == setCode) {
                const int releaseDate = set->getReleaseDate().toJulianDay();
                return [=](int s) { return comparer(s, releaseDate); };
            }
        }
        return [](int) { return false; };
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
        const CardSearchLanguage searchLanguage = searchLanguageContext;
        return [=](const CardData &x) {
            return matchesInSearchLanguage(x->getText(), x->getLocalizedText(searchLanguage.language), searchLanguage,
                                           matcher);
        };
    };

    search["ColorQuery"] = [](const peg::SemanticValues &sv) -> Filter {
        QString parts;
        for (const auto &i : sv) {
            parts += std::any_cast<char>(i);
        }
        const bool identity = sv.tokens[0].empty() || sv.tokens[0][0] != 'i';
        if (sv.tokens[1][0] == ':') {
            return [=](const CardData &x) {
                const QString match = identity ? x->getColors() : x->getColorIdentity();
                if (parts.contains("m") && match.length() < 2) {
                    return false;
                }
                if (parts == "m") {
                    return true;
                }

                if (parts.contains("c") && match.length() == 0) {
                    return true;
                }

                auto containsColor = [&parts](const QString &s) { return parts.contains(s); };
                return std::any_of(match.begin(), match.end(), containsColor);
            };
        }

        return [=](const CardData &x) {
            const QString match = identity ? x->getColors() : x->getColorIdentity();
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
        return [=](const CardData &x) -> bool {
            const QString &powTough = x->getPowTough();
            const qsizetype slash = powTough.indexOf('/');
            return matcher(slash < 0 ? powTough.toInt() : QStringView(powTough).left(slash).toInt());
        };
    };
    search["ToughnessQuery"] = [](const peg::SemanticValues &sv) -> Filter {
        const auto matcher = std::any_cast<NumberMatcher>(sv[0]);
        return [=](const CardData &x) -> bool {
            const QString &powTough = x->getPowTough();
            const qsizetype slash = powTough.indexOf('/');
            return matcher(slash < 0 ? 0 : QStringView(powTough).mid(slash + 1).toInt());
        };
    };
    search["FieldQuery"] = [](const peg::SemanticValues &sv) -> Filter {
        const auto field = std::any_cast<QString>(sv[0]);
        if (sv.choice() == 0) {
            const auto matcher = std::any_cast<StringMatcher>(sv[1]);
            return [=](const CardData &x) -> bool {
                const auto &properties = x->getPropertiesHash();
                const auto it = properties.constFind(field);
                return it != properties.constEnd() && matcher(it.value());
            };
        }

        const auto matcher = std::any_cast<NumberMatcher>(sv[1]);
        return [=](const CardData &x) -> bool {
            const auto &properties = x->getPropertiesHash();
            const auto it = properties.constFind(field);
            return it != properties.constEnd() && matcher(it.value().toInt());
        };
    };
    search["GenericQuery"] = [](const peg::SemanticValues &sv) -> Filter {
        const auto matcher = std::any_cast<StringMatcher>(sv[0]);
        const CardSearchLanguage searchLanguage = searchLanguageContext;
        return [=](const CardData &x) {
            return matchesInSearchLanguage(x->getName(), x->getLocalizedName(searchLanguage.language), searchLanguage,
                                           matcher);
        };
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

FilterString::FilterString(const QString &expr, const CardSearchLanguage &searchLanguage)
{
    QByteArray ba = expr.simplified().toUtf8();

    std::call_once(init, setupParserRules);

    _error = QString();

    if (ba.isEmpty()) {
        result = [](const CardData &) -> bool { return true; };
        return;
    }

    searchLanguageContext = searchLanguage;

    search.set_logger([&](size_t /*ln*/, size_t col, const std::string &msg) {
        _error = QString("Error at position %1: %2").arg(col).arg(QString::fromStdString(msg));
    });

    if (!search.parse(ba.data(), result)) {
        qCInfo(FilterStringLog).nospace() << "FilterString error for " << expr << "; " << qPrintable(_error);
        result = [](const CardData &) -> bool { return false; };
    }
}
