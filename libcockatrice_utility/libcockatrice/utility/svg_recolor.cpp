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
    for (qsizetype i = from; i + SENTINEL.size() <= bytes.size(); ++i) {
        if (bytes.sliced(i, SENTINEL.size()).compare(SENTINEL, Qt::CaseInsensitive) == 0) {
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
