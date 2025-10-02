/**
 * @file theme_manager.h
 * @ingroup CoreSettings
 * @brief TODO: Document this.
 */

#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QBrush>
#include <QDir>
#include <QLoggingCategory>
#include <QMap>
#include <QObject>
#include <QPixmap>
#include <QString>
#include <array>

inline Q_LOGGING_CATEGORY(ThemeManagerLog, "theme_manager");

typedef QMap<QString, QString> QStringMap;
typedef QMap<int, QBrush> QBrushMap;

class QApplication;

class ThemeManager : public QObject
{
    Q_OBJECT
public:
    ThemeManager(QObject *parent = nullptr);

    enum Role
    {
        MinRole = 0,
        Hand = MinRole,
        Stack,
        Table,
        Player,
        MaxRole = Player,
    };

private:
    std::array<QBrush, Role::MaxRole + 1> brushes;
    QStringMap availableThemes;
    /*
      Internal cache for multiple backgrounds
    */
    std::array<QBrushMap, Role::MaxRole + 1> brushesCache;

protected:
    void ensureThemeDirectoryExists();
    QBrush loadBrush(QString fileName, QColor fallbackColor);
    QBrush loadExtraBrush(QString fileName, QBrush &fallbackBrush);

public:
    QStringMap &getAvailableThemes();

    QBrush &getBgBrush(Role zone);
    QBrush getExtraBgBrush(Role zone, int zoneId = 0);
protected slots:
    void themeChangedSlot();
signals:
    void themeChanged();
};

extern ThemeManager *themeManager;

#endif
