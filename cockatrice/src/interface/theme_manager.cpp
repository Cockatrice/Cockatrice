#include "theme_manager.h"

#include "../../client/settings/cache_settings.h"

#include <QApplication>
#include <QColor>
#include <QDebug>
#include <QLibraryInfo>
#include <QPalette>
#include <QPixmapCache>
#include <QStandardPaths>
#include <QStyleFactory>
#include <QStyleHints>
#include <Qt>

#define NONE_THEME_NAME "Default"
#define FUSION_THEME_NAME "Fusion"
#define STYLE_CSS_NAME "style.css"
#define HANDZONE_BG_NAME "handzone"
#define PLAYERZONE_BG_NAME "playerzone"
#define STACKZONE_BG_NAME "stackzone"
#define TABLEZONE_BG_NAME "tablezone"
static const QColor HANDZONE_BG_DEFAULT = QColor(80, 100, 50);
static const QColor TABLEZONE_BG_DEFAULT = QColor(70, 50, 100);
static const QColor PLAYERZONE_BG_DEFAULT = QColor(200, 200, 200);
static const QColor STACKZONE_BG_DEFAULT = QColor(113, 43, 43);
static const QStringList DEFAULT_RESOURCE_PATHS = {":/resources"};

#include <QMap>
#include <QMetaEnum>
#include <QString>

struct PaletteColorInfo
{
    QPalette::ColorGroup group;
    QPalette::ColorRole role;
    QColor color;
};

QList<PaletteColorInfo> queryAllPaletteColors(const QPalette &palette = qApp->palette())
{
    QList<PaletteColorInfo> colors;

    // QMetaEnum groupEnum = QMetaEnum::fromType<QPalette::ColorGroup>();
    // QMetaEnum roleEnum = QMetaEnum::fromType<QPalette::ColorRole>();

    // Iterate through relevant color groups (Active, Disabled, Inactive)
    const QList<QPalette::ColorGroup> groups = {QPalette::Active, QPalette::Disabled, QPalette::Inactive};

    for (auto group : groups) {
        // Iterate through all color roles (excluding NoRole and NColorRoles)
        for (int r = 0; r < QPalette::NColorRoles; ++r) {
            auto role = static_cast<QPalette::ColorRole>(r);
            if (role == QPalette::NoRole)
                continue;

            PaletteColorInfo info;
            info.group = group;
            info.role = role;
            info.color = palette.color(group, role);
            colors.append(info);
        }
    }

    return colors;
}

// Pretty print version
void printPaletteColors(const QPalette &palette = qApp->palette())
{
    QMetaEnum groupEnum = QMetaEnum::fromType<QPalette::ColorGroup>();
    QMetaEnum roleEnum = QMetaEnum::fromType<QPalette::ColorRole>();

    const QList<QPalette::ColorGroup> groups = {QPalette::Active, QPalette::Disabled, QPalette::Inactive};

    for (auto group : groups) {
        qInfo() << "\n===========" << groupEnum.valueToKey(group) << "===========";

        for (int r = 0; r < QPalette::NColorRoles; ++r) {
            auto role = static_cast<QPalette::ColorRole>(r);
            if (role == QPalette::NoRole)
                continue;

            QColor color = palette.color(group, role);
            qInfo().nospace() << qPrintable(QString("%1").arg(roleEnum.valueToKey(role), -20)) << " : "
                              << qPrintable(color.name(QColor::HexArgb)) << " (RGBA: " << color.red() << ", "
                              << color.green() << ", " << color.blue() << ", " << color.alpha() << ")";
        }
    }
}

// Export to structured format (e.g., for config files)
QMap<QString, QMap<QString, QString>> exportPaletteToMap(const QPalette &palette = qApp->palette())
{
    QMap<QString, QMap<QString, QString>> result;

    QMetaEnum groupEnum = QMetaEnum::fromType<QPalette::ColorGroup>();
    QMetaEnum roleEnum = QMetaEnum::fromType<QPalette::ColorRole>();

    const QList<QPalette::ColorGroup> groups = {QPalette::Active, QPalette::Disabled, QPalette::Inactive};

    for (auto group : groups) {
        QString groupName = groupEnum.valueToKey(group);

        for (int r = 0; r < QPalette::NColorRoles; ++r) {
            auto role = static_cast<QPalette::ColorRole>(r);
            if (role == QPalette::NoRole)
                continue;

            QString roleName = roleEnum.valueToKey(role);
            QColor color = palette.color(group, role);
            result[groupName][roleName] = color.name(QColor::HexArgb);
        }
    }

    return result;
}

ThemeManager::ThemeManager(QObject *parent) : QObject(parent)
{
    ensureThemeDirectoryExists();
#if (QT_VERSION >= QT_VERSION_CHECK(6, 5, 0))
    connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged, this, &ThemeManager::themeChangedSlot);
#endif
    connect(&SettingsCache::instance(), &SettingsCache::themeChanged, this, &ThemeManager::themeChangedSlot);
    themeChangedSlot();
}

void ThemeManager::ensureThemeDirectoryExists()
{
    if (SettingsCache::instance().getThemeName().isEmpty() ||
        !getAvailableThemes().contains(SettingsCache::instance().getThemeName())) {
        qCInfo(ThemeManagerLog) << "Theme name not set, setting default value";
        SettingsCache::instance().setThemeName(NONE_THEME_NAME);
    }
}

