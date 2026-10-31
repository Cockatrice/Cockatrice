#include "svg_recolor.h"

#include <QString>

QByteArray resolveSvgColorSentinel(const QByteArray &svg, const QByteArray &color)
{
    const QString source = QString::fromUtf8(svg);
    if (!source.contains(QStringLiteral("currentColor"), Qt::CaseInsensitive)) {
        return svg;
    }

    QString resolved = source;
    resolved.replace(QStringLiteral("currentColor"), QString::fromLatin1(color), Qt::CaseInsensitive);
    return resolved.toUtf8();
}
