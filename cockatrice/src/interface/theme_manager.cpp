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
#define FUSION_THEME_NAME "Fusion (System Default)"
#define FUSION_THEME_NAME_LIGHT "Fusion (Light)"
#define FUSION_THEME_NAME_DARK "Fusion (Dark)"
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

static inline QList<PaletteColorInfo> queryAllPaletteColors(const QPalette &palette = qApp->palette())
{
    QList<PaletteColorInfo> colors;

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
static inline void printPaletteColors(const QPalette &palette = qApp->palette())
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

    // load themes from user profile dir
    dir.setPath(SettingsCache::instance().getThemesPath());

    availableThemes.insert(FUSION_THEME_NAME, dir.filePath("Fusion (System Default)"));
    availableThemes.insert(FUSION_THEME_NAME_LIGHT, dir.filePath("Fusion (Light)"));
    availableThemes.insert(FUSION_THEME_NAME_DARK, dir.filePath("Fusion (Dark)"));

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

static inline QPalette createDarkGreenFusionPalette()
{
    QPalette p;

    // ---------- Core backgrounds ----------
    p.setColor(QPalette::Window, QColor(30, 30, 30));        // #ff1e1e1e
    p.setColor(QPalette::Base, QColor(45, 45, 45));          // #ff2d2d2d
    p.setColor(QPalette::AlternateBase, QColor(53, 53, 53)); // #ff353535
    p.setColor(QPalette::Button, QColor(60, 60, 60));        // #ff3c3c3c
    p.setColor(QPalette::ToolTipBase, QColor(60, 60, 60));   // #ff3c3c3c

    // ---------- Core text ----------
    p.setColor(QPalette::WindowText, Qt::white);                       // #ffffffff
    p.setColor(QPalette::Text, Qt::white);                             // #ffffffff
    p.setColor(QPalette::ButtonText, Qt::white);                       // #ffffffff
    p.setColor(QPalette::ToolTipText, QColor(212, 212, 212));          // #ffd4d4d4
    p.setColor(QPalette::PlaceholderText, QColor(255, 255, 255, 128)); // #80ffffff

    // ---------- Selection / focus ----------
    const QColor highlight(20, 140, 60); // #ff148c3c
    p.setColor(QPalette::Highlight, highlight);
    p.setColor(QPalette::HighlightedText, Qt::white); // #ffffffff

    // ---------- Links ----------
    p.setColor(QPalette::Link, QColor(0, 246, 82));        // #ff00f652
    p.setColor(QPalette::LinkVisited, QColor(0, 211, 70)); // #ff00d346

    // ---------- Accent (Qt 6) ----------
#if (QT_VERSION >= QT_VERSION_CHECK(6, 6, 0))
    p.setColor(QPalette::Accent, QColor(0, 211, 70)); // #ff00d346
#endif

    // ---------- Bright text ----------
    p.setColor(QPalette::BrightText, QColor(0, 246, 82)); // #ff00f652

    // ---------- 3D / frame shading ----------
    p.setColor(QPalette::Light, QColor(120, 120, 120)); // #ff787878
    p.setColor(QPalette::Midlight, QColor(90, 90, 90)); // #ff5a5a5a
    p.setColor(QPalette::Mid, QColor(40, 40, 40));      // #ff282828
    p.setColor(QPalette::Dark, QColor(30, 30, 30));     // #ff1e1e1e
    p.setColor(QPalette::Shadow, Qt::black);            // #ff000000

    // ---------- Disabled state ----------
    const QColor disabledText(157, 157, 157); // #ff9d9d9d
    p.setColor(QPalette::Disabled, QPalette::WindowText, disabledText);
    p.setColor(QPalette::Disabled, QPalette::Text, disabledText);
    p.setColor(QPalette::Disabled, QPalette::ButtonText, disabledText);
    p.setColor(QPalette::Disabled, QPalette::Base, QColor(30, 30, 30));
    p.setColor(QPalette::Disabled, QPalette::Window, QColor(30, 30, 30));
    p.setColor(QPalette::Disabled, QPalette::Link, QColor(48, 140, 198));       // #ff308cc6
    p.setColor(QPalette::Disabled, QPalette::LinkVisited, QColor(255, 0, 255)); // #ffff00ff
    p.setColor(QPalette::Disabled, QPalette::ToolTipBase, QColor(255, 255, 220));
    p.setColor(QPalette::Disabled, QPalette::ToolTipText, Qt::black);

#if (QT_VERSION >= QT_VERSION_CHECK(6, 6, 0))
    p.setColor(QPalette::Disabled, QPalette::Accent, disabledText);
#endif

    // ---------- Inactive state ----------
    p.setColor(QPalette::Inactive, QPalette::Highlight, QColor(30, 30, 30));
    p.setColor(QPalette::Inactive, QPalette::HighlightedText, Qt::white);

#if (QT_VERSION >= QT_VERSION_CHECK(6, 6, 0))
    p.setColor(QPalette::Inactive, QPalette::Accent, QColor(30, 30, 30));
#endif

    return p;
}

static inline QPalette createLightGreenFusionPalette()
{
    QPalette p;

    // ---------- Core backgrounds ----------
    p.setColor(QPalette::Window, QColor(240, 240, 240));        // #fff0f0f0
    p.setColor(QPalette::Base, Qt::white);                      // #ffffffff
    p.setColor(QPalette::AlternateBase, QColor(233, 231, 227)); // #ffe9e7e3
    p.setColor(QPalette::Button, QColor(240, 240, 240));        // #fff0f0f0
    p.setColor(QPalette::ToolTipBase, QColor(255, 255, 220));   // #ffffffdc

    // ---------- Core text ----------
    p.setColor(QPalette::WindowText, Qt::black);                 // #ff000000
    p.setColor(QPalette::Text, Qt::black);                       // #ff000000
    p.setColor(QPalette::ButtonText, Qt::black);                 // #ff000000
    p.setColor(QPalette::ToolTipText, Qt::black);                // #ff000000
    p.setColor(QPalette::PlaceholderText, QColor(0, 0, 0, 128)); // #80000000

    // ---------- Selection / focus ----------
    const QColor highlight(20, 140, 60); // #ff148c3c
    p.setColor(QPalette::Highlight, highlight);
    p.setColor(QPalette::HighlightedText, Qt::white); // #ffffffff

    // ---------- Links ----------
    p.setColor(QPalette::Link, QColor(13, 95, 40));       // #ff0d5f28
    p.setColor(QPalette::LinkVisited, QColor(8, 64, 27)); // #ff08401b

    // ---------- Accent (Qt 6) ----------
#if (QT_VERSION >= QT_VERSION_CHECK(6, 6, 0))
    p.setColor(QPalette::Accent, QColor(16, 117, 50)); // #ff107532
#endif

    // ---------- Bright text ----------
    p.setColor(QPalette::BrightText, Qt::white); // #ffffffff

    // ---------- 3D / frame shading ----------
    p.setColor(QPalette::Light, Qt::white);                // #ffffffff
    p.setColor(QPalette::Midlight, QColor(227, 227, 227)); // #ffe3e3e3
    p.setColor(QPalette::Mid, QColor(160, 160, 160));      // #ffa0a0a0
    p.setColor(QPalette::Dark, QColor(160, 160, 160));     // #ffa0a0a0
    p.setColor(QPalette::Shadow, QColor(105, 105, 105));   // #ff696969

    // ---------- Disabled state ----------
    const QColor disabledText(120, 120, 120); // #ff787878
    p.setColor(QPalette::Disabled, QPalette::WindowText, disabledText);
    p.setColor(QPalette::Disabled, QPalette::Text, disabledText);
    p.setColor(QPalette::Disabled, QPalette::ButtonText, disabledText);
    p.setColor(QPalette::Disabled, QPalette::Base, QColor(240, 240, 240));
    p.setColor(QPalette::Disabled, QPalette::Window, QColor(240, 240, 240));
    p.setColor(QPalette::Disabled, QPalette::Midlight, QColor(247, 247, 247));
    p.setColor(QPalette::Disabled, QPalette::AlternateBase, QColor(247, 247, 247));
    p.setColor(QPalette::Disabled, QPalette::Shadow, Qt::black);
    p.setColor(QPalette::Disabled, QPalette::Link, QColor(0, 0, 255));
    p.setColor(QPalette::Disabled, QPalette::LinkVisited, QColor(255, 0, 255));

#if (QT_VERSION >= QT_VERSION_CHECK(6, 6, 0))
    p.setColor(QPalette::Disabled, QPalette::Accent, disabledText);
#endif

    // ---------- Inactive state ----------
    p.setColor(QPalette::Inactive, QPalette::Highlight, QColor(240, 240, 240));
    p.setColor(QPalette::Inactive, QPalette::HighlightedText, Qt::black);

#if (QT_VERSION >= QT_VERSION_CHECK(6, 6, 0))
    p.setColor(QPalette::Inactive, QPalette::Accent, QColor(240, 240, 240));
#endif

    return p;
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
    } else if (themeName == FUSION_THEME_NAME_LIGHT) {
        qApp->setStyle(QStyleFactory::create("Fusion"));
        qApp->setPalette(createLightGreenFusionPalette());
    } else if (themeName == FUSION_THEME_NAME_DARK) {
        qApp->setStyle(QStyleFactory::create("Fusion"));
        qApp->setPalette(createDarkGreenFusionPalette());
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