QStringMap &ThemeManager::getAvailableThemes()
{
    QDir dir;
    availableThemes.clear();

    // add default value
    availableThemes.insert(NONE_THEME_NAME, QString());
    availableThemes.insert(FUSION_THEME_NAME, QString());

    // load themes from user profile dir
    dir.setPath(SettingsCache::instance().getThemesPath());

    for (QString themeName : dir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot, QDir::Name)) {
        if (!availableThemes.contains(themeName)) {
            availableThemes.insert(themeName, dir.absoluteFilePath(themeName));
        }
    }

    // load themes from cockatrice system dir
    dir.setPath(qApp->applicationDirPath() +
#ifdef Q_OS_MAC
                "/../Resources/themes"
#elif defined(Q_OS_WIN)
                "/themes"
#else // linux
                "/../share/cockatrice/themes"
#endif
    );

    for (QString themeName : dir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot, QDir::Name)) {
        if (!availableThemes.contains(themeName)) {
            availableThemes.insert(themeName, dir.absoluteFilePath(themeName));
        }
    }

    return availableThemes;
}

QBrush ThemeManager::loadBrush(QString fileName, QColor fallbackColor)
{
    QBrush brush;
    QPixmap tmp = QPixmap("theme:zones/" + fileName);
    if (tmp.isNull()) {
        brush.setColor(fallbackColor);
        brush.setStyle(Qt::SolidPattern);
    } else {
        brush.setTexture(tmp);
    }

    return brush;
}

QBrush ThemeManager::loadExtraBrush(QString fileName, QBrush &fallbackBrush)
{
    QBrush brush;
    QPixmap tmp = QPixmap("theme:zones/" + fileName);

    if (tmp.isNull()) {
        brush = fallbackBrush;
    } else {
        brush.setTexture(tmp);
    }

    return brush;
}

void ThemeManager::themeChangedSlot()
{
    QString themeName = SettingsCache::instance().getThemeName();
    qCInfo(ThemeManagerLog) << "Theme changed:" << themeName;

    QString dirPath = getAvailableThemes().value(themeName);
    QDir dir = dirPath;

    // css
    if (!dirPath.isEmpty() && dir.exists(STYLE_CSS_NAME)) {
        qApp->setStyleSheet("file:///" + dir.absoluteFilePath(STYLE_CSS_NAME));
    } else {
        qApp->setStyleSheet("");
    }

    if (themeName == FUSION_THEME_NAME) {
        qApp->setStyle(QStyleFactory::create("Fusion"));
#if (QT_VERSION >= QT_VERSION_CHECK(6, 5, 0))
        QPalette palette;
        if (QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark) {
            palette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
        }

        qApp->setPalette(palette);
#endif
    } else {
        qApp->setStyle(QStyleFactory::create(QStyleFactory::keys().first()));
    }

    if (dirPath.isEmpty()) {
        // set default values
        QDir::setSearchPaths("theme", DEFAULT_RESOURCE_PATHS);
        brushes[Role::Hand] = HANDZONE_BG_DEFAULT;
        brushes[Role::Table] = TABLEZONE_BG_DEFAULT;
        brushes[Role::Player] = PLAYERZONE_BG_DEFAULT;
        brushes[Role::Stack] = STACKZONE_BG_DEFAULT;
    } else {
        // resources
        QStringList resources;
        resources << dir.absolutePath() << DEFAULT_RESOURCE_PATHS;
        QDir::setSearchPaths("theme", resources);

        // zones bg
        dir.cd("zones");
        brushes[Role::Hand] = loadBrush(HANDZONE_BG_NAME, HANDZONE_BG_DEFAULT);
        brushes[Role::Table] = loadBrush(TABLEZONE_BG_NAME, TABLEZONE_BG_DEFAULT);
        brushes[Role::Player] = loadBrush(PLAYERZONE_BG_NAME, PLAYERZONE_BG_DEFAULT);
        brushes[Role::Stack] = loadBrush(STACKZONE_BG_NAME, STACKZONE_BG_DEFAULT);
    }
    for (auto &brushCache : brushesCache) {
        brushCache.clear();
    }

    QPixmapCache::clear();

    printPaletteColors();

    emit themeChanged();
}

static QString roleBgName(ThemeManager::Role role)
{
    switch (role) {
        case ThemeManager::Hand:
            return HANDZONE_BG_NAME;

        case ThemeManager::Player:
            return PLAYERZONE_BG_NAME;

        case ThemeManager::Stack:
            return STACKZONE_BG_NAME;

        case ThemeManager::Table:
            return TABLEZONE_BG_NAME;

        default:
            Q_ASSERT(false);
    }
}

QBrush &ThemeManager::getBgBrush(Role role)
{
    return brushes[role];
}

QBrush ThemeManager::getExtraBgBrush(Role role, int zoneId)
{
    if (zoneId <= 0) {
        return getBgBrush(role);
    }

    QBrushMap &brushCache = brushesCache[role];

    if (!brushCache.contains(zoneId)) {
        QBrush brush = loadExtraBrush(roleBgName(role) + QString::number(zoneId), getBgBrush(role));
        brushCache.insert(zoneId, brush);
        return brush;
    }

    return brushCache.value(zoneId);
}
