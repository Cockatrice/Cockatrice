#include <QColor>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QXmlStreamReader>
#include <gtest/gtest.h>

namespace
{
const QString ICONS_RELATIVE_PATH = QStringLiteral("cockatrice/resources/icons");
const QString BASELINE_RELATIVE_PATH = QStringLiteral("tests/icon_assets/icon-baseline.txt");

QString iconsDir()
{
    return QDir(QStringLiteral(COCKATRICE_SOURCE_DIR)).absoluteFilePath(ICONS_RELATIVE_PATH);
}

QString baselinePath()
{
    return QDir(QStringLiteral(COCKATRICE_SOURCE_DIR)).absoluteFilePath(BASELINE_RELATIVE_PATH);
}

/** @brief True for content art that is allowed to carry full color. */
bool isContentArt(const QString &relativePath)
{
    return relativePath.startsWith(QStringLiteral("mana/"));
}

QString violation(const QString &rule, const QString &relativePath)
{
    return rule + QLatin1Char(' ') + relativePath;
}

/**
 * @brief Reads one SVG and returns true when the root carries a usable viewBox.
 *
 * Also reports malformed XML and editor-only metadata through the out params.
 */
bool inspectSvg(const QString &absolutePath,
                bool &validXml,
                bool &hasViewBox,
                bool &hasEditorMetadata,
                bool &hasPhysicalSize)
{
    validXml = false;
    hasViewBox = false;
    hasEditorMetadata = false;
    hasPhysicalSize = false;

    QFile file(absolutePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QXmlStreamReader reader(&file);
    bool sawRoot = false;

    while (!reader.atEnd()) {
        reader.readNext();

        if (reader.hasError()) {
            return false;
        }

        if (!reader.isStartElement()) {
            continue;
        }

        if (!sawRoot) {
            sawRoot = true;
            if (reader.name() != QStringLiteral("svg")) {
                return false;
            }
            const QString viewBox = reader.attributes().value(QStringLiteral("viewBox")).toString().simplified();
            hasViewBox = viewBox == QStringLiteral("0 0 24 24");
            hasPhysicalSize = !reader.attributes().value(QStringLiteral("width")).isEmpty() ||
                              !reader.attributes().value(QStringLiteral("height")).isEmpty();
        }

        if (reader.name() == QStringLiteral("metadata") || reader.name() == QStringLiteral("namedview")) {
            hasEditorMetadata = true;
        }

        for (const QXmlStreamAttribute &attribute : reader.attributes()) {
            const QString prefix = attribute.prefix().toString();
            if (prefix == QStringLiteral("inkscape") || prefix == QStringLiteral("sodipodi") ||
                prefix == QStringLiteral("dc") || prefix == QStringLiteral("cc") || prefix == QStringLiteral("rdf")) {
                hasEditorMetadata = true;
            }
        }
    }

    validXml = sawRoot && !reader.hasError();
    return validXml;
}

bool isAllowedPaintValue(const QString &value)
{
    const QString normalized = value.trimmed().toLower();
    return normalized.isEmpty() || normalized == QStringLiteral("none") ||
           normalized == QStringLiteral("currentcolor") || normalized == QStringLiteral("inherit") ||
           normalized == QStringLiteral("context-fill") || normalized == QStringLiteral("context-stroke") ||
           normalized.startsWith(QStringLiteral("url("));
}

QRegularExpression paintValuePattern()
{
    static const QRegularExpression pattern(
        QStringLiteral(R"re((?:fill|stroke|stop-color|color)\s*[:=]\s*["']?\s*([^"'\s;>]+))re"),
        QRegularExpression::CaseInsensitiveOption);
    return pattern;
}

