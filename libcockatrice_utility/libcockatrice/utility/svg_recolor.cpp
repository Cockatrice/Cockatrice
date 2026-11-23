#include "svg_recolor.h"

#include <QByteArrayView>

namespace
{
constexpr QByteArrayView SENTINEL = "currentColor";

/// Case-insensitive byte search for the sentinel, starting at @p from.
/// QByteArray::indexOf() only matches case-sensitively for QByteArrayView, and
/// its char overload cannot match a word.
qsizetype findSentinel(const QByteArray &bytes, qsizetype from)
{
    // QByteArrayView::sliced() is a view, so the scan does not allocate a copy
    // at every byte position (QByteArray::sliced() would).
    const QByteArrayView view(bytes);
    for (qsizetype i = from; i + SENTINEL.size() <= view.size(); ++i) {
        if (view.sliced(i, SENTINEL.size()).compare(SENTINEL, Qt::CaseInsensitive) == 0) {
            return i;
        }
    }
    return -1;
}
} // namespace

QByteArray resolveSvgColorSentinel(const QByteArray &svg, const QByteArray &color)
{
    // The sentinel and the replacement are both ASCII, so stay in bytes: a
    // QString round trip would decode the whole document (and mangle bytes that
    // are not valid UTF-8) even on the no-op path.
    qsizetype pos = findSentinel(svg, 0);
    if (pos < 0) {
        return svg;
    }

    QByteArray resolved = svg;
    while (pos >= 0) {
        resolved.replace(pos, SENTINEL.size(), color);
        // Skip past the inserted color: a caller's color never contains the
        // sentinel, and this keeps the scan linear.
        pos = findSentinel(resolved, pos + color.size());
    }
    return resolved;
}

QByteArray resolveSvgStrokeWidth(const QByteArray &svg, double width)
{
    constexpr QByteArrayView ATTRIBUTE = "stroke-width";
    constexpr QByteArrayView DOUBLE_QUOTE = "\"";
    constexpr QByteArrayView SINGLE_QUOTE = "'";
    const QByteArray value = QByteArray::number(width, 'f', 2);

    QByteArray resolved;
    resolved.reserve(svg.size());

    qsizetype cursor = 0;
    while (true) {
        const qsizetype start = svg.indexOf(ATTRIBUTE, cursor);
        if (start < 0) {
            resolved.append(svg.sliced(cursor));
            return resolved;
        }

        // Only an XML attribute is rewritten. A CSS declaration such as
        // `style="stroke-width:2"` has no `=` right after the name, so it stays
        // untouched and the scan moves past it.
        qsizetype nameEnd = start + ATTRIBUTE.size();
        while (nameEnd < svg.size() && svg.at(nameEnd) == ' ') {
            ++nameEnd;
        }
        const bool isAttribute = nameEnd < svg.size() && svg.at(nameEnd) == '=' && nameEnd + 1 < svg.size();
        const char quoteChar = isAttribute ? svg.at(nameEnd + 1) : '\0';
        const bool quoted = quoteChar == '"' || quoteChar == '\'';
        const qsizetype close = quoted ? svg.indexOf(quoteChar == '"' ? DOUBLE_QUOTE : SINGLE_QUOTE, nameEnd + 2) : -1;
        if (close < 0) {
            resolved.append(svg.sliced(cursor, start + ATTRIBUTE.size() - cursor));
            cursor = start + ATTRIBUTE.size();
            continue;
        }

        // Keep the opening quote, replace the value, and resume at the closing
        // quote so the rest of the document is copied verbatim.
        resolved.append(svg.sliced(cursor, nameEnd + 2 - cursor));
        resolved.append(value);
        cursor = close;
    }
}
