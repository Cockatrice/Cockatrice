#include "theme_config.h"

#include <QByteArray>
#include <QChar>
#include <QDir>
#include <QFile>
#include <QFlags>
#include <QIODevice>
#include <QList>
#include <QMetaEnum>
#include <QPair>
#include <QStringLiteral>
#include <QTextStream>
#include <qnamespace.h>
#include <utility>

bool ThemeConfig::isEmpty() const
{
    return colorScheme.isEmpty() && styleName.isEmpty();
}

QString ThemeConfig::toIni() const
{
    QString out;
    out += "[Appearance]\n";
    out += QString("ColorScheme = %1\n").arg(colorScheme.isEmpty() ? "System" : colorScheme);
    out += "\n[Style]\n";
    out += QString("Name = %1\n").arg(styleName.isEmpty() ? "System" : styleName);
    return out;
}

ThemeConfig ThemeConfig::fromThemeDir(const QString &themeDirPath)
{
    ThemeConfig cfg;

    if (themeDirPath.isEmpty()) {
        return cfg;
    }

    QFile f(QDir(themeDirPath).absoluteFilePath("theme.cfg"));
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return cfg;
    }

    QString currentSection;

    QTextStream in(&f);

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();

        if (line.isEmpty() || line.startsWith('#') || line.startsWith(';')) {
            continue;
        }

        if (line.startsWith('[') && line.endsWith(']')) {
            currentSection = line.mid(1, line.length() - 2).trimmed();
            continue;
        }

        int eq = line.indexOf('=');
        if (eq < 0) {
            continue;
        }

        QString key = line.left(eq).trimmed();
        QString value = line.mid(eq + 1).trimmed();

        if (currentSection.compare("Appearance", Qt::CaseInsensitive) == 0) {
            if (key.compare("ColorScheme", Qt::CaseInsensitive) == 0) {
                cfg.colorScheme = value;
            }
        } else if (currentSection.compare("Style", Qt::CaseInsensitive) == 0) {
            if (key.compare("Name", Qt::CaseInsensitive) == 0) {
                cfg.styleName = value;
            }
        }
    }

    return cfg;
}

bool ThemeConfig::save(const QString &themeDirPath) const
{
    if (themeDirPath.isEmpty()) {
        return false;
    }

    QDir dir(themeDirPath);

    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QFile f(dir.absoluteFilePath("theme.cfg"));
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        return false;
    }

    QTextStream out(&f);
    out << toIni();

    return true;
}

bool PaletteConfig::hasPalette() const
{
    return !colors.isEmpty() || !appColors.isEmpty();
}

QString PaletteConfig::toToml() const
{
    QMetaEnum roleEnum = QMetaEnum::fromType<QPalette::ColorRole>();

    QString out;

    static const QList<QPair<QPalette::ColorGroup, QString>> groups = {
        {QPalette::Active, "Palette"},
        {QPalette::Disabled, "Palette.Disabled"},
        {QPalette::Inactive, "Palette.Inactive"},
    };

    for (const auto &[group, sectionName] : groups) {
        if (!colors.contains(group)) {
            continue;
        }

        out += QString("[%1]\n").arg(sectionName);

        const auto &roleMap = colors[group];

        for (auto it = roleMap.cbegin(); it != roleMap.cend(); ++it) {
            const char *roleName = roleEnum.valueToKey(it.key());

            if (!roleName) {
                continue;
            }

            out += QString("%1 = %2\n").arg(QString(roleName), -20).arg(it.value().name(QColor::HexArgb));
        }

        out += "\n";
    }

    if (!appColors.isEmpty()) {
        QMetaEnum appEnum = QMetaEnum::fromType<AppColor::Role>();

        out += "[AppColors]\n";

        for (auto it = appColors.cbegin(); it != appColors.cend(); ++it) {
            const char *roleName = appEnum.valueToKey(it.key());

            if (!roleName) {
                continue;
            }

            out += QString("%1 = %2\n").arg(QString(roleName), -20).arg(it.value().name(QColor::HexArgb));
        }

        out += "\n";
    }

    return out;
}

QString PaletteConfig::fileName(const QString &colorScheme)
{
    return colorScheme.compare("Dark", Qt::CaseInsensitive) == 0 ? "palette-dark.toml" : "palette-light.toml";
}

