#include "release_notes.h"

#include <QList>
#include <QRegularExpression>
#include <QStringList>
#include <optional>

namespace
{
constexpr auto APP_NOTES_START = "<!-- app-notes:start -->";
constexpr auto APP_NOTES_END = "<!-- app-notes:end -->";

constexpr auto DOT_ALL = QRegularExpression::DotMatchesEverythingOption;
constexpr auto CASE_INSENSITIVE = QRegularExpression::CaseInsensitiveOption;
constexpr auto MULTILINE = QRegularExpression::MultilineOption;

QString normalizeNewlines(QString text)
{
    text.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    text.replace(QLatin1Char('\r'), QLatin1Char('\n'));
    return text;
}

QString stripMatches(QString text, const QRegularExpression &expression)
{
    text.remove(expression);
    return text;
}

QString convertAlerts(QString text)
{
    static const QRegularExpression alert(
        QStringLiteral("^[ \\t]*>[ \\t]*\\[!(NOTE|TIP|IMPORTANT|WARNING|CAUTION)\\][ \\t]*$"),
        MULTILINE | CASE_INSENSITIVE);

    QList<QRegularExpressionMatch> matches;
    auto iterator = alert.globalMatch(text);
    while (iterator.hasNext()) {
        matches.append(iterator.next());
    }

    // Backwards, so the remaining match offsets stay valid while replacing.
    for (auto index = matches.size() - 1; index >= 0; --index) {
        const auto &match = matches.at(index);
        QString label = match.captured(1).toLower();
        if (!label.isEmpty()) {
            label[0] = label.at(0).toUpper();
        }
        text.replace(match.capturedStart(), match.capturedLength(), QStringLiteral("> **%1:**").arg(label));
    }
    return text;
}

bool isHeading(const QString &line)
{
    static const QRegularExpression heading(QStringLiteral("^#{1,6}[ \\t]+\\S"));
    return heading.match(line).hasMatch();
}

/**
 * Drops headings whose content was removed along with a folded block, so the
 * notes never end in a bare `### Fixed Bugs` with nothing under it.
 */
QString dropEmptyHeadings(const QString &text)
{
    const QStringList lines = text.split(QLatin1Char('\n'));
    QStringList kept;
    for (auto index = 0; index < lines.size(); ++index) {
        if (!isHeading(lines.at(index))) {
            kept.append(lines.at(index));
            continue;
        }

        auto next = index + 1;
        while (next < lines.size() && lines.at(next).trimmed().isEmpty()) {
            ++next;
        }
        if (next >= lines.size() || isHeading(lines.at(next))) {
            continue;
        }
        kept.append(lines.at(index));
    }
    return kept.join(QLatin1Char('\n'));
}

QString cleanSection(QString text)
{
    static const QRegularExpression htmlComment(QStringLiteral("<!--.*?-->"), DOT_ALL);
    static const QRegularExpression detailsBlock(QStringLiteral("<details[^>]*>.*?</details>"),
                                                 DOT_ALL | CASE_INSENSITIVE);
    static const QRegularExpression preBlock(QStringLiteral("<pre[^>]*>.*?</pre>"), DOT_ALL | CASE_INSENSITIVE);
    static const QRegularExpression repeatedBlankLines(QStringLiteral("\n{3,}"));

    text = stripMatches(text, htmlComment);
    text = stripMatches(text, detailsBlock);
    text = stripMatches(text, preBlock);
    text = convertAlerts(text);
    text = dropEmptyHeadings(text);
    text.replace(repeatedBlankLines, QStringLiteral("\n\n"));
    return text.trimmed();
}

std::optional<QString> markerBlock(const QString &text)
{
    const auto start = text.indexOf(QLatin1String(APP_NOTES_START));
    if (start < 0) {
        return std::nullopt;
    }

    const auto contentStart = start + static_cast<int>(QLatin1String(APP_NOTES_START).size());
    const auto end = text.indexOf(QLatin1String(APP_NOTES_END), contentStart);
    if (end < 0) {
        return std::nullopt;
    }

    return text.mid(contentStart, end - contentStart);
}

std::optional<QString> changelogSection(const QString &text)
{
    static const QRegularExpression changelog(QStringLiteral("^##[ \\t]+Changelog[ \\t]*\\n(.*?)(?=^##[ \\t]|\\z)"),
                                              DOT_ALL | MULTILINE | CASE_INSENSITIVE);
    const auto match = changelog.match(text);
    if (!match.hasMatch()) {
        return std::nullopt;
    }
    return match.captured(1);
}

std::optional<QString> detailsContent(const QString &text)
{
    static const QRegularExpression details(QStringLiteral("<details[^>]*>(.*?)</details>"),
                                            DOT_ALL | CASE_INSENSITIVE);
    static const QRegularExpression summary(QStringLiteral("<summary[^>]*>.*?</summary>"), DOT_ALL | CASE_INSENSITIVE);

    const auto match = details.match(text);
    if (!match.hasMatch()) {
        return std::nullopt;
    }

    QString content = match.captured(1);
    content.remove(summary);
    return content;
}
} // namespace

QString curatedReleaseNotes(const QString &body)
{
    const QString normalized = normalizeNewlines(body);

    // A marker block is authoritative even when it is empty: an uncurated
    // release must not silently fall back to the generated commit list.
    if (const auto block = markerBlock(normalized); block.has_value()) {
        return cleanSection(*block);
    }

    if (const auto changelog = changelogSection(normalized); changelog.has_value()) {
        return cleanSection(*changelog);
    }

    // Bodies published before the markers existed (betas in particular) only
    // carry the generated list, wrapped in a folded block.
    if (const auto content = detailsContent(normalized); content.has_value()) {
        return cleanSection(*content);
    }

    return {};
}
