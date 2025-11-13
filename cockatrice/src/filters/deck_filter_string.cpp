#include "deck_filter_string.h"

#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/filters/filter_string.h>
#include <libcockatrice/utility/peglib.h>

static peg::parser search(R"(
Start <- QueryPartList
~ws <- [ ]+
QueryPartList <- ComplexQueryPart ( ws ("AND" ws)? ComplexQueryPart)* ws*

ComplexQueryPart <- SomewhatComplexQueryPart ws "OR" ws ComplexQueryPart / SomewhatComplexQueryPart
SomewhatComplexQueryPart <- [(] QueryPartList [)] / QueryPart

QueryPart <- NotQuery / DeckContentQuery / DeckNameQuery / FileNameQuery / PathQuery / GenericQuery

NotQuery <- ('NOT' ws/'-') SomewhatComplexQueryPart

DeckContentQuery <- CardSearch NumericExpression?
CardSearch <- '[[' CardFilterString ']]'
CardFilterString <- (!']]'.)*

DeckNameQuery <- ([Dd] 'eck')? [Nn] 'ame'? [:] String
FileNameQuery <- [Ff] ('ile' 'name'?)? [:] String
PathQuery <- [Pp] 'ath'? [:] String

GenericQuery <- String

NonDoubleQuoteUnlessEscaped <- '\\\"'. / !["].
NonSingleQuoteUnlessEscaped <- "\\\'". / !['].
UnescapedStringListPart <- !['":<>=! ].
SingleApostropheString <- (UnescapedStringListPart+ ws*)* ['] (UnescapedStringListPart+ ws*)*

String <- SingleApostropheString / UnescapedStringListPart+ / ["] <NonDoubleQuoteUnlessEscaped*> ["] / ['] <NonSingleQuoteUnlessEscaped*> [']

NumericExpression <- NumericOperator ws? NumericValue
NumericOperator <- [=:] / <[><!][=]?>
NumericValue <- [0-9]+
)");

static std::once_flag init;

static void setupParserRules()
{
    // plumbing
    auto passthru = [](const peg::SemanticValues &sv) -> DeckFilter {
        return !sv.empty() ? std::any_cast<DeckFilter>(sv[0]) : nullptr;
    };

    search["Start"] = passthru;
    search["QueryPartList"] = [](const peg::SemanticValues &sv) -> DeckFilter {
        return [=](const DeckPreviewWidget *deck, const ExtraDeckSearchInfo &info) {
            auto matchesFilter = [&deck, &info](const std::any &query) {
                return std::any_cast<DeckFilter>(query)(deck, info);
            };
            return std::all_of(sv.begin(), sv.end(), matchesFilter);
        };
    };
    search["ComplexQueryPart"] = [](const peg::SemanticValues &sv) -> DeckFilter {
        return [=](const DeckPreviewWidget *deck, const ExtraDeckSearchInfo &info) {
            auto matchesFilter = [&deck, &info](const std::any &query) {
                return std::any_cast<DeckFilter>(query)(deck, info);
            };
            return std::any_of(sv.begin(), sv.end(), matchesFilter);
        };
    };
    search["SomewhatComplexQueryPart"] = passthru;
    search["QueryPart"] = passthru;
    search["NotQuery"] = [](const peg::SemanticValues &sv) -> DeckFilter {
        const auto dependent = std::any_cast<DeckFilter>(sv[0]);
        return [=](const DeckPreviewWidget *deck, const ExtraDeckSearchInfo &info) -> bool {
            return !dependent(deck, info);
        };
    };

    search["String"] = [](const peg::SemanticValues &sv) -> QString {
        if (sv.choice() == 0) {
            return QString::fromStdString(std::string(sv.sv()));
        }

        return QString::fromStdString(std::string(sv.token(0)));
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

    // actual functionality
    search["DeckContentQuery"] = [](const peg::SemanticValues &sv) -> DeckFilter {
        auto cardFilter = FilterString(std::any_cast<QString>(sv[0]));
        auto numberMatcher = sv.size() > 1 ? std::any_cast<NumberMatcher>(sv[1]) : [](int count) { return count > 0; };

        return [=](const DeckPreviewWidget *deck, const ExtraDeckSearchInfo &) -> bool {
            int count = 0;
            deck->deckLoader->getDeckList()->forEachCard([&](InnerDecklistNode *, const DecklistCardNode *node) {
                auto cardInfoPtr = CardDatabaseManager::query()->getCardInfo(node->getName());
                if (!cardInfoPtr.isNull() && cardFilter.check(cardInfoPtr)) {
                    count += node->getNumber();
                }
            });
            return numberMatcher(count);
        };
    };

    search["CardSearch"] = [](const peg::SemanticValues &sv) -> QString { return std::any_cast<QString>(sv[0]); };

    search["CardFilterString"] = [](const peg::SemanticValues &sv) -> QString {
        return QString::fromStdString(std::string(sv.sv()));
    };

    search["DeckNameQuery"] = [](const peg::SemanticValues &sv) -> DeckFilter {
        auto name = std::any_cast<QString>(sv[0]);
        return [=](const DeckPreviewWidget *deck, const ExtraDeckSearchInfo &) {
            return deck->deckLoader->getDeckList()->getName().contains(name, Qt::CaseInsensitive);
        };
    };

    search["FileNameQuery"] = [](const peg::SemanticValues &sv) -> DeckFilter {
        auto name = std::any_cast<QString>(sv[0]);
        return [=](const DeckPreviewWidget *deck, const ExtraDeckSearchInfo &) {
            auto filename = QFileInfo(deck->filePath).fileName();
            return filename.contains(name, Qt::CaseInsensitive);
        };
    };

    search["PathQuery"] = [](const peg::SemanticValues &sv) -> DeckFilter {
        auto name = std::any_cast<QString>(sv[0]);
        return [=](const DeckPreviewWidget *, const ExtraDeckSearchInfo &info) {
            return info.relativeFilePath.contains(name, Qt::CaseInsensitive);
        };
    };

    search["GenericQuery"] = [](const peg::SemanticValues &sv) -> DeckFilter {
        auto name = std::any_cast<QString>(sv[0]);
        return [=](const DeckPreviewWidget *deck, const ExtraDeckSearchInfo &) {
            return deck->getDisplayName().contains(name, Qt::CaseInsensitive);
        };
    };
}

DeckFilterString::DeckFilterString()
{
    filter = [](const DeckPreviewWidget *, const ExtraDeckSearchInfo &) { return false; };
    _error = "Not initialized";
}

DeckFilterString::DeckFilterString(const QString &expr)
{
    QByteArray ba = expr.simplified().toUtf8();

    std::call_once(init, setupParserRules);

    _error = QString();

    if (ba.isEmpty()) {
        filter = [](const DeckPreviewWidget *, const ExtraDeckSearchInfo &) { return true; };
        return;
    }

    search.set_logger([&](size_t /*ln*/, size_t col, const std::string &msg) {
        _error = QString("Error at position %1: %2").arg(col).arg(QString::fromStdString(msg));
    });

    if (!search.parse(ba.data(), filter)) {
        qCInfo(DeckFilterStringLog).nospace() << "DeckFilterString error for " << expr << "; " << qPrintable(_error);
        filter = [](const DeckPreviewWidget *, const ExtraDeckSearchInfo &) { return false; };
    }
}