PaletteConfig PaletteConfig::fromFile(const QString &filePath)
{
    PaletteConfig cfg;

    QFile f(filePath);

    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return cfg;
    }

    QMetaEnum roleEnum = QMetaEnum::fromType<QPalette::ColorRole>();
    QMetaEnum appEnum = QMetaEnum::fromType<AppColor::Role>();

    QString currentSection;
    QPalette::ColorGroup currentGroup = QPalette::Active;

    QTextStream in(&f);

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();

        if (line.isEmpty() || line.startsWith('#') || line.startsWith(';')) {
            continue;
        }

        if (line.startsWith('[') && line.endsWith(']')) {
            currentSection = line.mid(1, line.length() - 2).trimmed();

            if (currentSection.startsWith("Palette", Qt::CaseInsensitive)) {
                int dot = currentSection.indexOf('.');

                QString groupStr = (dot >= 0) ? currentSection.mid(dot + 1) : "Active";

                if (groupStr.compare("Disabled", Qt::CaseInsensitive) == 0) {
                    currentGroup = QPalette::Disabled;
                } else if (groupStr.compare("Inactive", Qt::CaseInsensitive) == 0) {
                    currentGroup = QPalette::Inactive;
                } else {
                    currentGroup = QPalette::Active;
                }
            }

            continue;
        }

        int eq = line.indexOf('=');

        if (eq < 0) {
            continue;
        }

        QString key = line.left(eq).trimmed();
        QString value = line.mid(eq + 1).trimmed();

        // Strip inline comments if preceded by whitespace
        for (int i = 1; i < value.size(); ++i) {
            if (value[i] == '#' && value[i - 1].isSpace()) {
                value = value.left(i).trimmed();
                break;
            }
        }

        QColor color(value);

        if (!color.isValid()) {
            continue;
        }

        if (currentSection.compare("AppColors", Qt::CaseInsensitive) == 0) {
            if (key.startsWith("AppColor::")) {
                key = key.mid(10);
            }

            int appRoleInt = appEnum.keyToValue(key.toUtf8().constData());

            if (appRoleInt >= 0) {
                cfg.appColors[static_cast<AppColor::Role>(appRoleInt)] = color;
            }

            continue;
        }

        if (!currentSection.startsWith("Palette", Qt::CaseInsensitive)) {
            continue;
        }

        if (key.startsWith("QPalette::")) {
            key = key.mid(10);
        }

        int roleInt = roleEnum.keyToValue(key.toUtf8().constData());

        if (roleInt < 0) {
            continue;
        }

        cfg.colors[currentGroup][static_cast<QPalette::ColorRole>(roleInt)] = color;
    }

    return cfg;
}

PaletteConfig PaletteConfig::fromScheme(const QString &themeDirPath, const QString &colorScheme)
{
    if (themeDirPath.isEmpty()) {
        return {};
    }

    return fromFile(QDir(themeDirPath).absoluteFilePath(fileName(colorScheme)));
}

PaletteConfig PaletteConfig::fromDefault(const QString &themeDirPath, const QString &colorScheme)
{
    if (themeDirPath.isEmpty()) {
        return {};
    }

    QDir dir(themeDirPath);

    bool wantDark = colorScheme.compare("Dark", Qt::CaseInsensitive) == 0;

    // Only the default file matching the requested scheme is used. Falling back
    // to the opposite scheme's default would silently apply dark colours to a
    // "Light" scheme (or vice versa). Callers already fall back to the OS /
    // application palette when no default palette is available.
    return fromFile(dir.absoluteFilePath(wantDark ? "palette-default-dark.toml" : "palette-default-light.toml"));
}

QPalette PaletteConfig::apply(QPalette base) const
{
    for (auto git = colors.cbegin(); git != colors.cend(); ++git) {
        for (auto rit = git.value().cbegin(); rit != git.value().cend(); ++rit) {
            base.setColor(git.key(), rit.key(), rit.value());
        }
    }

    return base;
}

bool FontConfig::hasFonts() const
{
    return !fonts.isEmpty() || cardTextMinSize != 1 || cardTextMaxSize > 0;
}

