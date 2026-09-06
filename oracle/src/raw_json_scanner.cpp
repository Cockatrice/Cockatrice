#include "raw_json_scanner.h"

#include <cstring>

namespace
{

// Nesting cap matching QJsonDocument's limit, so a pathologically deep document
// fails shallowly instead of overflowing the stack through the recursive
// skipValue/skipArray/skipObject walk (Qt's parser caps at 1024 for the same
// reason and reports DeepNesting).
constexpr int kMaxNestingDepth = 1024;

inline bool isWhitespace(char c)
{
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

const char *skipWhitespace(const char *p, const char *end)
{
    while (p < end && isWhitespace(*p)) {
        ++p;
    }
    return p;
}

inline bool isHexDigit(char c)
{
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

inline quint8 hexValue(char c)
{
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    return c - 'A' + 10;
}

/**
 * @brief Skips past a JSON string without decoding it, validating escapes.
 * @param p In: pointing at the opening quote. Out: pointing past the closing quote.
 */
bool skipString(const char *&p, const char *end)
{
    ++p; // opening quote
    for (;;) {
        const void *quote = memchr(p, '"', static_cast<size_t>(end - p));
        if (!quote) {
            return false; // unterminated string
        }
        // Backslash escapes can only appear before the closing quote, so bound
        // the scan to the string extent instead of the rest of the document.
        const void *backslash = memchr(p, '\\', static_cast<size_t>(static_cast<const char *>(quote) - p));
        if (!backslash) {
            p = static_cast<const char *>(quote) + 1;
            return true;
        }
        const char *b = static_cast<const char *>(backslash);
        if (end - b < 2) {
            return false;
        }
        const char escaped = b[1];
        if (escaped == 'u') {
            if (end - b < 6) {
                return false;
            }
            quint32 codepoint = 0;
            for (int i = 0; i < 4; ++i) {
                if (!isHexDigit(b[2 + i])) {
                    return false;
                }
                codepoint = codepoint * 16 + hexValue(b[2 + i]);
            }
            p = b + 6;
            if (codepoint >= 0xD800 && codepoint <= 0xDBFF) {
                // expect the low-surrogate escape for the second half
                if (end - p < 6 || p[0] != '\\' || p[1] != 'u') {
                    return false; // unpaired high surrogate
                }
                quint32 low = 0;
                for (int i = 0; i < 4; ++i) {
                    if (!isHexDigit(p[2 + i])) {
                        return false;
                    }
                    low = low * 16 + hexValue(p[2 + i]);
                }
                if (low < 0xDC00 || low > 0xDFFF) {
                    return false;
                }
                p += 6;
            } else if (codepoint >= 0xDC00 && codepoint <= 0xDFFF) {
                return false; // unpaired low surrogate
            }
            continue;
        }
        switch (escaped) {
            case '"':
            case '\\':
            case '/':
            case 'b':
            case 'f':
            case 'n':
            case 'r':
            case 't':
                p = b + 2;
                continue;
            default:
                return false; // invalid escape
        }
    }
}

/**
 * @brief Decodes a JSON string into @p out, validating it as it goes.
 * @param p In: pointing at the opening quote. Out: pointing past the closing quote.
 */
bool decodeString(const char *&p, const char *end, QString &out)
{
    out.clear();
    QByteArray utf8;
    auto flush = [&out, &utf8]() {
        if (!utf8.isEmpty()) {
            out += QString::fromUtf8(utf8);
            utf8.clear();
        }
    };

    ++p; // opening quote
    while (p < end) {
        const char c = *p;
        if (c == '\\') {
            flush();
            ++p; // escaped character
            if (p >= end) {
                return false;
            }
            const char escaped = *p;
            if (escaped == 'u') {
                ++p; // first hex digit
                if (p + 4 > end) {
                    return false;
                }
                quint32 codepoint = 0;
                for (int i = 0; i < 4; ++i) {
                    if (!isHexDigit(p[i])) {
                        return false;
                    }
                    codepoint = codepoint * 16 + hexValue(p[i]);
                }
                p += 4;
                if (codepoint >= 0xD800 && codepoint <= 0xDBFF) {
                    // expect a low-surrogate escape for the second half
                    if (p + 6 > end || p[0] != '\\' || p[1] != 'u') {
                        return false; // unpaired high surrogate
                    }
                    quint32 low = 0;
                    for (int i = 0; i < 4; ++i) {
                        if (!isHexDigit(p[2 + i])) {
                            return false;
                        }
                        low = low * 16 + hexValue(p[2 + i]);
                    }
                    if (low < 0xDC00 || low > 0xDFFF) {
                        return false;
                    }
                    out += QChar(codepoint);
                    out += QChar(low);
                    p += 6;
                } else if (codepoint >= 0xDC00 && codepoint <= 0xDFFF) {
                    return false; // unpaired low surrogate
                } else {
                    out += QChar(codepoint);
                }
                continue;
            }
            switch (escaped) {
                case '"':
                    out += '"';
                    break;
                case '\\':
                    out += '\\';
                    break;
                case '/':
                    out += '/';
                    break;
                case 'b':
                    out += '\b';
                    break;
                case 'f':
                    out += '\f';
                    break;
                case 'n':
                    out += '\n';
                    break;
                case 'r':
                    out += '\r';
                    break;
                case 't':
                    out += '\t';
                    break;
                default:
                    return false;
            }
            ++p;
            continue;
        }
        if (c == '"') {
            ++p;
            flush();
            return true;
        }
        // Deliberately accept unescaped control characters (e.g. a tab inside
        // a set name): QJsonDocument and skipString accept them too, so
        // rejecting them here would fail the whole document on a byte that
        // Qt is fine with — the very total-failure mode this scanner avoids.
        utf8 += c;
        ++p;
    }
    return false;
}

/**
 * @brief Reads a set-metadata field, tolerating null and non-string values.
 *
 * A set's metadata may carry null or non-string values in otherwise-valid
 * payloads ("releaseDate": null, "type": 7). The token itself was already
 * structurally validated by skipValue, so a non-string value is accepted and
 * leaves @p out at its default (empty) — one bad set must not abort the
 * import of every other set in the document.
 */
bool decodeStringMember(const char *&fs, const char *&fe, QString &out)
{
    if (fs >= fe) {
        return false;
    }
    if (*fs != '"') {
        return true;
    }
    return decodeString(fs, fe, out);
}

bool matchLiteral(const char *&p, const char *end, const char *literal, int length)
{
    if (end - p < length || memcmp(p, literal, static_cast<size_t>(length)) != 0) {
        return false;
    }
    const char *after = p + length;
    if (after < end && (QChar::isLetter(*after) || QChar::isDigit(*after) || *after == '_')) {
        return false;
    }
    p = after;
    return true;
}

bool skipNumber(const char *&p, const char *end)
{
    // JSON number: -?(0|[1-9][0-9]*)(\.[0-9]+)?([eE][+-]?[0-9]+)?
    if (p < end && *p == '-') {
        ++p;
    }
    if (p < end && *p == '0') {
        ++p;
    } else if (p < end && *p >= '1' && *p <= '9') {
        ++p;
        while (p < end && QChar::isDigit(*p)) {
            ++p;
        }
    } else {
        return false;
    }
    if (p < end && *p == '.') {
        ++p;
        if (p >= end || !QChar::isDigit(*p)) {
            return false;
        }
        while (p < end && QChar::isDigit(*p)) {
            ++p;
        }
    }
    if (p < end && (*p == 'e' || *p == 'E')) {
        ++p;
        if (p < end && (*p == '+' || *p == '-')) {
            ++p;
        }
        if (p >= end || !QChar::isDigit(*p)) {
            return false;
        }
        while (p < end && QChar::isDigit(*p)) {
            ++p;
        }
    }
    return true;
}

bool skipValue(const char *&p, const char *end, int depth);
bool skipObject(const char *&p, const char *end, int depth);
bool skipArray(const char *&p, const char *end, int depth);

bool skipPrimitive(const char *&p, const char *end)
{
    if (p >= end) {
        return false;
    }
    const char c = *p;
    if (c == '"') {
        return skipString(p, end);
    }
    if (c == 't') {
        return matchLiteral(p, end, "true", 4);
    }
    if (c == 'f') {
        return matchLiteral(p, end, "false", 5);
    }
    if (c == 'n') {
        return matchLiteral(p, end, "null", 4);
    }
    if (c == '-' || (c >= '0' && c <= '9')) {
        return skipNumber(p, end);
    }
    return false;
}

bool skipObject(const char *&p, const char *end, int depth)
{
    if (depth <= 0) {
        return false; // nest deeper than the cap
    }
    ++p; // '{'
    p = skipWhitespace(p, end);
    if (p < end && *p == '}') {
        ++p;
        return true;
    }
    for (;;) {
        p = skipWhitespace(p, end);
        if (p >= end || *p != '"') {
            return false;
        }
        if (!skipString(p, end)) {
            return false;
        }
        p = skipWhitespace(p, end);
        if (p >= end || *p != ':') {
            return false;
        }
        ++p;
        if (!skipValue(p, end, depth - 1)) {
            return false;
        }
        p = skipWhitespace(p, end);
        if (p >= end) {
            return false;
        }
        if (*p == ',') {
            ++p;
            continue;
        }
        if (*p == '}') {
            ++p;
            return true;
        }
        return false;
    }
}

bool skipArray(const char *&p, const char *end, int depth)
{
    if (depth <= 0) {
        return false; // nest deeper than the cap
    }
    ++p; // '['
    p = skipWhitespace(p, end);
    if (p < end && *p == ']') {
        ++p;
        return true;
    }
    for (;;) {
        if (!skipValue(p, end, depth - 1)) {
            return false;
        }
        p = skipWhitespace(p, end);
        if (p >= end) {
            return false;
        }
        if (*p == ',') {
            ++p;
            continue;
        }
        if (*p == ']') {
            ++p;
            return true;
        }
        return false;
    }
}

bool skipValue(const char *&p, const char *end, int depth)
{
    p = skipWhitespace(p, end);
    if (p >= end) {
        return false;
    }
    const char c = *p;
    if (c == '{') {
        // pass depth through: skipObject consumes the single decrement for this level
        return skipObject(p, end, depth);
    }
    if (c == '[') {
        return skipArray(p, end, depth);
    }
    // a primitive is a leaf, so it never wastes a nesting level
    return skipPrimitive(p, end);
}

/**
 * @brief Iterates the members of the object starting at @p p.
 *
 * For each member invokes @p memberCallback with the key and the byte range of
 * its value. Advancing @p p is unaffected by the callback.
 */
template <typename F> bool forEachObjectMember(const char *&p, const char *end, int depth, F &&memberCallback)
{
    if (depth <= 0) {
        return false; // nest deeper than the cap
    }
    ++p; // '{'
    p = skipWhitespace(p, end);
    if (p < end && *p == '}') {
        ++p;
        return true;
    }
    for (;;) {
        p = skipWhitespace(p, end);
        if (p >= end || *p != '"') {
            return false;
        }
        QString key;
        if (!decodeString(p, end, key)) {
            return false;
        }
        p = skipWhitespace(p, end);
        if (p >= end || *p != ':') {
            return false;
        }
        ++p;
        const char *valueStart = skipWhitespace(p, end);
        const char *valueEnd = valueStart;
        if (!skipValue(valueEnd, end, depth - 1)) {
            return false;
        }
        if (!memberCallback(key, valueStart, valueEnd)) {
            return false;
        }
        p = valueEnd;
        p = skipWhitespace(p, end);
        if (p >= end) {
            return false;
        }
        if (*p == ',') {
            ++p;
            continue;
        }
        if (*p == '}') {
            ++p;
            return true;
        }
        return false;
    }
}

// Counts the direct elements of an array value; returns -1 if the array is malformed.
int countArrayElements(const char *p, const char *end, int depth)
{
    if (depth <= 0) {
        return -1; // nest deeper than the cap
    }
    ++p; // '['
    p = skipWhitespace(p, end);
    int count = 0;
    if (p < end && *p == ']') {
        return 0;
    }
    for (;;) {
        if (!skipValue(p, end, depth - 1)) {
            return -1;
        }
        ++count;
        p = skipWhitespace(p, end);
        if (p >= end) {
            return -1;
        }
        if (*p == ',') {
            ++p;
            continue;
        }
        if (*p == ']') {
            return count;
        }
        return -1;
    }
}

} // namespace

namespace RawJson
{

QList<SetRange> scanSetRanges(const QByteArray &json, ScanError *error)
{
    QList<SetRange> ranges;
    if (error) {
        *error = ScanError{};
    }

    const auto fail = [&](const QString &message) -> QList<SetRange> {
        if (error) {
            error->message = message;
        }
        return {};
    };

    const char *begin = json.constData();
    const char *end = begin + json.size();
    if (begin >= end) {
        return fail(QStringLiteral("empty JSON document"));
    }

    const char *p = skipWhitespace(begin, end);
    if (p >= end || *p != '{') {
        return fail(QStringLiteral("top-level JSON must be an object"));
    }

    bool foundData = false;
    bool malformedSetData = false;

    const auto topLevelCallback = [&](const QString &key, const char *valueStart, const char *valueEnd) {
        if (key == QStringLiteral("data")) {
            foundData = true;
            if (valueStart >= valueEnd || *valueStart != '{') {
                malformedSetData = true;
                return false;
            }
            const char *setP = valueStart;
            const bool ok = forEachObjectMember(setP, valueEnd, kMaxNestingDepth - 1,
                                                [&](const QString &setCode, const char *setStart, const char *setEnd) {
                                                    if (setStart >= setEnd || *setStart != '{') {
                                                        malformedSetData = true;
                                                        return false;
                                                    }
                                                    SetRange range;
                                                    range.dataRange.start = setStart - begin;
                                                    range.dataRange.length = setEnd - setStart;
                                                    range.code = setCode;

                                                    const char *memberP = setStart;
                                                    const bool metaOk = forEachObjectMember(
                                                        memberP, setEnd, kMaxNestingDepth - 2,
                                                        [&](const QString &field, const char *fs, const char *fe) {
                                                            if (field == QStringLiteral("code")) {
                                                                return decodeStringMember(fs, fe, range.code);
                                                            }
                                                            if (field == QStringLiteral("name")) {
                                                                return decodeStringMember(fs, fe, range.name);
                                                            }
                                                            if (field == QStringLiteral("type")) {
                                                                return decodeStringMember(fs, fe, range.type);
                                                            }
                                                            if (field == QStringLiteral("releaseDate")) {
                                                                return decodeStringMember(fs, fe, range.releaseDate);
                                                            }
                                                            if (field == QStringLiteral("cards")) {
                                                                if (fs >= fe) {
                                                                    return false;
                                                                }
                                                                if (*fs != '[') {
                                                                    // e.g. "cards": null — treat as an empty array,
                                                                    // matching Qt's tolerance.
                                                                    return true;
                                                                }
                                                                range.dataRange.cardCount =
                                                                    countArrayElements(fs, fe, kMaxNestingDepth - 2);
                                                                return range.dataRange.cardCount >= 0;
                                                            }
                                                            return true;
                                                        });
                                                    if (!metaOk) {
                                                        malformedSetData = true;
                                                        return false;
                                                    }
                                                    ranges.append(range);
                                                    return true;
                                                });
            if (!ok) {
                malformedSetData = true;
                return false;
            }
        }
        return true;
    };

    if (!forEachObjectMember(p, end, kMaxNestingDepth, topLevelCallback)) {
        return fail(malformedSetData ? QStringLiteral("malformed set data") : QStringLiteral("malformed JSON"));
    }
    p = skipWhitespace(p, end);
    if (p != end) {
        return fail(QStringLiteral("trailing content after top-level JSON object"));
    }
    if (!foundData) {
        return fail(QStringLiteral("missing \"data\" object"));
    }
    if (ranges.isEmpty()) {
        return fail(QStringLiteral("no sets found in \"data\""));
    }
    return ranges;
}

} // namespace RawJson