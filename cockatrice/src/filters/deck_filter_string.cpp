#include "deck_filter_string.h"

#include "libcockatrice/card/card_localization.h"
#include "libcockatrice/card/database/card_database_querier.h"
#include "libcockatrice/deck_list/deck_list.h"
#include "libcockatrice/deck_list/tree/deck_list_card_node.h"

#include <QByteArray>
#include <QDebug>
#include <QFileInfo>
#include <QList>
#include <QMessageLogger>
#include <QSharedPointer>
#include <algorithm>
#include <any>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/deck_list/loaded_deck.h>
#include <libcockatrice/filters/filter_string.h>
#include <libcockatrice/utility/peglib.h>
#include <mutex>
#include <qnamespace.h>
#include <stddef.h>
#include <string>
#include <string_view>
#include <vector>

static peg::parser search(R"(
Start <- QueryPartList
~ws <- [ \t\r\n]+
QueryPartList <- ComplexQueryPart ( ws ("AND" ws)? ComplexQueryPart)* ws*

ComplexQueryPart <- SomewhatComplexQueryPart ws "OR" ws ComplexQueryPart / SomewhatComplexQueryPart
SomewhatComplexQueryPart <- [(] QueryPartList [)] / QueryPart

QueryPart <- NotQuery / DeckContentQuery / DeckNameQuery / FileNameQuery / PathQuery / FormatQuery / CommentQuery / GenericQuery

NotQuery <- ('NOT' ws/'-') SomewhatComplexQueryPart

DeckContentQuery <- CardSearch NumericExpression?
CardSearch <- '[[' CardFilterString ']]'
CardFilterString <- (!']]'.)*

DeckNameQuery <- ([Dd] 'eck')? [Nn] 'ame'? [:] String
FileNameQuery <- [Ff] ([Nn] / 'ile' ([Nn] 'ame')?) [:] String
PathQuery <- [Pp] 'ath'? [:] String
FormatQuery <- [Ff] 'ormat'? [:] String
CommentQuery <- [Cc] ('omment' 's'?)? [:] String

GenericQuery <- String

NonDoubleQuoteUnlessEscaped <- '\\\"'. / !["].
NonSingleQuoteUnlessEscaped <- "\\\'". / !['].
UnescapedStringListPart <- !['":<>()=! \t\r\n].
SingleApostropheString <- (UnescapedStringListPart+ ws*)* ['] (UnescapedStringListPart+ ws*)*

String <- SingleApostropheString / UnescapedStringListPart+ / ["] <NonDoubleQuoteUnlessEscaped*> ["] / ['] <NonSingleQuoteUnlessEscaped*> [']

NumericExpression <- NumericOperator ws? NumericValue
NumericOperator <- '!=' / '>=' / '<=' / [><=:]
NumericValue <- [0-9]+
)");

static std::once_flag init;

// Semantic actions receive per-parse data so a rejected nested query cannot be hidden by NOT or OR.
struct DeckFilterParseContext
{
    CardSearchLanguage searchLanguage;
    QString error;
};

using DeckNumberMatcher = std::function<bool(qint64)>;

static void setupParserRules()
{
    // plumbing
    auto passthru = [](const peg::SemanticValues &sv) -> DeckFilter {
        return !sv.empty() ? std::any_cast<DeckFilter>(sv[0]) : nullptr;
    };

    search["Start"] = passthru;
    search["QueryPartList"] = [](const peg::SemanticValues &sv) -> DeckFilter {
        return [=](const DeckSearchData &data) {
            auto matchesFilter = [&data](const std::any &query) { return std::any_cast<DeckFilter>(query)(data); };
            return std::all_of(sv.begin(), sv.end(), matchesFilter);
        };
    };
    search["ComplexQueryPart"] = [](const peg::SemanticValues &sv) -> DeckFilter {
        return [=](const DeckSearchData &data) {
            auto matchesFilter = [&data](const std::any &query) { return std::any_cast<DeckFilter>(query)(data); };
            return std::any_of(sv.begin(), sv.end(), matchesFilter);
        };
    };
    search["SomewhatComplexQueryPart"] = passthru;
    search["QueryPart"] = passthru;
    search["NotQuery"] = [](const peg::SemanticValues &sv) -> DeckFilter {
        const auto dependent = std::any_cast<DeckFilter>(sv[0]);
        return [=](const DeckSearchData &data) -> bool { return !dependent(data); };
    };

    search["String"] = [](const peg::SemanticValues &sv) -> QString {
        if (sv.choice() == 0) {
            return QString::fromStdString(std::string(sv.sv()));
        }

        return QString::fromStdString(std::string(sv.token(0)));
    };

    search["NumericExpression"] = [](const peg::SemanticValues &sv) -> DeckNumberMatcher {
        const auto arg = std::any_cast<qint64>(sv[1]);
        const auto op = std::any_cast<QString>(sv[0]);

        if (op == ">") {
            return [=](const qint64 s) { return s > arg; };
        }
        if (op == ">=") {
            return [=](const qint64 s) { return s >= arg; };
        }
        if (op == "<") {
            return [=](const qint64 s) { return s < arg; };
        }
        if (op == "<=") {
            return [=](const qint64 s) { return s <= arg; };
        }
        if (op == "=") {
            return [=](const qint64 s) { return s == arg; };
        }
        if (op == ":") {
            return [=](const qint64 s) { return s == arg; };
        }
        if (op == "!=") {
            return [=](const qint64 s) { return s != arg; };
        }
        return [](qint64) { return false; };
    };

    search["NumericValue"] = [](const peg::SemanticValues &sv, std::any &data) -> qint64 {
        bool valid = false;
        const auto number = QString::fromStdString(std::string(sv.sv())).toLongLong(&valid);
        if (!valid) {
            std::any_cast<DeckFilterParseContext &>(data).error = "Card count is outside the supported integer range.";
        }
        return number;
    };

    search["NumericOperator"] = [](const peg::SemanticValues &sv) -> QString {
        return QString::fromStdString(std::string(sv.sv()));
    };

    // actual functionality
    search["DeckContentQuery"] = [](const peg::SemanticValues &sv, std::any &parseData) -> DeckFilter {
        auto &context = std::any_cast<DeckFilterParseContext &>(parseData);
        auto cardFilter = FilterString(std::any_cast<QString>(sv[0]), context.searchLanguage);
        if (!cardFilter.valid()) {
            context.error = QString("Invalid card query: %1").arg(cardFilter.error());
        }
        auto numberMatcher =
            sv.size() > 1 ? std::any_cast<DeckNumberMatcher>(sv[1]) : [](qint64 count) { return count > 0; };

        return [=](const DeckSearchData &data) -> bool {
            qint64 count = 0;
            auto cardNodes = data.deck->deckList.getCardNodes();
            for (auto node : cardNodes) {
                auto cardInfoPtr = CardDatabaseManager::query()->getCardInfo(node->getName());
                if (node->getNumber() > 0 && !cardInfoPtr.isNull() && cardFilter.check(cardInfoPtr)) {
                    count += node->getNumber();
                }
            }
            return numberMatcher(count);
        };
    };

    search["CardSearch"] = [](const peg::SemanticValues &sv) -> QString { return std::any_cast<QString>(sv[0]); };

    search["CardFilterString"] = [](const peg::SemanticValues &sv) -> QString {
        return QString::fromStdString(std::string(sv.sv()));
    };

    search["DeckNameQuery"] = [](const peg::SemanticValues &sv) -> DeckFilter {
        auto name = std::any_cast<QString>(sv[0]);
        return [=](const DeckSearchData &data) {
            return data.deck->deckList.getName().contains(name, Qt::CaseInsensitive);
        };
    };

    search["FileNameQuery"] = [](const peg::SemanticValues &sv) -> DeckFilter {
        auto name = std::any_cast<QString>(sv[0]);
        return [=](const DeckSearchData &data) {
            auto filename = QFileInfo(data.filePath).fileName();
            return filename.contains(name, Qt::CaseInsensitive);
        };
    };

    search["PathQuery"] = [](const peg::SemanticValues &sv) -> DeckFilter {
        auto name = std::any_cast<QString>(sv[0]);
        return [=](const DeckSearchData &data) { return data.relativeFilePath.contains(name, Qt::CaseInsensitive); };
    };

    search["FormatQuery"] = [](const peg::SemanticValues &sv) -> DeckFilter {
        auto format = std::any_cast<QString>(sv[0]);
        return [=](const DeckSearchData &data) {
            auto gameFormat = data.deck->deckList.getGameFormat();
            return QString::compare(format, gameFormat, Qt::CaseInsensitive) == 0;
        };
    };

    search["CommentQuery"] = [](const peg::SemanticValues &sv) -> DeckFilter {
        auto value = std::any_cast<QString>(sv[0]);
        return [=](const DeckSearchData &data) {
            auto comments = data.deck->deckList.getComments();
            return comments.contains(value, Qt::CaseInsensitive);
        };
    };

    search["GenericQuery"] = [](const peg::SemanticValues &sv) -> DeckFilter {
        auto name = std::any_cast<QString>(sv[0]);
        return [=](const DeckSearchData &data) { return data.displayName.contains(name, Qt::CaseInsensitive); };
    };
}

DeckFilterString::DeckFilterString()
{
    filter = [](const DeckSearchData &) { return false; };
    _error = "Not initialized";
}

DeckFilterString::DeckFilterString(const QString &expr, const CardSearchLanguage &searchLanguage)
{
    // Trim only the query boundary: repeated spaces inside quoted names, paths and comments are significant.
    QByteArray ba = expr.trimmed().toUtf8();

    std::call_once(init, setupParserRules);

    _error = QString();

    if (ba.isEmpty()) {
        filter = [](const DeckSearchData &) { return true; };
        return;
    }

    std::any parseData = DeckFilterParseContext{searchLanguage, {}};

    search.set_logger([&](size_t /*ln*/, size_t col, const std::string &msg) {
        _error = QString("Error at position %1: %2").arg(col).arg(QString::fromStdString(msg));
    });

    const bool parsed = search.parse(std::string_view(ba.constData(), ba.size()), parseData, filter);
    // The parser keeps its logger between calls; do not leave a callback referring to this object behind.
    search.set_logger(peg::Log{});
    const auto &semanticError = std::any_cast<const DeckFilterParseContext &>(parseData).error;
    if (!semanticError.isEmpty()) {
        _error = semanticError;
    }
    if (!parsed || !_error.isEmpty()) {
        qCInfo(DeckFilterStringLog).nospace() << "DeckFilterString error for " << expr << "; " << qPrintable(_error);
        filter = [](const DeckSearchData &) { return false; };
    }
}