QString FontConfig::toToml() const
{
    QMetaEnum roleEnum = QMetaEnum::fromType<AppFont::Role>();

    QString entries;

    for (auto it = fonts.cbegin(); it != fonts.cend(); ++it) {
        const char *roleName = roleEnum.valueToKey(it.key());

        if (!roleName) {
            continue;
        }

        QString family = it.value().family().trimmed();
        int pointSize = it.value().pointSize();

        if (family.isEmpty() || pointSize <= 0) {
            continue;
        }

        entries += QString("%1 = \"%2, %3\"\n").arg(QString(roleName), -20).arg(family).arg(pointSize);
    }

    if (cardTextMinSize != 1) {
        entries += QString("%1 = %2\n").arg(QStringLiteral("CardTextMinSize"), -20).arg(cardTextMinSize);
    }

    if (cardTextMaxSize > 0) {
        entries += QString("%1 = %2\n").arg(QStringLiteral("CardTextMaxSize"), -20).arg(cardTextMaxSize);
    }

    if (entries.isEmpty()) {
        return {};
    }

    return "[Fonts]\n" + entries + "\n";
}

QString FontConfig::fileName()
{
    return "fonts.toml";
}

FontConfig FontConfig::fromFile(const QString &filePath)
{
    FontConfig cfg;

    QFile f(filePath);

    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return cfg;
    }

    QMetaEnum roleEnum = QMetaEnum::fromType<AppFont::Role>();

    QString currentSection;

    QTextStream in(&f);

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();

        if (line.isEmpty() || line.startsWith('#') || line.startsWith(';')) {
            continue;
        }

        if (line.startsWith('[') && line.endsWith(']')) {
            currentSection = line.mid(1, line.length() - 2).trimmed();
            continue;
        }

        if (currentSection.compare("Fonts", Qt::CaseInsensitive) != 0) {
            continue;
        }

        int eq = line.indexOf('=');

        if (eq < 0) {
            continue;
        }

        QString key = line.left(eq).trimmed();
        QString value = line.mid(eq + 1).trimmed();

        // Strip inline comments if preceded by whitespace
        for (int i = 1; i < value.size(); ++i) {
            if ((value[i] == '#' || value[i] == ';') && value[i - 1].isSpace()) {
                value = value.left(i).trimmed();
                break;
            }
        }

        // Strip surrounding quotes, if any
        if (value.size() >= 2 &&
            ((value.startsWith('"') && value.endsWith('"')) || (value.startsWith('\'') && value.endsWith('\'')))) {
            value = value.mid(1, value.size() - 2).trimmed();
        }

        // CardText size bounds: bare integers, e.g. CardTextMinSize = 2.
        // Tolerate quoting like the family entries; skip invalid lines.
        QString sizeKey = key;
        if (sizeKey.startsWith("AppFont::")) {
            sizeKey = sizeKey.mid(QStringLiteral("AppFont::").size());
        }
        if (sizeKey.compare("CardTextMinSize", Qt::CaseInsensitive) == 0) {
            bool ok = false;
            int minSize = value.trimmed().toInt(&ok);
            if (!ok || minSize <= 0) {
                continue;
            }
            cfg.cardTextMinSize = minSize;
            continue;
        }
        if (sizeKey.compare("CardTextMaxSize", Qt::CaseInsensitive) == 0) {
            bool ok = false;
            int maxSize = value.trimmed().toInt(&ok);
            if (!ok || maxSize <= 0) {
                continue;
            }
            cfg.cardTextMaxSize = maxSize;
            continue;
        }

        int comma = value.lastIndexOf(',');

        if (comma < 0) {
            continue;
        }

        QString family = value.left(comma).trimmed();

        // Tolerate a partially-quoted family, e.g. "Inter", 10
        if (family.size() >= 2 &&
            ((family.startsWith('"') && family.endsWith('"')) || (family.startsWith('\'') && family.endsWith('\'')))) {
            family = family.mid(1, family.size() - 2).trimmed();
        }

        bool ok = false;
        int pointSize = value.mid(comma + 1).trimmed().toInt(&ok);

        if (!ok || family.isEmpty() || pointSize <= 0) {
            continue;
        }

        if (key.startsWith("AppFont::")) {
            key = key.mid(QStringLiteral("AppFont::").size());
        }

        int roleInt = roleEnum.keyToValue(key.toUtf8().constData());

        if (roleInt < 0) {
            continue;
        }

        // Canonical family + point size only: a fresh QFont carries default
        // (non-bold, non-italic) weight/style, so nothing caller-side leaks in.
        // No validation against installed fonts; Qt substitution applies.
        QFont font(family);
        font.setPointSize(pointSize);
        cfg.fonts[static_cast<AppFont::Role>(roleInt)] = font;
    }

    return cfg;
}

