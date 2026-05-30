#include "theme_config.h"

#include <QDir>
#include <QFile>
#include <QMetaEnum>
#include <QTextStream>

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
    out += QString("Name = %1\n").arg(styleName.isEmpty() ? "Default" : styleName);
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
    return !colors.isEmpty();
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

        QColor color(value);

        if (color.isValid()) {
            cfg.colors[currentGroup][static_cast<QPalette::ColorRole>(roleInt)] = color;
        }
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

    PaletteConfig cfg =
        fromFile(dir.absoluteFilePath(wantDark ? "palette-default-dark.toml" : "palette-default-light.toml"));

    if (!cfg.hasPalette()) {
        cfg = fromFile(dir.absoluteFilePath(wantDark ? "palette-default-light.toml" : "palette-default-dark.toml"));
    }

    return cfg;
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