bool hasHardcodedColor(const QString &absolutePath)
{
    QFile file(absolutePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    const QString content = QString::fromUtf8(file.readAll());
    auto matches = paintValuePattern().globalMatch(content);
    while (matches.hasNext()) {
        const QString value = matches.next().captured(1).trimmed();
        if (isAllowedPaintValue(value)) {
            continue;
        }
        const QString normalized = value.toLower();
        if (normalized.startsWith(QLatin1Char('#')) || normalized.contains(QStringLiteral("rgb(")) ||
            normalized.contains(QStringLiteral("hsl(")) || QColor(value).isValid()) {
            return true;
        }
    }
    return false;
}

bool hasCurrentColorToken(const QString &absolutePath)
{
    QFile file(absolutePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    const QString content = QString::fromUtf8(file.readAll());
    auto matches = paintValuePattern().globalMatch(content);
    while (matches.hasNext()) {
        if (matches.next().captured(1).trimmed().compare(QStringLiteral("currentColor"), Qt::CaseInsensitive) == 0) {
            return true;
        }
    }
    return false;
}

bool isSchemeVariant(const QString &fileName)
{
    return fileName.contains(QStringLiteral("-dark."), Qt::CaseInsensitive) ||
           fileName.contains(QStringLiteral("-light."), Qt::CaseInsensitive);
}

bool hasColorInFilename(const QString &fileName)
{
    static const QRegularExpression colorSuffix(
        QStringLiteral(R"(_(green|blue|red|yellow|black|white|gray|grey)\.svg$)"),
        QRegularExpression::CaseInsensitiveOption);
    return colorSuffix.match(fileName).hasMatch();
}

/** @brief Collects every current violation as "rule relative/path.svg". */
QSet<QString> collectViolations()
{
    QSet<QString> violations;

    QDirIterator it(iconsDir(), QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        const QFileInfo info = it.fileInfo();
        const QString relativePath = QDir(iconsDir()).relativeFilePath(info.absoluteFilePath());
        if (isContentArt(relativePath)) {
            continue;
        }
        if (info.suffix().compare(QStringLiteral("svg"), Qt::CaseInsensitive) != 0) {
            violations.insert(violation(QStringLiteral("raster"), relativePath));
            continue;
        }

        bool validXml = false;
        bool hasViewBox = false;
        bool hasEditorMetadata = false;
        bool hasPhysicalSize = false;
        inspectSvg(info.absoluteFilePath(), validXml, hasViewBox, hasEditorMetadata, hasPhysicalSize);

        if (!validXml) {
            violations.insert(violation(QStringLiteral("xml"), relativePath));
        }
        if (validXml && !hasViewBox) {
            violations.insert(violation(QStringLiteral("viewbox"), relativePath));
        }
        if (hasPhysicalSize) {
            violations.insert(violation(QStringLiteral("physical-size"), relativePath));
        }
        if (hasEditorMetadata) {
            violations.insert(violation(QStringLiteral("editor-metadata"), relativePath));
        }
        if (hasHardcodedColor(info.absoluteFilePath()) && !isSchemeVariant(info.fileName())) {
            violations.insert(violation(QStringLiteral("hardcoded-color"), relativePath));
        }
        if (!hasCurrentColorToken(info.absoluteFilePath()) && !isSchemeVariant(info.fileName())) {
            violations.insert(violation(QStringLiteral("no-token"), relativePath));
        }
        if (hasColorInFilename(info.fileName())) {
            violations.insert(violation(QStringLiteral("color-filename"), relativePath));
        }
    }

    return violations;
}

/** @brief Parses the ratchet file; returns false with an error when it cannot be read. */
bool readBaseline(QSet<QString> &entries, QString &error)
{
    QFile file(baselinePath());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        error = QStringLiteral("Cannot read %1").arg(baselinePath());
        return false;
    }

    const QStringList lines = QString::fromUtf8(file.readAll()).split(QLatin1Char('\n'));
    for (const QString &line : lines) {
        const QString trimmed = line.trimmed();
        if (trimmed.isEmpty() || trimmed.startsWith(QLatin1Char('#'))) {
            continue;
        }
        entries.insert(trimmed);
    }
    return true;
}

QString formatSet(const QSet<QString> &entries)
{
    QStringList lines(entries.begin(), entries.end());
    lines.sort();
    return lines.join(QLatin1Char('\n'));
}

} // namespace

/**
 * @brief The icon ratchet: no new violations, and fixed icons must leave the baseline.
 *
 * Existing UI glyphs are grandfathered in tests/icon_assets/icon-baseline.txt. Fixing an icon
 * without deleting its baseline lines fails on purpose, so the list can only shrink.
 */
TEST(IconAssets, BaselineRatchet)
{
    ASSERT_TRUE(QDir::isAbsolutePath(iconsDir()));
    ASSERT_TRUE(QDir(iconsDir()).exists()) << "Missing icon directory: " << iconsDir().toStdString();

    QSet<QString> baseline;
    QString error;
    ASSERT_TRUE(readBaseline(baseline, error)) << error.toStdString();

    const QSet<QString> actual = collectViolations();

    const QSet<QString> newViolations = actual - baseline;
    const QSet<QString> staleBaseline = baseline - actual;

    EXPECT_TRUE(newViolations.isEmpty()) << "New icon rule violations. Fix the icon; do not add exceptions:\n"
                                         << formatSet(newViolations).toStdString();

    EXPECT_TRUE(staleBaseline.isEmpty()) << "These baseline entries are fixed or gone; delete them from "
                                         << BASELINE_RELATIVE_PATH.toStdString() << ":\n"
                                         << formatSet(staleBaseline).toStdString();
}

/** @brief Guards against duplicate or malformed baseline entries. */
TEST(IconAssets, BaselineIsWellFormed)
{
    QSet<QString> baseline;
    QString error;
    ASSERT_TRUE(readBaseline(baseline, error)) << error.toStdString();
    EXPECT_TRUE(baseline.isEmpty()) << "The legacy baseline is fully paid down; fix new violations instead";

    static const QRegularExpression entryPattern(QStringLiteral(
        R"(^(xml|viewbox|editor-metadata|hardcoded-color|color-filename|no-token|physical-size|raster) \S+$)"));
    for (const QString &entry : baseline) {
        EXPECT_TRUE(entryPattern.match(entry).hasMatch()) << "Malformed baseline entry: " << entry.toStdString();
    }
}