FontConfig FontConfig::fromThemeDir(const QString &themeDirPath)
{
    if (themeDirPath.isEmpty()) {
        return {};
    }

    return fromFile(QDir(themeDirPath).absoluteFilePath(fileName()));
}

QColor defaultAppColor(AppColor::Role role, bool dark)
{
    switch (role) {
        case AppColor::AccentStrong:
        case AppColor::AccentSoft:
        // The neutral icon family is palette-derived like the accent: a theme
        // that leaves it unauthored (System, and every theme written before the
        // roles existed) keeps the live platform palette's WindowText instead
        // of pinning one scheme's ink.
        case AppColor::Icon:
        case AppColor::IconStrong:
        case AppColor::IconSoft:
            return {};
        // The semantic family follows the same chroma ladder as the authored
        // themes. A theme that authors none of it has no per-surface palette to
        // measure against, so these are authored against the representative
        // platform surfaces the System theme runs on.
        case AppColor::Positive:
            return QColor(dark ? QStringLiteral("#6dd58c") : QStringLiteral("#1a7d3a"));
        case AppColor::PositiveStrong:
            return QColor(dark ? QStringLiteral("#32c05c") : QStringLiteral("#007e28"));
        case AppColor::PositiveSoft:
            return QColor(dark ? QStringLiteral("#8ab196") : QStringLiteral("#4c7659"));
        case AppColor::Negative:
            return QColor(dark ? QStringLiteral("#ff8a80") : QStringLiteral("#b3261e"));
        case AppColor::NegativeStrong:
            return QColor(dark ? QStringLiteral("#ff8378") : QStringLiteral("#da0b00"));
        case AppColor::NegativeSoft:
            return QColor(dark ? QStringLiteral("#c79e9b") : QStringLiteral("#935f5c"));
        // Informational is the third semantic family: a state that is neither a
        // gain nor a loss (a face-down play, the exile zone). It carries the
        // same three-level ladder so its emphasis is authored, not computed.
        case AppColor::Informational:
            return QColor(dark ? QStringLiteral("#7aa9f2") : QStringLiteral("#1c64c4"));
        case AppColor::InformationalStrong:
            return QColor(dark ? QStringLiteral("#69aeff") : QStringLiteral("#0a4fb5"));
        case AppColor::InformationalSoft:
            return QColor(dark ? QStringLiteral("#9db2cd") : QStringLiteral("#526a8a"));
        // Hidden is the game's hidden-information cluster: a face-down play and
        // peeking at a card face. Violet keeps it apart from the blue
        // Informational family, which carries public information (reveal).
        case AppColor::Hidden:
            return QColor(dark ? QStringLiteral("#c6a5fa") : QStringLiteral("#6d28d9"));
        case AppColor::HiddenStrong:
            return QColor(dark ? QStringLiteral("#c99bff") : QStringLiteral("#4c0f9e"));
        case AppColor::HiddenSoft:
            return QColor(dark ? QStringLiteral("#b3a6c9") : QStringLiteral("#6a5a8c"));
        // Marked is the marks a player leaves on a card: an annotation or a
        // draw arrow. Amber names a persistent, removable mark rather than a
        // gain or a loss, so annotation and arrow finally share a colour.
        case AppColor::Marked:
            return QColor(dark ? QStringLiteral("#e2a63a") : QStringLiteral("#9a5b05"));
        case AppColor::MarkedStrong:
            return QColor(dark ? QStringLiteral("#ffb020") : QStringLiteral("#8a4f02"));
        case AppColor::MarkedSoft:
            return QColor(dark ? QStringLiteral("#c2ab7d") : QStringLiteral("#7b6a4a"));
        // Movement is a card changing position without a gain or a loss: the
        // library placements (top, bottom, X from top). Teal keeps it apart
        // from the green "into play" moves and the blue informational states.
        case AppColor::Movement:
            return QColor(dark ? QStringLiteral("#4cc2b5") : QStringLiteral("#0e7a72"));
        case AppColor::MovementStrong:
            return QColor(dark ? QStringLiteral("#2dd4bf") : QStringLiteral("#096b62"));
        case AppColor::MovementSoft:
            return QColor(dark ? QStringLiteral("#a3c1bc") : QStringLiteral("#4a6b67"));
    }

    return {};
}
